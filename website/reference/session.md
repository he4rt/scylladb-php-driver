# Cluster and Session

## `Cassandra\Cluster`

An interface. `Cassandra\Cluster\Builder::build()` returns the `Cassandra\DefaultCluster`
implementation. A cluster holds configuration only. It opens no sockets.

```php
interface Cluster
{
    public function connect(?string $keyspace = null, ?int $timeout = null): Session;
    public function connectAsync(?string $keyspace = null): Future;
}
```

### `connect()`

```php
$session = $cluster->connect();            // no keyspace
$session = $cluster->connect('shop');      // set the keyspace
$session = $cluster->connect('shop', 10);  // 10 second timeout
```

Opens the connection pool and returns a `Cassandra\Session`. Blocks until the pool is ready.

`$timeout` is in **seconds**, as an integer.

Pass `null` rather than an empty string when there is no keyspace. An empty string produces an
invalid `USE` statement.

Throws:

| Exception | Cause |
| --- | --- |
| `Cassandra\Exception\RuntimeException` | No contact point could be reached |
| `Cassandra\Exception\AuthenticationException` | The credentials were rejected |
| `Cassandra\Exception\InvalidQueryException` | The keyspace does not exist |

### `connectAsync()`

```php
$future  = $cluster->connectAsync('shop');
$session = $future->get(10.0);
```

Returns a `Cassandra\FutureSession`. See [asynchronous queries](/guide/async).

## `Cassandra\Session`

An interface. `connect()` returns the `Cassandra\DefaultSession` implementation.

```php
interface Session
{
    public function execute(string|Statement $statement, array|ExecutionOptions|null $options = null, string|\UnitEnum|null $executionProfile = null): Rows;
    public function executeAsync(string|Statement $statement, array|ExecutionOptions|null $options = null, string|\UnitEnum|null $executionProfile = null): FutureRows;
    public function prepare(string $cql, array|ExecutionOptions|null $options = null): PreparedStatement;
    public function prepareAsync(string $cql, array|ExecutionOptions|null $options = null): FuturePreparedStatement;
    public function close(int|float|null $timeout = null): void;
    public function closeAsync(): FutureClose;
    public function metrics(): array;
    public function schema(): Schema;
}
```

### `execute()`

```php
$rows = $session->execute($statement, $options, $executionProfile);
```

Runs one statement and returns one page of results. `$statement` is a CQL string, a
`Cassandra\SimpleStatement`, a `Cassandra\PreparedStatement`, or a `Cassandra\BatchStatement`.

`$options` is an array. Every key is optional:

| Key | Type | Default |
| --- | --- | --- |
| `arguments` | `array` | none |
| `consistency` | `int` | The cluster default |
| `serial_consistency` | `int` | none |
| `page_size` | `int` | The cluster default, 5000 |
| `paging_state_token` | `string` | none |
| `timeout` | `int` or `float` | The cluster default |
| `retry_policy` | `Cassandra\RetryPolicy` | The cluster policy |
| `timestamp` | `int` or numeric string | Server side |

`$executionProfile` selects a profile registered with
[`withExecutionProfile()`](/reference/cluster-builder). It takes a string or an enum case, and `null`
uses the cluster settings. See [execution profiles](/guide/execution-profiles).

```php
$rows = $session->execute($statement, null, 'analytics');
$rows = $session->execute($statement, null, Profile::Analytics);
```

See [queries and statements](/guide/queries).

### `executeAsync()`

```php
$future = $session->executeAsync($statement, $options, $executionProfile);
$rows   = $future->get();
```

Returns a `Cassandra\FutureRows` at once. Same options as `execute()`.

### `prepare()`

```php
$statement = $session->prepare('SELECT * FROM users WHERE id = ?');
```

Sends the CQL to the cluster and returns a `Cassandra\PreparedStatement`. One network round trip.
Prepare once, keep the object.

### `prepareAsync()`

```php
$future    = $session->prepareAsync('SELECT * FROM users WHERE id = ?');
$statement = $future->get();
```

Returns a `Cassandra\FuturePreparedStatement`.

### `close()`

```php
$session->close();
$session->close(5.0);   // wait at most 5 seconds
```

Closes the connection pool. `$timeout` is in seconds.

The session closes at script shutdown as well. Call `close()` when you want the sockets released at
a known point.

With [persistent sessions](/guide/connecting#persistent-sessions) on, the underlying cluster stays
cached in the worker process, so a later `connect()` with the same configuration is cheap.

### `closeAsync()`

```php
$future = $session->closeAsync();
$future->get();
```

Returns a `Cassandra\FutureClose`.

### `metrics()`

```php
$m = $session->metrics();

$m['requests']['median'];              // microseconds
$m['requests']['p99'];
$m['requests']['m1_rate'];             // requests per second
$m['stats']['total_connections'];
$m['stats']['available_connections'];
$m['errors']['request_timeouts'];
```

Reads process memory. No network call. The full key list is in
[metrics and logging](/guide/observability#metrics).

### `schema()`

```php
$keyspace = $session->schema()->keyspace('shop');
$table    = $keyspace->table('users');
```

Returns the live schema metadata. See [schema metadata](/guide/schema-metadata).

## Lifetime

| Object | Cost to create | Lifetime |
| --- | --- | --- |
| `Cluster\Builder` | Free | Local to the configuration function |
| `Cluster` | Free | Free to keep, holds no sockets |
| `Session` | Expensive: handshake, auth, schema fetch | One per process, or per worker |
| `PreparedStatement` | One round trip | For as long as the session lives |

See [performance](/guide/performance#1-reuse-the-session).
