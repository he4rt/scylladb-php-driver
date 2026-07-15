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

## Framework adapters (`Cassandra\Async\*`)

Thin bridges live in `lib/Async` (autoloaded as `Cassandra\Async\`). Each takes
a `Future` and converts its notification fd into that framework's await
primitive. The frameworks themselves are optional peer dependencies — install
only the one you use.

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
the other, not both).

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
    foreach (Reactor::poll() as $future) {         // futures completed since last poll
        $rows = $future->get();                    // ready — no block
    }
}
```

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
