# Async and event loops

Classes for awaiting futures inside an event loop. See the
[event loops guide](/guide/event-loops) for how they fit together.

## `Cassandra\Future` additions

```php
interface Future
{
    public function get(int|float|null $timeout = null): mixed;

    /** @return resource */
    public function getResource(): mixed;

    public function isReady(): bool;
}
```

| Method | Notes |
| --- | --- |
| `getResource()` | A stream that becomes readable when the future resolves. Cached: the same resource every call. |
| `isReady()` | `true` when `get()` returns without a network wait. Never blocks. |

The contract: **when the stream is readable, `get()` does no network wait**. It still decodes the
result, and it still throws the driver error for a failed query.

`getResource()` throws `Cassandra\Exception\RuntimeException` when the future is registered with
`Cassandra\Async\Reactor`. A future uses one completion model, not two.

## `Cassandra\Async\Reactor`

One descriptor for any number of in-flight futures. All methods are static, and the reactor is
per process (per thread on ZTS), reset at the end of each request.

```php
final class Reactor
{
    /** @return resource */
    public static function resource(): mixed;
    public static function add(\Cassandra\Future $future, ?callable $onComplete = null): void;
    /** @return list<\Cassandra\Future> */
    public static function poll(?int $max = null): array;
    public static function pending(): int;
}
```

| Method | Notes |
| --- | --- |
| `resource()` | The one stream to register with your loop. Readable when one or more registered futures have completed. Cached per request. |
| `add()` | Registers a future. Every concrete future type is accepted. With `$onComplete`, `poll()` calls it instead of returning the future. |
| `poll()` | Returns the futures resolved since the last call. Never waits. `$max` caps the batch, `null` takes every completion. |
| `pending()` | Registered futures not yet returned by `poll()`. |

### Errors

| Condition | Exception |
| --- | --- |
| The future already has a per-future descriptor | `Cassandra\Exception\RuntimeException` |
| The future is already registered | `Cassandra\Exception\RuntimeException` |
| `poll($max)` with `$max < 1` | `ValueError` |

### Dispatch

A future registered without a callback comes back from `poll()`, ready. A future registered with one
has that callback called inside `poll()`, on the PHP thread, with the resolved future as its only
argument. Both styles mix freely in one reactor.

A callback that throws stops the batch and the exception leaves `poll()`. The rest stays queued, and
the futures that call had already collected go back to the front of the queue for the next `poll()`.

```php
use Cassandra\Async\Reactor;

Reactor::add($session->executeAsync($cql));
Reactor::add($session->prepareAsync($cql), fn ($f) => $prepared[] = $f->get());

$resource = Reactor::resource();
while (Reactor::pending() > 0) {
    $read = [$resource];
    $write = $except = [];
    stream_select($read, $write, $except, 5);

    foreach (Reactor::poll(64) as $future) {
        $rows = $future->get();
    }
}
```

## `Cassandra\Async\Poll`

A loop over PHP 8.6's `Io\Poll\Context`. Present only in a build with `--enable-poll-api`.

```php
final class Poll
{
    public function __construct(?\Io\Poll\Backend $backend = null);
    public static function isSupported(): bool;
    public function watch(\Cassandra\Future $future, ?callable $onComplete = null): \Io\Poll\Watcher;
    public function pending(): int;
    /** @return list<\Cassandra\Future> */
    public function tick(?int $timeout = 5): array;
    public function run(?int $timeout = 5): void;
    public function context(): \Io\Poll\Context;
}
```

| Method | Notes |
| --- | --- |
| `isSupported()` | `false` when this build has no polling API. Check it before `new Poll()`. |
| `watch()` | Watches one future and returns its `Io\Poll\Watcher`. Without `$onComplete`, `tick()` returns the future instead. |
| `pending()` | Futures watched but not yet resolved. |
| `tick()` | Waits for the next completions, dispatches them, and drops their watchers. `$timeout` is in seconds, `null` waits indefinitely. |
| `run()` | Ticks until nothing is watched. A future without a callback is dropped, since nothing would read it. |
| `context()` | The underlying context, for adding your own descriptors to the same loop. |

Within one `tick()` the callbacks run before the return array is built, so a callback that throws
leaves every callback-less future of that round still watched for the next call.

One descriptor holds one watcher per context, so watching the same future twice throws
`Io\Poll\HandleAlreadyWatchedException`.

```php
if (!Cassandra\Async\Poll::isSupported()) {
    return;
}

$loop = new Cassandra\Async\Poll();
$loop->watch($session->executeAsync($cql), function ($future) {
    foreach ($future->get() as $row) { /* … */ }
});
$loop->run();
```

## `Cassandra\Async\PollHandle`

A driver descriptor as a native `Io\Poll\Handle`, for a loop you drive yourself. Present only in a
build with `--enable-poll-api`.

```php
final class PollHandle implements \Io\Poll\Handle
{
    public static function reactor(): PollHandle;
    public static function forFuture(\Cassandra\Future $future): PollHandle;
    public function getFileDescriptor(): int;
    public function isValid(): bool;
}
```

| Method | Notes |
| --- | --- |
| `reactor()` | The shared reactor's handle. Cached per request: one object, one descriptor. |
| `forFuture()` | A per-future handle, the poll-API counterpart of `getResource()`. |
| `getFileDescriptor()` | The raw descriptor. Do not close it — the driver owns it. |
| `isValid()` | `false` once the descriptor is gone. |

Unlike `getResource()`, this allocates no stream and duplicates no descriptor. The handle holds its
own reference to the driver's notifier, so it stays valid after the future is released.

```php
$ctx = new Io\Poll\Context();
$ctx->add(Cassandra\Async\PollHandle::reactor(), [Io\Poll\Event::Read]);

foreach ($ctx->wait(null) as $watcher) {
    foreach (Cassandra\Async\Reactor::poll(64) as $future) {
        $rows = $future->get();
    }
}
```

## Framework adapters

Shipped separately, so the extension depends on no framework:

```bash
composer require codelieutenant/scylla-driver-async-adapters
```

| Class | Framework | Entry point |
| --- | --- | --- |
| `Cassandra\Async\Revolt` | Revolt | `await(Future, ?float): mixed`, `awaitAll(iterable, ?float): array` |
| `Cassandra\Async\Amp` | AMPHP v3 | `toFuture(Future): Amp\Future` |
| `Cassandra\Async\ReactPhp` | ReactPHP | `toPromise(Future, LoopInterface): PromiseInterface` |
| `Cassandra\Async\Swoole` | Swoole / OpenSwoole | `await(Future, ?float): mixed` |
| `Cassandra\Async\ReactorRevolt` | Revolt, through the shared reactor | `await(Future): mixed`, `awaitAll(iterable): array` |
