# Retry policies

A retry policy decides what the driver does when a request fails with a timeout, an unavailable
error, or a read error. It runs inside the C driver, before the exception reaches PHP.

```php
$cluster = Cassandra::cluster()
    ->withRetryPolicy(new Cassandra\RetryPolicy\DefaultPolicy())
    ->build();
```

Set it per statement as well:

```php
$rows = $session->execute($statement, [
    'retry_policy' => new Cassandra\RetryPolicy\Fallthrough(),
]);
```

The statement value wins over the cluster value.

## The policies

### `DefaultPolicy`

```php
new Cassandra\RetryPolicy\DefaultPolicy()
```

The sensible baseline, and the behavior you get when you set no policy.

- **Read timeout** — retry once when enough replicas answered but the data did not arrive.
- **Write timeout** — retry once, and only for a batch log write, where a retry is safe.
- **Unavailable** — retry once on the next host.

It never lowers the consistency level. A request that cannot meet the requested consistency fails.

### `Fallthrough`

```php
new Cassandra\RetryPolicy\Fallthrough()
```

Never retries. Every error goes straight to PHP as an exception.

Use it when the application owns the retry decision, for example when a job queue already retries
with backoff, or when the write is not idempotent and a duplicate would be wrong.

### `Logging`

```php
new Cassandra\RetryPolicy\Logging(new Cassandra\RetryPolicy\DefaultPolicy())
```

A decorator. It wraps another policy and writes a log line on every retry decision, then delegates.
The output goes to the driver log configured by `cassandra.log`. See
[metrics and logging](/guide/observability).

Use it to find out how often retries happen in production. It changes no behavior.

### `DowngradingConsistency` (deprecated)

```php
new Cassandra\RetryPolicy\DowngradingConsistency()   // deprecated
```

Retries with a lower consistency level when the requested level cannot be met.

::: danger Do not use this in new code
The class is deprecated. A silent downgrade means a query you believe ran at `LOCAL_QUORUM`
actually ran at `ONE`, and you cannot tell from the result. In a degraded cluster it turns a visible
failure into silent stale data.

Set the consistency each query really needs instead, and let the failure surface.
:::

## Choosing

| Situation | Policy |
| --- | --- |
| Normal application traffic | `DefaultPolicy` |
| Non-idempotent writes | `Fallthrough`, retry in the application |
| Investigating retry rates | `Logging` around `DefaultPolicy` |
| An external system already retries | `Fallthrough` |

## Retries and idempotency

A retry re-sends the same statement. That is safe for a read, and for a write that produces the same
result when applied twice.

Not idempotent, and therefore not safe to retry blindly:

```sql
UPDATE counters SET hits = hits + 1 WHERE id = ?;         -- counter increment
UPDATE users SET tags = tags + {'new'} WHERE id = ?;       -- collection append
INSERT INTO users (id, email) VALUES (?, ?) IF NOT EXISTS; -- lightweight transaction
```

The driver cannot tell which is which. For those statements, use `Fallthrough` and handle the failure
where you know whether a duplicate matters.

## What a retry does not fix

A retry policy handles a failed request. It does not handle:

- **A wrong query.** `InvalidQueryException` and `InvalidSyntaxException` are never retried.
- **Authorization.** `UnauthorizedException` is final.
- **An overloaded cluster.** Retries add load. Use backpressure in the application.

See [error handling](/guide/error-handling) for what reaches PHP after the policy runs.

## Application-level retry

When you need control over backoff, turn driver retries off and write the loop yourself:

```php
use Cassandra\Exception\ReadTimeoutException;
use Cassandra\Exception\UnavailableException;

function executeWithBackoff(
    Cassandra\Session $session,
    Cassandra\Statement $statement,
    array $options = [],
    int $attempts = 3,
): Cassandra\Rows {
    $options['retry_policy'] = new Cassandra\RetryPolicy\Fallthrough();

    for ($i = 1; ; $i++) {
        try {
            return $session->execute($statement, $options);
        } catch (ReadTimeoutException | UnavailableException $e) {
            if ($i >= $attempts) {
                throw $e;
            }
            usleep((int) (50_000 * 2 ** ($i - 1)));   // 50ms, 100ms, 200ms
        }
    }
}
```

Retry only the exceptions that a second attempt can fix. Let the rest propagate.
