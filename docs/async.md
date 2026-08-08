# Async / event-loop support

The driver executes queries on the underlying C driver's own IO threads, so
`Session::executeAsync()` returns immediately with a `Future`. Historically the
only way to get the result was `Future::get()`, which **blocks** the PHP thread
until the query resolves — unusable inside a non-blocking event loop.

As of v2, every `Future` also exposes a **notification file descriptor** so any
PHP event loop can await a query without blocking:

```php
interface Cassandra\Future {
    public function get(int|float|null $timeout = null): mixed; // blocking (unchanged)
    public function getResource(): mixed;   // readable stream; readable once resolved
    public function isReady(): bool;         // has the future resolved yet?
}
```

`getResource()` returns a PHP **stream resource** that becomes readable exactly
once, when the driver resolves the future on its IO thread (implemented with a
pipe + the C driver's completion callback). Register it with any reactor; on the
readable event call `get()`, which then returns without blocking (or rethrows
the driver error). The stream is **one-shot** — remove your watcher after the
first readable event.

This is the whole contract. Everything below is a thin, optional convenience on
top of it.

## The primitive: works with any loop, zero dependencies

```php
$futures = [];
for ($i = 0; $i < 100; $i++) {
    $f = $session->executeAsync(new Cassandra\SimpleStatement('SELECT ...'));
    $futures[(int) $f->getResource()] = $f;
}

while ($futures !== []) {
    $read = array_map(fn ($f) => $f->getResource(), $futures);
    $w = $e = [];
    stream_select($read, $w, $e, 5);
    foreach ($read as $stream) {
        $id = (int) $stream;
        $rows = $futures[$id]->get();   // ready — does not block
        // ... use $rows ...
        unset($futures[$id]);
    }
}
```

All 100 queries are in flight concurrently and are drained as they complete —
one `stream_select` loop, no blocking, no framework.

## Framework adapters (separate package)

The extension is pure C and ships no PHP. The bridges to third-party loops live
in a companion package, because they call framework classes the C code cannot see
and must not need an extension rebuild when a framework changes:

```bash
composer require codelieutenant/scylla-driver-async-adapters
```

Each one takes a `Future` and converts its notification fd into that framework's
await primitive. The frameworks themselves are optional peer dependencies —
install only the one you use.

You may not need the package at all. `Cassandra\Async\Poll` (PHP 8.6, below) is a
complete loop in C, and a `--enable-swoole` build makes `Future::get()`
coroutine-aware on its own.

### Revolt (framework-agnostic fiber await — recommended default)

```php
use Cassandra\Async\Revolt;
use Revolt\EventLoop;

EventLoop::queue(function () use ($session) {
    $rows = Revolt::await($session->executeAsync($cql));           // suspends the fiber
    $all  = Revolt::awaitAll([                                     // concurrent
        $session->executeAsync($cql1),
        $session->executeAsync($cql2),
    ]);
});
EventLoop::run();
```

`composer require revolt/event-loop`

### AMPHP v3

```php
use Cassandra\Async\Amp;
use function Amp\async;

$rows = async(fn () => Amp::toFuture($session->executeAsync($cql))->await())->await();

// concurrent
$results = Amp\Future\await(array_map(
    fn ($cql) => Amp::toFuture($session->executeAsync($cql)),
    $queries,
));
```

`composer require amphp/amp`

### ReactPHP

```php
use Cassandra\Async\ReactPhp;
use React\EventLoop\Loop;

$loop = Loop::get();
ReactPhp::toPromise($session->executeAsync($cql), $loop)
    ->then(fn ($rows) => /* ... */, fn (Throwable $e) => /* ... */);
$loop->run();
```

`composer require react/event-loop react/promise`

### Swoole

```php
use Cassandra\Async\Swoole;

Swoole\Coroutine\run(function () use ($session) {
    $rows = Swoole::await($session->executeAsync($cql)); // suspends only this coroutine
});
```

Requires `ext-swoole` (or `ext-openswoole`); must run inside a coroutine. The
userland adapter works with any build (it watches the fd via
`Swoole\Coroutine\System::waitEvent`).

#### Native build (optional, behind a flag)

For tighter integration you can compile native (Open)Swoole support into the
extension. When built this way, **`Future::get()` itself becomes coroutine-aware**:
called inside a Swoole coroutine it suspends only that coroutine (via
`swoole::coroutine::System::wait_event` on the notification fd) instead of
blocking the worker — so ordinary blocking-style code cooperates with the
scheduler, and `Cassandra\Async\Swoole::await()` becomes equivalent to a plain
`->get()`. Outside a coroutine (or when swoole isn't loaded) `get()` blocks
exactly as before.

This is opt-in because Swoole's coroutine API is C++; enabling it compiles one
small C++ shim (`src/Async/SwooleBridge.cc`) against the (open)swoole source
headers. The default build stays pure C.

Build against a Swoole source tree (a `phpize`-configured checkout, so
`include/swoole_config.h` exists):

```bash
# Swoole
cmake --preset DebugPHP8.4NTS \
    -DPHP_SCYLLADB_ENABLE_SWOOLE=ON \
    -DPHP_SCYLLADB_SWOOLE_SRC=/path/to/swoole-src

# OpenSwoole
cmake --preset DebugPHP8.4NTS \
    -DPHP_SCYLLADB_ENABLE_OPENSWOOLE=ON \
    -DPHP_SCYLLADB_SWOOLE_SRC=/path/to/openswoole-src
```

(Or `pie`/`pecl` with `--enable-swoole` / `--enable-openswoole` and
`--with-swoole-src=...`.) The extension does **not** link `libswoole`; the
swoole C++ symbols resolve at runtime from the loaded extension, so ensure
`extension=swoole` is loaded **before** `extension=cassandra` in your INI. If
swoole isn't loaded at runtime, `get()` transparently falls back to blocking.

## Shared reactor — O(1) fds for high fan-out

The per-future model above watches one fd per in-flight future, which is simple
and, for typical concurrency, free. But awaiting thousands of queries means
thousands of fds — and `stream_select()` caps out at `FD_SETSIZE` (~512 on
macOS), while every loop tick scans all of them. For heavy fan-out, the **shared
reactor** collapses this to a single fd + a completion queue: the driver's
IO-thread callbacks push completed futures onto one queue and signal one
eventfd, so the loop watches **one fd regardless of how many futures are in
flight**.

It's opt-in and coexists with `getResource()` (a given future uses one model or
the other, not both). Every concrete future works — `FutureRows`,
`FutureSession`, `FuturePreparedStatement`, `FutureClose` and `FutureValue` — so
a boot sequence can fan out `prepareAsync()` the same way a request fans out
`executeAsync()`.

```php
use Cassandra\Async\Reactor;

$futures = [];
for ($i = 0; $i < 5000; $i++) {
    Reactor::add($session->executeAsync($cql));   // register with the shared reactor
}

$resource = Reactor::resource();                   // the ONE stream to watch
while (Reactor::pending() > 0) {
    $r = [$resource]; $w = $e = [];
    stream_select($r, $w, $e, 5);
    foreach (Reactor::poll(64) as $future) {       // at most 64 per tick
        $rows = $future->get();                    // ready — no network wait
    }
}
```

### Push instead of pull

`add()` takes an optional completion callback. `poll()` then calls it with the
resolved future rather than returning that future, which keeps the dispatch next
to the query that produced it:

```php
foreach ($queries as $cql) {
    Reactor::add($session->executeAsync($cql), function (FutureRows $future) {
        $rows = $future->get();
    });
}

while (Reactor::pending() > 0) {
    stream_select($r = [$resource], $w, $e, 5);
    Reactor::poll(64);                             // callbacks fire here
}
```

The choice is per future, so both styles mix: a future added without a callback
still comes back from `poll()`.

The callback runs on the PHP thread, inside `poll()`. It cannot run on the
driver IO thread that resolves the future — no Zend API is safe there, which is
why the completion travels over a descriptor at all. The C-level
`cass_future_set_callback` hop is already in place: it writes the wakeup byte
and queues the completion, and nothing more.

A callback that throws stops that batch and the exception leaves `poll()`.
Nothing is lost. What is left stays queued, and the futures that same call had
already collected go back to the front of the queue — a throwing `poll()` cannot
also return its array, so they come back on the next call, ahead of the rest.
The descriptor stays readable either way.

### Keep the tick short

`get()` on a future that `poll()` returned does not wait on the network — the
driver already resolved it. It does decode the result set on the calling thread,
and `poll()` with no argument hands back every completion at once. Five thousand
completions in one tick means five thousand decodes before any other watcher
runs.

Pass a batch size. What `poll($max)` does not return stays queued, and the
notification descriptor stays readable, so the loop comes straight back for the
rest and everything else gets a turn in between. Tune `$max` to the size of your
rows: small point reads take a large batch, wide partitions take a small one.

The same rule applies to a fiber-based loop, and the `ReactorRevolt` adapter
already follows it: its watcher only resumes the waiting fibers, so each decode
lands in its own fiber on a later tick instead of in the watcher callback.

Revolt adapter (one watcher, dispatches to per-future fibers):

```php
use Cassandra\Async\ReactorRevolt;

EventLoop::queue(function () use ($session, $queries) {
    $rows = ReactorRevolt::awaitAll(array_map(
        fn ($cql) => $session->executeAsync($cql),
        $queries,          // thousands is fine — one fd
    ));
});
EventLoop::run();
```

The reactor lives in module globals (per-thread under ZTS). Its eventfd + mutex
persist for the process/thread; its registration state is reset each request.
Currently `add()` accepts query futures (`FutureRows` from `executeAsync`).

## `Io\Poll` handles (PHP 8.6, build flag)

PHP 8.6 adds a polling API ([RFC: poll_api](https://wiki.php.net/rfc/poll_api)):
an `Io\Poll\Context` backed by epoll, kqueue, event ports or WSAPoll, plus a C
interface that lets an extension present its own descriptors as
`Io\Poll\Handle` objects.

When the extension is built against PHP 8.6 or later, it registers
`Cassandra\Async\PollHandle` — a driver descriptor as a native `Io\Poll\Handle`.

### The whole thing: `Cassandra\Async\Poll`

A complete loop, in C, in the extension. No package, no adapter, no
`stream_select`:

```php
use Cassandra\Async\Poll;

$loop = new Poll();

foreach ($queries as $cql) {
    $loop->watch($session->executeAsync($cql), function (Cassandra\Future $future) {
        foreach ($future->get() as $row) { /* … */ }
    });
}

$loop->run();
```

`watch()` returns the `Io\Poll\Watcher`, and `context()` gives you the context
itself, so your own sockets and pipes go in the same loop.

Leave the callback out to pull instead of push — `tick()` returns the futures
that resolved:

```php
$loop->watch($session->executeAsync($cql));

while ($loop->pending() > 0) {
    foreach ($loop->tick(5) as $future) {
        $rows = $future->get();
    }
}
```

Both styles mix. Within one `tick()` the callbacks run first and the return
array is built after, so a callback that throws leaves every callback-less
future of that round still watched — the next `tick()` returns it.

`Poll` also removes each watcher once its future resolves. That step is not
optional if you drive a `Context` yourself: a driver descriptor stays readable
after it fires, so a watcher left in place spins the loop.

### Driving `Io\Poll\Context` yourself

`PollHandle::forFuture($future)` is the poll-API counterpart of
`getResource()`. Attach the future as the watcher data and the loop carries it
for you:

```php
use Cassandra\Async\PollHandle;
use Io\Poll\Context;
use Io\Poll\Event;

$ctx = new Context();
$future = $session->executeAsync($cql);
$ctx->add(PollHandle::forFuture($future), [Event::Read], $future);

foreach ($ctx->wait(5) as $watcher) {
    $rows = $watcher->getData()->get();
    $watcher->remove();                        // or it fires on every tick
}
```

That is one descriptor and one watcher per query. At thousands in flight, watch
the shared reactor instead — one descriptor for all of them:

```php
$ctx->add(PollHandle::reactor(), [Event::Read]);

for ($i = 0; $i < 5000; $i++) {
    Reactor::add($session->executeAsync($cql));
}

while (Reactor::pending() > 0) {
    $ctx->wait(5);
    Reactor::poll(64);                         // bounded tick, see above
}
```

`PollHandle::reactor()` is cached per request and returns the same object every
call, which matches how a context keys its watchers. A per-future handle is a
new object each call, and a future uses one async model at a time — do not
combine `forFuture()` with `Reactor::add()`.

`Context::wait()` is still changing while 8.6 is in alpha. Up to 8.6.0alpha3 it
took `(?int $seconds, int $microseconds, ?int $maxEvents)`. After that it takes a
`?\Time\Duration`, built through a static factory because the class is readonly
with a private constructor:

```php
$ctx->wait(\Time\Duration::fromSeconds(5));     // 8.6 after alpha3
$ctx->wait(5);                                  // 8.6.0alpha3 and earlier
```

Check the signature in your build. `Poll::tick()` reads the declared parameter
type and calls whichever form it finds, so `Poll` needs no change either way, and
`PollHandle` only supplies the descriptor.

### What it buys

`Io\Poll\Context::add()` takes an `Io\Poll\Handle`, and 8.6 ships no handle
class for a plain PHP stream, so a driver descriptor cannot reach the polling
API without one. `PollHandle` is that handle, and it skips a layer while it is
at it: it registers the driver's own descriptor, so there is no `dup()` and no
stream resource per watched future. What it does not change is throughput — the
shared reactor already folds every future onto one descriptor, so a faster
many-descriptor backend has nothing to speed up there. Read the gain as
interoperability and fewer descriptors, not queries per second.

### Off by default — turn it on

The integration is **not built unless you ask for it**:

```bash
cmake --preset DebugPHP8.6NTS -DPHP_SCYLLADB_ENABLE_POLL_API=ON
```

`PHP_SCYLLADB_ENABLE_POLL_API` in the root `CMakeLists.txt` takes:

| Value | Effect |
| --- | --- |
| `OFF` | Default. Neither class is compiled, on any PHP. |
| `ON` | Compile it, and fail configure when `main/php_poll.h` is missing. |
| `AUTO` | Compile it when the PHP being built against provides that header. |

It stays off until PHP 8.6 is released, because the API is still moving in
8.6-dev: `Io\Poll\Context::wait()` changed shape after 8.6.0alpha3. A default-on
build would follow a moving target.

So the classes are absent from every default build, and from any build against
PHP 8.5 or older. Guard call sites with `Poll::isSupported()`, or
`class_exists(\Cassandra\Async\PollHandle::class)`.

## How much does it cost?

The notification path adds only the overhead of a pipe + one `stream_select`
over a blind blocking drain, while never blocking a thread a real reactor could
use for other IO. From the committed live baselines (`benchmarks/baselines/`,
256 point reads):

| Strategy | Mean | Notes |
| --- | ---: | --- |
| `benchSequential` (blocking `execute` loop) | ~55 ms | serialized |
| `benchPipelined` (`executeAsync` + blocking drain) | ~7.5 ms | concurrent, but blocks the thread |
| `benchEventLoopConcurrent` (`getResource` + `stream_select`) | ~7.6 ms | concurrent **and** non-blocking |

The event-loop path is within ~1–2 % of raw pipelining and ~7× faster than a
sequential loop — while cooperating with the rest of your reactor.

## Notes & limits

- **fd usage & platform fast path:** the notifier uses a Linux **`eventfd`**
  (one syscall to create, one fd, an 8-byte counter, and — being a counter, not
  a pipe — no SIGPIPE) and falls back to a POSIX **pipe** on macOS/BSD. Per
  pending future: on Linux `getResource()` holds 2 fds (eventfd + a duplicate
  for the stream) and the native coroutine `get()` path holds just 1; on
  macOS/BSD it's 3 and 2 respectively (pipe read+write, plus the stream's dup
  for `getResource()`). At very high concurrency (thousands in flight) mind your
  `RLIMIT_NOFILE`. `get()` in a non-coroutine, non-Swoole build never allocates
  an fd, so blocking users pay nothing. For heavy fan-out use the **shared
  reactor** (above) — it stays at one fd regardless of concurrency, and in the
  committed baselines it is actually *faster* than per-future fds at 256 in
  flight (fewer fds for `stream_select` to scan) and keeps scaling to thousands
  where the per-future approach hits `FD_SETSIZE`.
- **cost:** the notification path adds only the eventfd/pipe create + one
  `write` + one wait over a blind blocking drain. In the committed baselines the
  `benchEventLoopConcurrent` vs `benchPipelined` delta is within a couple of
  percent — negligible against a real query's network round-trip. Native Swoole
  detection is cached after the first call (a `get_current_cid()` check per
  `get()`, no repeated module lookups).
- **One-shot:** the stream fires once; adapters remove their watcher on the
  first event. Don't keep a persistent watcher on it.
- **Errors** surface from `get()` (and therefore from every adapter) as the
  usual driver exceptions.
- **Uniform:** already-resolved futures (`FutureValue`, cached sessions) return
  an immediately-readable stream and `isReady() === true`, so one code path
  handles every future type.
