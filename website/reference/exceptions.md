# Exceptions

Every driver exception implements the `Cassandra\Exception` marker interface and extends a matching
SPL exception.

```php
namespace Cassandra;

interface Exception {}
```

## The tree

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

## Reference

| Exception | Thrown when | Retry |
| --- | --- | --- |
| `InvalidArgumentException` | A bad argument reaches the driver: an unknown option, a negative page size, an unreadable certificate path, a malformed address | No |
| `DivideByZeroException` | A numeric value class divides by zero | No |
| `LogicException` | An operation is not valid for the object state | No |
| `DomainException` | A value is outside the valid domain | No |
| `RangeException` | A numeric conversion is out of range | No |
| `RuntimeException` | A driver error with no more specific class | Depends |
| `TimeoutException` | The client wait expired. The request may still be running on the server | Maybe |
| `AuthenticationException` | The credentials were rejected | No |
| `ProtocolException` | A protocol-level disagreement with the server | No |
| `ReadTimeoutException` | Not enough replicas answered a read in time | Yes |
| `WriteTimeoutException` | Not enough replicas acknowledged a write in time. **The write may have applied** | Only if idempotent |
| `UnavailableException` | Not enough live replicas to meet the consistency level | After a delay |
| `TruncateException` | A `TRUNCATE` failed | Yes |
| `InvalidQueryException` | The table or column does not exist, or the query is not valid | No |
| `InvalidSyntaxException` | The CQL does not parse | No |
| `UnauthorizedException` | The role lacks permission | No |
| `UnpreparedException` | The coordinator does not know the prepared statement identifier | The driver handles it |
| `ConfigurationException` | A schema statement is not valid | No |
| `AlreadyExistsException` | A `CREATE` for something that exists | No |
| `IsBootstrappingException` | The node is still joining the cluster | After a delay |
| `OverloadedException` | The node rejected the request under load | After a delay |

## Codes

Server-side exceptions carry the C driver error code in `getCode()`, and the driver message in
`getMessage()`.

```php
catch (Cassandra\Exception $e) {
    $logger->error('cassandra error', [
        'class'   => $e::class,
        'code'    => $e->getCode(),
        'message' => $e->getMessage(),
    ]);
}
```

## Catching by branch

The three branches under `RuntimeException` group the exceptions by what you should do about them.

```php
try {
    $session->execute($statement, $options);
} catch (Cassandra\Exception\ValidationException $e) {
    // Your bug. Fix the query or the grant. Do not retry.
    throw $e;
} catch (Cassandra\Exception\ExecutionException $e) {
    // The cluster could not complete it. Retry when the statement is idempotent.
    $this->scheduleRetry();
} catch (Cassandra\Exception\ServerException $e) {
    // A node is not in a state to serve. Back off.
    $this->backOff();
} catch (Cassandra\Exception $e) {
    throw $e;
}
```

See [error handling](/guide/error-handling) for the full guidance.
