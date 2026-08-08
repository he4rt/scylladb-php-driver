# Event loops and non-blocking waits

[Asynchronous queries](/guide/async) start many requests at once, but `get()` still stops the PHP
process until the result arrives. In a script that only talks to ScyllaDB, that is fine. In an event
loop, it is not: one blocking `get()` freezes every other socket, timer and coroutine the loop owns.

This page covers the part that fixes it. Every future can hand you a descriptor that becomes
readable when the driver resolves it, so a loop waits on ScyllaDB the same way it waits on anything
else.

## The primitive

Two methods on `Cassandra\Future` carry the whole feature:

```php
/** @return resource a stream that becomes readable when the future resolves */
public function getResource(): mixed;

public function isReady(): bool;
```

The contract is short and worth memorising:

> When the stream is readable, `get()` returns without a network wait.

That holds for every future type, for a failed query as well as a successful one, and it is the only
promise the descriptor makes. `get()` still decodes the result on the PHP thread, and it still throws
the driver error for a failed query.

`isReady()` answers the same question without a descriptor. Use it to skip the loop entirely for a
future that has already resolved.

::: tip One model per future
A future uses either a per-future descriptor or the [shared reactor](#shared-reactor), never both.
The driver allows one completion callback per future, so mixing the two throws
`Cassandra\Exception\RuntimeException`.
:::

## Without a framework

`stream_select()` is enough to await many queries at once:

```php
$pending = [];
$streams = [];

foreach ($ids as $id) {
    $future                = $session->executeAsync($statement, ['arguments' => [$id]]);
    $key                   = (int) $future->getResource();
    $pending[$key]         = $future;
    $streams[$key]         = $future->getResource();
}

while ($pending !== []) {
    $read = $streams;
    $write = $except = [];

    if (stream_select($read, $write, $except, 5) < 1) {
        break;                                  // timed out
    }

    foreach ($read as $stream) {
        $key = (int) $stream;
        $rows = $pending[$key]->get();          // ready — no network wait
        unset($pending[$key], $streams[$key]);
    }
}
```

`getResource()` is cached. Every call on one future returns the same stream, so a loop registers one
watcher and keeps it.

::: warning stream_select has a ceiling
`stream_select()` cannot watch more than `FD_SETSIZE` descriptors, which is around 512 on macOS, and
it scans every one of them on every tick. Past a few hundred futures in flight, use the
[shared reactor](#shared-reactor).
:::

## Framework adapters

The adapters live in a separate package, so the extension carries no framework dependency:

```bash
composer require codelieutenant/scylla-driver-async-adapters
```

| Class | Framework | Entry point |
| --- | --- | --- |
| `Cassandra\Async\Revolt` | Revolt (and anything on it) | `Revolt::await($future)` |
| `Cassandra\Async\Amp` | AMPHP v3 | `Amp::toFuture($future)` |
| `Cassandra\Async\ReactPhp` | ReactPHP | `ReactPhp::toPromise($future, $loop)` |
| `Cassandra\Async\Swoole` | Swoole / OpenSwoole | `Swoole::await($future)` |
| `Cassandra\Async\ReactorRevolt` | Revolt, through the shared reactor | `ReactorRevolt::awaitAll($futures)` |

Each one is thin. It registers the descriptor with the loop, waits for the readable event, and calls
`get()`:

```php
use Cassandra\Async\Revolt;
use Revolt\EventLoop;

EventLoop::queue(function () use ($session, $statement, $ids) {
    // Suspends this fiber only. The loop keeps running.
    $rows = Revolt::await($session->executeAsync($statement, ['arguments' => [$ids[0]]]));
});

EventLoop::run();
```

`Revolt::awaitAll()` takes an iterable of futures and awaits them concurrently, one fiber each,
keyed the same as the input.

## Shared reactor

One descriptor per future is simple and, for typical concurrency, free. For heavy fan-out it is
neither. `Cassandra\Async\Reactor` collapses the cost to **one descriptor no matter how many futures
are in flight**: the driver IO threads push completions onto a queue and signal a single eventfd, and
the loop watches that.

```php
use Cassandra\Async\Reactor;

for ($i = 0; $i < 5000; $i++) {
    Reactor::add($session->executeAsync($statement, ['arguments' => [$ids[$i]]]));
}

$resource = Reactor::resource();                // the ONE stream to watch

while (Reactor::pending() > 0) {
    $read = [$resource];
    $write = $except = [];
    stream_select($read, $write, $except, 5);

    foreach (Reactor::poll(64) as $future) {    // at most 64 per tick
        $rows = $future->get();
    }
}
```

Every future type works, so a boot sequence can fan out `prepareAsync()` the same way a request fans
out `executeAsync()`.

### Push instead of pull

`add()` takes an optional callback. `poll()` then calls it with the resolved future rather than
returning that future, which keeps the handling next to the query that produced it:

```php
Reactor::add($session->executeAsync($cql), function (Cassandra\FutureRows $future) {
    $rows = $future->get();
});

while (Reactor::pending() > 0) {
    stream_select($read = [$resource], $write, $except, 5);
    Reactor::poll(64);                          // callbacks fire here
}
```

The choice is per future, so both styles mix. The callback runs on the PHP thread inside `poll()`,
never on a driver IO thread, so it may do anything a normal callable may.

A callback that throws stops that batch and the exception leaves `poll()`. Nothing is lost. What is
left stays queued, and the futures that same call had already collected go back to the front of the
queue, so the next `poll()` returns them first.

### Keep the tick short

`get()` on a future `poll()` returned does no network wait, but it does decode the result set on the
PHP thread. `poll()` with no argument hands back every completion at once, so five thousand
completions means five thousand decodes before any other watcher runs.

Pass a batch size. What `poll($max)` does not return stays queued and the descriptor stays readable,
so the loop takes the remainder on the next tick and everything else gets a turn in between.

## Native polling on PHP 8.6

PHP 8.6 added `Io\Poll` — epoll, kqueue or event ports behind one API. When the extension is built
with `--enable-poll-api`, two more classes appear.

`Cassandra\Async\Poll` is a ready-made loop:

```php
$loop = new Cassandra\Async\Poll();

foreach ($queries as $cql) {
    $loop->watch($session->executeAsync($cql), function ($future) {
        foreach ($future->get() as $row) { /* … */ }
    });
}

$loop->run();
```

It removes each watcher as soon as its future resolves. That is required, not tidiness: a driver
descriptor stays readable after it fires, so a watcher left in place makes the loop spin.
`context()` returns the underlying `Io\Poll\Context`, so your own descriptors go in the same loop.

`Cassandra\Async\PollHandle` is the lower-level piece — a driver descriptor as a native
`Io\Poll\Handle`, for a loop you drive yourself:

```php
$ctx = new Io\Poll\Context();
$ctx->add(Cassandra\Async\PollHandle::reactor(), [Io\Poll\Event::Read]);
```

::: warning Off by default
PHP 8.6 is not released and its polling API is still moving, so these classes are absent unless the
build asks for them. Guard with `Cassandra\Async\Poll::isSupported()` or
`class_exists(Cassandra\Async\PollHandle::class)`.
:::

## Native Swoole coroutines

Built with `--enable-swoole` (or `--enable-openswoole`), `Future::get()` becomes coroutine-aware on
its own: inside a coroutine it suspends only that coroutine, and the scheduler keeps running
everything else. No adapter and no code change:

```php
Swoole\Coroutine\run(function () use ($session, $cql) {
    $rows = $session->executeAsync($cql)->get();   // yields, does not block
});
```

Outside a coroutine, and in a build without the flag, `get()` blocks exactly as before.

## Choosing

| Situation | Use |
| --- | --- |
| No framework, tens of futures | `getResource()` + `stream_select()` |
| Revolt, AMPHP or ReactPHP | The adapter for that framework |
| Hundreds or thousands in flight | `Cassandra\Async\Reactor` |
| Swoole or OpenSwoole | A native build, then plain `get()` |
| PHP 8.6 with `--enable-poll-api` | `Cassandra\Async\Poll` |

## Limits

- **The result still decodes on the PHP thread.** Non-blocking means the wait is free, not the work.
- **One completion model per future.** Pick the descriptor or the reactor, not both.
- **The descriptor is level-triggered.** It stays readable until the completion is consumed, so a
  loop that wakes twice sees it twice. Edge-triggered watchers work too, because each completion
  writes again.
- **`Reactor::poll()` never waits.** It returns whatever has resolved since the last call. The
  waiting is your loop's job.
