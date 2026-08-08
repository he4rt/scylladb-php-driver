# Rows and Futures

## `Cassandra\Rows`

One page of a result set.

```php
final class Rows implements \Iterator, \Countable, \ArrayAccess
{
    public function count(): int;
    public function first(): mixed;

    public function isLastPage(): bool;
    public function nextPage(int|float|null $timeout = null): Rows|false;
    public function nextPageAsync(): Future;
    public function pagingStateToken(): ?string;

    // Iterator
    public function rewind(): void;
    public function current(): mixed;
    public function key(): mixed;
    public function next(): void;
    public function valid(): bool;

    // ArrayAccess
    public function offsetExists(mixed $offset): bool;
    public function offsetGet(mixed $offset): mixed;
    public function offsetSet(mixed $offset, mixed $value): void;
    public function offsetUnset(mixed $offset): void;
}
```

| Method | Notes |
| --- | --- |
| `count()` | Rows on **this page**, not in the whole result set. Triggers no fetch. |
| `first()` | The first row of the page, or `null` when the page is empty. |
| `isLastPage()` | `true` when no further page exists. Check it before `nextPage()`. |
| `nextPage()` | Fetches and returns a new `Rows`. Blocks. `$timeout` is in seconds. |
| `nextPageAsync()` | Starts the fetch and returns a future at once. |
| `pagingStateToken()` | An opaque resume token, or `null` when there is no next page. |

```php
foreach ($rows as $index => $row) {
    echo $index, ': ', $row['email'], "\n";
}

$rows[0];            // the first row, by offset
count($rows);        // rows on this page
$rows->first();      // the first row, or null
```

Each row is an associative array keyed by column name.

::: warning Check isLastPage() before nextPage()
Calling `nextPage()` past the end does not return a usable result set.
:::

See [results and paging](/guide/results).

## `Cassandra\Future`

```php
interface Future
{
    public function get(int|float|null $timeout = null): mixed;

    /** @return resource */
    public function getResource(): mixed;

    public function isReady(): bool;
}
```

`get()` blocks until the result arrives, then returns it. `$timeout` is in seconds. It throws
whatever the blocking call would have thrown, plus `Cassandra\Exception\TimeoutException` when the
wait expires. Calling `get()` twice returns the same result.

### Implementations

| Class | `get()` returns | Produced by |
| --- | --- | --- |
| `Cassandra\FutureRows` | `Cassandra\Rows` | `Session::executeAsync()` |
| `Cassandra\FutureSession` | `Cassandra\Session` | `Cluster::connectAsync()` |
| `Cassandra\FuturePreparedStatement` | `Cassandra\PreparedStatement` | `Session::prepareAsync()` |
| `Cassandra\FutureClose` | `null` | `Session::closeAsync()` |
| `Cassandra\FutureValue` | The wrapped value | Internal |

```php
$future = $session->executeAsync($statement, ['arguments' => [$id]]);

try {
    $rows = $future->get(2.5);
} catch (Cassandra\Exception\TimeoutException $e) {
    // The result did not arrive in time.
}
```

### Awaiting without blocking

`getResource()` returns a stream that becomes readable when the future resolves, and `isReady()`
answers the same question without one. When the stream is readable, `get()` does no network wait.

That is what event loop integration is built on: framework adapters, the shared
`Cassandra\Async\Reactor`, and native `Io\Poll` support on PHP 8.6. See
[async and event loops](/reference/async) for the API and
[event loops](/guide/event-loops) for the guide.

### Limits

`get()` blocks the PHP process. The C driver resolves futures on its own IO threads, so many
requests really do run at once, but:

- Between `executeAsync()` and `get()`, PHP can run other PHP code, not other requests.
- The result still decodes on the PHP thread, whichever model you wait with.

See [asynchronous queries](/guide/async).

## `Cassandra\Schema`

```php
interface Schema
{
    public function keyspace(string $name): Keyspace|false;
    public function keyspaces(): array;
}
```

Returned by `Session::schema()`. See [schema metadata](/guide/schema-metadata).

## `Cassandra\Keyspace`

| Method | Returns |
| --- | --- |
| `name()` | `string` |
| `replicationClassName()` | `string` |
| `replicationOptions()` | `array<string, mixed>` |
| `hasDurableWrites()` | `bool` |
| `table(string $name)` | `Table` or `false` |
| `tables()` | `array<string, Table>` |
| `userType(string $name)` | `Cassandra\Type\UserType` or `null` |
| `userTypes()` | `array<string, Cassandra\Type\UserType>` |
| `materializedView(string $name)` | `MaterializedView` or `false` |
| `materializedViews()` | `array<string, MaterializedView>` |
| `function(string $name, mixed ...$types)` | `Cassandra\Function_` or `false` |
| `functions()` | `array<string, Cassandra\Function_>` |
| `aggregate(string $name, mixed ...$types)` | `Aggregate` or `false` |
| `aggregates()` | `array<string, Aggregate>` |

## `Cassandra\Table`

| Method | Returns |
| --- | --- |
| `name()` | `string` |
| `option(string $name)` | `mixed` |
| `options()` | `array<string, mixed>` |
| `comment()` | `string` or `false` |
| `column(string $name)` | `Column` or `false` |
| `columns()` | `array<string, Column>` |
| `partitionKey()` | `array<int, Column>` |
| `clusteringKey()` | `array<int, Column>` |
| `primaryKey()` | `array<int, Column>` |
| `clusteringOrder()` | `array<int, string>` |
| `defaultTTL()` | `int` or `false` |
| `gcGraceSeconds()` | `int` or `false` |
| `caching()` | `string` or `false` |
| `bloomFilterFPChance()` | `float` or `false` |
| `compactionStrategyClassName()` | `string` or `false` |
| `compactionStrategyOptions()` | `array` or `false` |
| `compressionParameters()` | `array` or `false` |
| `speculativeRetry()` | `string` or `false` |
| `memtableFlushPeriodMs()` | `int` or `false` |
| `minIndexInterval()` | `int` or `false` |
| `maxIndexInterval()` | `int` or `false` |
| `indexInterval()` | `int` or `false` |
| `readRepairChance()` | `float` or `false` |
| `localReadRepairChance()` | `float` or `false` |
| `populateIOCacheOnFlush()` | `bool` or `false` |
| `replicateOnWrite()` | `bool` or `false` |

`Cassandra\MaterializedView` extends `Cassandra\Table` and adds `baseTable(): ?Table`.

## `Cassandra\Column`

| Method | Returns |
| --- | --- |
| `name()` | `string` |
| `type()` | `Cassandra\Type` or `null` |
| `isReversed()` | `bool` |
| `isStatic()` | `bool` |
| `isFrozen()` | `bool` |
| `indexName()` | `string` or `null` |
| `indexOptions()` | `string` or `null` |
