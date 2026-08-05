# Asynchronous queries

Every blocking call has an asynchronous twin that returns a future. The request starts at once. The
result arrives when you ask for it.

| Blocking | Asynchronous | Future class |
| --- | --- | --- |
| `Cluster::connect()` | `Cluster::connectAsync()` | `Cassandra\FutureSession` |
| `Session::execute()` | `Session::executeAsync()` | `Cassandra\FutureRows` |
| `Session::prepare()` | `Session::prepareAsync()` | `Cassandra\FuturePreparedStatement` |
| `Session::close()` | `Session::closeAsync()` | `Cassandra\FutureClose` |
| `Rows::nextPage()` | `Rows::nextPageAsync()` | `Cassandra\Future` |

All of them implement `Cassandra\Future`:

```php
interface Future
{
    public function get(int|float|null $timeout = null): mixed;
}
```

`get()` blocks until the result arrives, then returns it. It throws the same exceptions the blocking
call would throw. Calling `get()` twice returns the same result.

## The basic pattern

```php
$future = $session->executeAsync($statement, ['arguments' => [$id]]);

// Do other work here. The query is already travelling.

$rows = $future->get();
```

One future on its own buys little. The gain comes from starting many at once.

## Running queries in parallel

```php
$statement = $session->prepare('SELECT * FROM users WHERE id = ?');

$futures = [];
foreach ($ids as $id) {
    $futures[(string) $id] = $session->executeAsync($statement, ['arguments' => [$id]]);
}

$users = [];
foreach ($futures as $key => $future) {
    $users[$key] = $future->get()->first();
}
```

The whole set costs about as much as the slowest single query, not the sum. Each request goes to a
replica that owns the data, so the load spreads across the cluster.

::: warning Bound the number in flight
Starting ten thousand futures at once fills the client request queue and the server. Work in
windows.
:::

```php
$window   = 128;
$inFlight = [];
$results  = [];

foreach ($ids as $id) {
    $inFlight[] = $session->executeAsync($statement, ['arguments' => [$id]]);

    if (count($inFlight) >= $window) {
        foreach ($inFlight as $future) {
            $results[] = $future->get();
        }
        $inFlight = [];
    }
}

foreach ($inFlight as $future) {
    $results[] = $future->get();
}
```

A window between 64 and 256 suits most workloads. Measure with your own data.

## Timeouts

`get()` takes a timeout in seconds:

```php
try {
    $rows = $future->get(2.5);
} catch (Cassandra\Exception\TimeoutException $e) {
    // The result did not arrive in time. The request may still complete on the server.
}
```

Without an argument, `get()` waits for the request timeout configured on the cluster. See
[connection pool and timeouts](/guide/connection-tuning).

## Errors

A failed query throws from `get()`, not from `executeAsync()`. Wrap the collection loop:

```php
$futures = array_map(
    fn ($id) => $session->executeAsync($statement, ['arguments' => [$id]]),
    $ids,
);

$results = [];
$errors  = [];

foreach ($futures as $i => $future) {
    try {
        $results[$i] = $future->get();
    } catch (Cassandra\Exception $e) {
        $errors[$i] = $e;
    }
}
```

One failed future does not affect the others.

## Connecting asynchronously

```php
$futureSession = $cluster->connectAsync('shop');

// Load configuration, warm a cache, open other connections.

$session = $futureSession->get(10.0);
```

Useful at process start, where the connection handshake and other startup work can overlap.

## Preparing asynchronously

Prepare a whole statement set at boot in parallel:

```php
$cql = [
    'findById'    => 'SELECT * FROM users WHERE id = ?',
    'findByEmail' => 'SELECT * FROM users_by_email WHERE email = ?',
    'insert'      => 'INSERT INTO users (id, email) VALUES (?, ?)',
];

$futures = array_map(fn ($q) => $session->prepareAsync($q), $cql);
$prepared = array_map(fn ($f) => $f->get(), $futures);

$rows = $session->execute($prepared['findById'], ['arguments' => [$id]]);
```

## Pipelined paging

`nextPageAsync()` fetches the next page while the current one is being processed:

```php
$rows = $session->execute($statement, ['page_size' => 1000]);

while (true) {
    $next = $rows->isLastPage() ? null : $rows->nextPageAsync();

    foreach ($rows as $row) {
        process($row);
    }

    if ($next === null) {
        break;
    }

    $rows = $next->get();
}
```

## A read-then-write pipeline

```php
$read  = $session->prepare('SELECT id, email FROM users WHERE id = ?');
$write = $session->prepare('UPDATE users SET normalized_email = ? WHERE id = ?');

$window = 100;
$reads  = [];

foreach ($ids as $id) {
    $reads[] = $session->executeAsync($read, ['arguments' => [$id]]);

    if (count($reads) < $window) {
        continue;
    }

    $writes = [];
    foreach ($reads as $future) {
        $row = $future->get()->first();
        if ($row === null) {
            continue;
        }
        $writes[] = $session->executeAsync($write, [
            'arguments' => [strtolower($row['email']), $row['id']],
        ]);
    }

    foreach ($writes as $future) {
        $future->get();
    }

    $reads = [];
}
```

## What futures are not

`get()` blocks the PHP process. The driver resolves futures on its own IO threads, but PHP has one
execution thread, so:

- **A future does not free the PHP process.** Between `executeAsync()` and `get()`, PHP can run
  other PHP code, not other requests.
- **There is no callback and no event loop integration** in this release. You cannot register a
  handler that fires when the future resolves.
- **`get()` is the only way to observe completion.** Collect futures in the order you want to
  consume them.

Within those limits, concurrency across many queries is real and large. The work happens on the C
driver IO threads while PHP waits once instead of many times.
