# Statements

`Cassandra\Statement` is a marker interface. Three classes implement it.

| Class | Constructor | Use |
| --- | --- | --- |
| `Cassandra\SimpleStatement` | public | One-off CQL, schema changes |
| `Cassandra\PreparedStatement` | private, from `Session::prepare()` | Everything on the request path |
| `Cassandra\BatchStatement` | public | Atomicity across statements |

## `Cassandra\SimpleStatement`

```php
final class SimpleStatement implements Statement
{
    public function __construct(string $cql) {}
}
```

```php
$statement = new Cassandra\SimpleStatement('SELECT * FROM users LIMIT 10');
$rows = $session->execute($statement);
```

A plain string passed to `execute()` is wrapped in a `SimpleStatement` automatically, so these two
lines are the same:

```php
$session->execute('SELECT * FROM users LIMIT 10');
$session->execute(new Cassandra\SimpleStatement('SELECT * FROM users LIMIT 10'));
```

A simple statement carries no partition key metadata, so
[token-aware routing](/guide/load-balancing#token-aware-routing) cannot work. Every request goes
through a coordinator that then forwards it.

## `Cassandra\PreparedStatement`

```php
final class PreparedStatement implements Statement
{
    private function __construct() {}
}
```

`Session::prepare()` is the only way to obtain one.

```php
$statement = $session->prepare('INSERT INTO users (id, email) VALUES (?, ?)');

$session->execute($statement, ['arguments' => [$id, $email]]);
```

The object holds no state beyond the server identifier and the marker types, so one instance is
reusable across any number of executions and any number of value sets.

Prepared statements are tied to the session that prepared them.

## `Cassandra\BatchStatement`

```php
final class BatchStatement implements Statement
{
    public function __construct(int $type = Cassandra::BATCH_LOGGED) {}

    public function add(string|Statement $statement, ?array $arguments = null): static {}
}
```

```php
$batch = new Cassandra\BatchStatement(Cassandra::BATCH_UNLOGGED);

$batch->add($insertUser, [$id, $email])
      ->add($insertIndex, [$email, $id]);

$session->execute($batch);
```

### `add()`

`$statement` is a CQL string, a `SimpleStatement`, or a `PreparedStatement`. Another
`BatchStatement` is not allowed.

`$arguments` is an array of bound values. Integer keys bind by position, string keys bind by name.

Returns the batch, so calls chain.

### Batch types

| Constant | Meaning |
| --- | --- |
| `Cassandra::BATCH_LOGGED` | Atomic across partitions, at the cost of a batch log write |
| `Cassandra::BATCH_UNLOGGED` | No batch log. Correct when every statement hits one partition |
| `Cassandra::BATCH_COUNTER` | Required for counter updates. Cannot mix with normal writes |

See [batches](/guide/batches).

## `Cassandra\ExecutionOptions`

```php
final class ExecutionOptions
{
    /** @deprecated */
    public function __construct(array $options) {}

    public function __get(string $name): mixed {}
}
```

::: warning Deprecated
Pass a plain array to `execute()` instead. The class remains for compatibility with the old DataStax
driver.
:::

The constructor takes the same snake_case keys that `execute()` takes. The magic getter reads them
back in camel case:

| Constructor key | Property |
| --- | --- |
| `consistency` | `$options->consistency` |
| `serial_consistency` | `$options->serialConsistency` |
| `page_size` | `$options->pageSize` |
| `paging_state_token` | `$options->pagingStateToken` |
| `timeout` | `$options->timeout` |
| `arguments` | `$options->arguments` |
| `retry_policy` | `$options->retryPolicy` |
| `timestamp` | `$options->timestamp` |

## `Cassandra\RetryPolicy`

A marker interface with four implementations.

| Class | Behavior |
| --- | --- |
| `Cassandra\RetryPolicy\DefaultPolicy` | Retry once on a read timeout, a batch log write timeout, or an unavailable error |
| `Cassandra\RetryPolicy\Fallthrough` | Never retry |
| `Cassandra\RetryPolicy\Logging` | Wrap another policy and log every decision |
| `Cassandra\RetryPolicy\DowngradingConsistency` | Deprecated. Retry at a lower consistency |

```php
new Cassandra\RetryPolicy\Logging(new Cassandra\RetryPolicy\DefaultPolicy());
```

See [retry policies](/guide/retry-policies).

## `Cassandra\TimestampGenerator`

A marker interface with two implementations.

| Class | Behavior |
| --- | --- |
| `Cassandra\TimestampGenerator\Monotonic` | Client-side, strictly increasing within one process |
| `Cassandra\TimestampGenerator\ServerSide` | The coordinator assigns the timestamp. The default |

```php
$cluster = Cassandra::cluster()
    ->withTimestampGenerator(new Cassandra\TimestampGenerator\Monotonic())
    ->build();
```

Set a timestamp for one statement with the `timestamp` execution option, in microseconds. See
[explicit write timestamps](/guide/queries#explicit-write-timestamps).
