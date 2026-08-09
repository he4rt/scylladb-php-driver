# Queries and statements

`Session::execute()` runs one statement and returns [`Cassandra\Rows`](/guide/results).

```php
public function execute(
    string|Statement $statement,
    array|ExecutionOptions|null $options = null,
): Rows;
```

The first argument is a CQL string or a `Cassandra\Statement`. The second holds the bound values and
the per-query settings.

## Simple statements

A plain string is the shortest form. The driver wraps it in a `Cassandra\SimpleStatement`.

```php
$rows = $session->execute('SELECT id, email FROM users LIMIT 10');
```

Build the object yourself when you want to keep and reuse it:

```php
$statement = new Cassandra\SimpleStatement('SELECT id, email FROM users LIMIT 10');
$rows = $session->execute($statement);
```

A simple statement can take bound values too:

```php
$rows = $session->execute(
    'SELECT * FROM users WHERE id = ?',
    ['arguments' => [$id]],
);
```

::: warning Simple statements defeat token-aware routing
A simple statement carries no partition key metadata, so the driver cannot route it to a replica.
The request goes to an arbitrary coordinator, which then forwards it. Prepare any statement you run
more than once.
:::

Use simple statements for schema changes and one-off administrative queries. Use prepared statements
for everything on the request path.

## Prepared statements

`prepare()` sends the CQL to the cluster once. The server answers with an identifier and the type of
every marker.

```php
$insert = $session->prepare(
    'INSERT INTO users (id, email, created_at) VALUES (?, ?, ?)'
);

$session->execute($insert, [
    'arguments' => [$id, 'ada@example.com', new Cassandra\Timestamp(time())],
]);
```

Preparing buys three things:

1. **Token-aware routing.** The driver knows the partition key, so it reaches a replica directly.
2. **Type checking on the client.** Marker types are known, so a wrong value fails before the
   network call.
3. **A smaller request.** Later executions send the identifier and the values, not the CQL text.

Prepare each statement once and keep the object:

```php
final class UserRepository
{
    private Cassandra\PreparedStatement $findById;
    private Cassandra\PreparedStatement $insert;

    public function __construct(private Cassandra\Session $session)
    {
        $this->findById = $session->prepare('SELECT * FROM users WHERE id = ?');
        $this->insert   = $session->prepare('INSERT INTO users (id, email) VALUES (?, ?)');
    }

    public function find(Cassandra\Uuid $id): ?array
    {
        return $this->session->execute($this->findById, ['arguments' => [$id]])->first();
    }
}
```

`Cassandra\PreparedStatement` has a private constructor. `prepare()` is the only way to get one.

::: tip Do not prepare in a loop
`prepare()` is a network round trip. Preparing the same CQL on every request adds a round trip to
every request.
:::

Prepare without blocking with `prepareAsync()`, which returns a
`Cassandra\FuturePreparedStatement`. See [asynchronous queries](/guide/async).

## Binding values

Values go in the `arguments` key of the options array.

### By position

An array with integer keys binds by position. Markers are `?`.

```php
$statement = $session->prepare(
    'INSERT INTO users (id, email, created_at) VALUES (?, ?, ?)'
);

$session->execute($statement, [
    'arguments' => [$id, 'ada@example.com', new Cassandra\Timestamp(time())],
]);
```

### By name

An array with string keys binds by name. Markers are `:name`.

```php
$statement = $session->prepare(
    'INSERT INTO users (id, email, created_at) VALUES (:id, :email, :created_at)'
);

$session->execute($statement, [
    'arguments' => [
        'id'         => $id,
        'email'      => 'ada@example.com',
        'created_at' => new Cassandra\Timestamp(time()),
    ],
]);
```

Named binding does not depend on the order of the markers, which makes a long insert easier to read
and safer to change.

### Never concatenate CQL

```php
// Wrong. This is a CQL injection.
$session->execute("SELECT * FROM users WHERE email = '$email'");

// Right.
$session->execute($findByEmail, ['arguments' => [$email]]);
```

Bound values travel as typed protocol values. They are never parsed as CQL.

### Null values

```php
$session->execute($statement, ['arguments' => [$id, null]]);
```

A PHP `null` binds as a CQL `null`, which deletes the cell. That is not the same as leaving the
column out of the insert, which writes nothing at all.

## Execution options

The second argument to `execute()` accepts a plain array. Each key is optional.

| Key | Type | Purpose |
| --- | --- | --- |
| `arguments` | `array` | Values for the markers, by position or by name. |
| `consistency` | `int` | A `Cassandra::CONSISTENCY_*` constant. Overrides the cluster default. |
| `serial_consistency` | `int` | `CONSISTENCY_SERIAL` or `CONSISTENCY_LOCAL_SERIAL`, for lightweight transactions. |
| `page_size` | `int` | Rows per page. Must be greater than zero. |
| `paging_state_token` | `string` | Resume from a token returned by a previous page. |
| `timeout` | `int` or `float` | Seconds to wait. Must be greater than zero, or `null`. |
| `retry_policy` | `Cassandra\RetryPolicy` | Overrides the cluster policy for this statement. |
| `timestamp` | `int` or numeric string | An explicit write timestamp, in microseconds. |

```php
$rows = $session->execute($statement, [
    'arguments'   => [$id],
    'consistency' => Cassandra::CONSISTENCY_LOCAL_QUORUM,
    'page_size'   => 500,
    'timeout'     => 2.5,
]);
```

An invalid value raises `Cassandra\Exception\InvalidArgumentException` and names the key.

::: warning The ExecutionOptions constructor is deprecated
`new Cassandra\ExecutionOptions([...])` still works and still reads its values through magic
properties in camel case (`$options->pageSize`). Pass a plain array in new code.
:::

## Lightweight transactions

A conditional write returns a result set with an `[applied]` column instead of an empty result.
Read that column with `wasApplied()`.

```php
$statement = $session->prepare(
    'INSERT INTO users (id, email) VALUES (?, ?) IF NOT EXISTS'
);

$rows = $session->execute($statement, [
    'arguments'          => [$id, 'ada@example.com'],
    'serial_consistency' => Cassandra::CONSISTENCY_LOCAL_SERIAL,
]);

if (! $rows->wasApplied()) {
    // The write lost. $rows->first() holds the current values of the row.
    $current = $rows->first();
}
```

`wasApplied()` reads the `[applied]` column of the first row. A statement with no condition has no
such column, and the method then returns `true`. You can call it on any result.

::: warning `wasApplied()` is not `exists()`
`false` means the condition failed, not that the row is present. After `IF NOT EXISTS` a `false`
tells you the row **is** there. After `IF EXISTS` the same `false` tells you the row **is not**
there. The meaning comes from your condition.
:::

The `[applied]` column stays readable through `first()` and array access, so old code keeps working.

Lightweight transactions use Paxos and cost several round trips. Use them where correctness needs
them, not as a general concurrency tool.

## Explicit write timestamps

```php
$session->execute($statement, [
    'arguments' => [$id, $email],
    'timestamp' => (int) (microtime(true) * 1_000_000),
]);
```

The value is in microseconds. It sets the cell timestamp that decides which write wins. Supply it
when you replay events and need the original order preserved.

For automatic timestamps generated on the client, set a generator on the cluster:

```php
->withTimestampGenerator(new Cassandra\TimestampGenerator\Monotonic())
```

`Monotonic` guarantees a strictly increasing value inside one process, which prevents two writes in
the same microsecond from getting the same timestamp. `ServerSide` leaves the timestamp to the
coordinator, which is the default behavior.

## Asynchronous execution

```php
$future = $session->executeAsync($statement, ['arguments' => [$id]]);
$rows   = $future->get();
```

`executeAsync()` returns immediately with a `Cassandra\FutureRows`. See
[asynchronous queries](/guide/async).

## Next

- [Batches](/guide/batches) — group several statements into one request.
- [Results and paging](/guide/results) — read the result set.
- [Data types](/guide/data-types) — what to bind for each CQL type.
