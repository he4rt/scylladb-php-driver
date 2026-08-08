# Error handling

Every exception the driver throws implements `Cassandra\Exception`. Each one also extends a matching
SPL exception, so existing `catch (RuntimeException $e)` blocks keep working.

```php
try {
    $rows = $session->execute($statement, ['arguments' => [$id]]);
} catch (Cassandra\Exception $e) {
    // Any driver error.
}
```

## The hierarchy

```
Cassandra\Exception  (interface)
│
├── Cassandra\Exception\LogicException            extends \LogicException
├── Cassandra\Exception\DomainException           extends \DomainException
├── Cassandra\Exception\InvalidArgumentException  extends \InvalidArgumentException
├── Cassandra\Exception\RangeException            extends \RangeException
│   └── Cassandra\Exception\DivideByZeroException
│
└── Cassandra\Exception\RuntimeException          extends \RuntimeException
    ├── Cassandra\Exception\TimeoutException
    ├── Cassandra\Exception\AuthenticationException
    ├── Cassandra\Exception\ProtocolException
    │
    ├── Cassandra\Exception\ExecutionException
    │   ├── Cassandra\Exception\ReadTimeoutException
    │   ├── Cassandra\Exception\WriteTimeoutException
    │   ├── Cassandra\Exception\UnavailableException
    │   └── Cassandra\Exception\TruncateException
    │
    ├── Cassandra\Exception\ValidationException
    │   ├── Cassandra\Exception\InvalidQueryException
    │   ├── Cassandra\Exception\InvalidSyntaxException
    │   ├── Cassandra\Exception\UnauthorizedException
    │   ├── Cassandra\Exception\UnpreparedException
    │   └── Cassandra\Exception\ConfigurationException
    │       └── Cassandra\Exception\AlreadyExistsException
    │
    └── Cassandra\Exception\ServerException
        ├── Cassandra\Exception\IsBootstrappingException
        └── Cassandra\Exception\OverloadedException
```

The three branches under `RuntimeException` split by cause, which is what decides your response:

| Branch | Cause | Retry? |
| --- | --- | --- |
| `ExecutionException` | The cluster could not complete the request | Often yes |
| `ValidationException` | The request is wrong | No |
| `ServerException` | A node is not in a state to serve | Yes, after a delay |

## The exceptions that matter

### `ReadTimeoutException`

Not enough replicas answered a read in time. The data may be fine. A retry commonly succeeds, and
the [default retry policy](/guide/retry-policies) already tried once.

```php
catch (Cassandra\Exception\ReadTimeoutException $e) {
    // Safe to retry. Consider a lower consistency for this specific read.
}
```

### `WriteTimeoutException`

The coordinator did not get enough acknowledgements in time. **The write may have applied.** A
timeout is not a rejection.

```php
catch (Cassandra\Exception\WriteTimeoutException $e) {
    // Retry only when the write is idempotent.
    // A counter increment or a collection append must not be retried blindly.
}
```

### `UnavailableException`

The coordinator knows there are not enough live replicas to meet the consistency level. It did not
even try. Retrying at once fails again.

```php
catch (Cassandra\Exception\UnavailableException $e) {
    // A node is down. Back off, or degrade the consistency deliberately.
}
```

### `InvalidQueryException` and `InvalidSyntaxException`

A bug in the query, the table name, or the column list. Never retry. Let it reach your error
reporting.

### `AlreadyExistsException`

A `CREATE` for something that exists. Use `IF NOT EXISTS` in migrations to avoid it.

### `UnpreparedException`

The coordinator does not know the prepared statement identifier, usually because the node restarted.
The driver re-prepares and retries on its own. Seeing this in PHP means the recovery also failed.

### `OverloadedException` and `IsBootstrappingException`

The node is saturated or is still joining the cluster. Back off. Do not retry in a tight loop.

### `AuthenticationException`

Bad credentials. Thrown from `connect()`. See [authentication](/guide/authentication).

### `TimeoutException`

The client gave up waiting. The request may still be running on the server. This is the PHP-side
timeout from `execute()` or `Future::get()`, not a server timeout.

### `InvalidArgumentException`

A bad value passed to the driver: an unknown execution option, a negative page size, an unreadable
certificate path, a malformed IP address. Thrown before any network work.

### `DivideByZeroException`

From arithmetic on the [numeric value classes](/guide/data-types#numbers).

## Catch order

Catch from specific to general. PHP takes the first matching block.

```php
use Cassandra\Exception\InvalidQueryException;
use Cassandra\Exception\UnavailableException;
use Cassandra\Exception\WriteTimeoutException;

try {
    $session->execute($insert, ['arguments' => $values]);
} catch (WriteTimeoutException $e) {
    $this->queueForVerification($values);          // may or may not have applied
} catch (UnavailableException $e) {
    $this->scheduleRetry($values, delaySeconds: 5); // cluster is degraded
} catch (InvalidQueryException $e) {
    throw $e;                                       // a bug, do not swallow it
} catch (Cassandra\Exception $e) {
    $this->logger->error('cassandra write failed', ['error' => $e->getMessage()]);
    throw $e;
}
```

## What to retry

| Exception | Retry | Note |
| --- | --- | --- |
| `ReadTimeoutException` | Yes | Reads are idempotent |
| `WriteTimeoutException` | Only if idempotent | The write may have applied |
| `UnavailableException` | After a delay | Retrying at once fails again |
| `OverloadedException` | After a delay | Backoff is required |
| `IsBootstrappingException` | After a delay | The node is joining |
| `TruncateException` | Yes | |
| `InvalidQueryException` | No | Fix the query |
| `InvalidSyntaxException` | No | Fix the query |
| `UnauthorizedException` | No | Fix the grant |
| `AlreadyExistsException` | No | Use `IF NOT EXISTS` |
| `AuthenticationException` | No | Fix the credentials |
| `InvalidArgumentException` | No | Fix the call |

## Exception codes

Server-side exceptions carry the C driver error code in `getCode()`. The message comes from the
driver.

```php
catch (Cassandra\Exception $e) {
    $this->logger->error('cassandra error', [
        'class'   => $e::class,
        'code'    => $e->getCode(),
        'message' => $e->getMessage(),
    ]);
}
```

## A wrapper for your application

Driver exceptions crossing a domain boundary couple your code to the driver. Translate at the edge:

```php
final class UserRepository
{
    public function find(Cassandra\Uuid $id): ?User
    {
        try {
            $row = $this->session
                ->execute($this->findById, ['arguments' => [$id]])
                ->first();
        } catch (Cassandra\Exception\ValidationException $e) {
            throw new StorageBugException($e->getMessage(), previous: $e);
        } catch (Cassandra\Exception $e) {
            throw new StorageUnavailableException($e->getMessage(), previous: $e);
        }

        return $row === null ? null : User::fromRow($row);
    }
}
```

The split matters. `ValidationException` is your bug and belongs in the error tracker.
Everything else is an infrastructure condition and belongs in a retry or a circuit breaker.
