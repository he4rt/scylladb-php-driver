# Constants

All constants live on the global `Cassandra` class.

## Version

```php
Cassandra::VERSION;              // the extension version, for example '1.4.1'
Cassandra::CPP_DRIVER_VERSION;   // the linked C/C++ driver version, resolved at runtime
```

```php
printf("driver %s on cpp-driver %s\n", Cassandra::VERSION, Cassandra::CPP_DRIVER_VERSION);
```

## Consistency levels

| Constant | Replicas that must answer |
| --- | --- |
| `Cassandra::CONSISTENCY_ANY` | Any node, including a hinted handoff. Writes only |
| `Cassandra::CONSISTENCY_ONE` | One replica, any datacenter |
| `Cassandra::CONSISTENCY_TWO` | Two replicas |
| `Cassandra::CONSISTENCY_THREE` | Three replicas |
| `Cassandra::CONSISTENCY_QUORUM` | A quorum across all datacenters |
| `Cassandra::CONSISTENCY_ALL` | Every replica |
| `Cassandra::CONSISTENCY_LOCAL_ONE` | One replica in the local datacenter |
| `Cassandra::CONSISTENCY_LOCAL_QUORUM` | A quorum inside the local datacenter |
| `Cassandra::CONSISTENCY_EACH_QUORUM` | A quorum in every datacenter. Writes only |
| `Cassandra::CONSISTENCY_SERIAL` | For a lightweight transaction, across datacenters |
| `Cassandra::CONSISTENCY_LOCAL_SERIAL` | For a lightweight transaction, local datacenter |

The cluster default is `CONSISTENCY_LOCAL_ONE`. Most applications set `CONSISTENCY_LOCAL_QUORUM`.

```php
$cluster = Cassandra::cluster()
    ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
    ->build();

$rows = $session->execute($statement, [
    'consistency' => Cassandra::CONSISTENCY_LOCAL_ONE,
]);
```

`CONSISTENCY_SERIAL` and `CONSISTENCY_LOCAL_SERIAL` belong in the `serial_consistency` option, not
in `consistency`. See
[lightweight transactions](/guide/queries#lightweight-transactions).

## TLS verification flags

| Constant | Meaning |
| --- | --- |
| `Cassandra::VERIFY_NONE` | Encrypt only. Do not check the server certificate |
| `Cassandra::VERIFY_PEER_CERT` | Check the certificate against the trusted certificates |
| `Cassandra::VERIFY_PEER_IDENTITY` | Also check that the identity matches the peer |

Combine them with the bitwise `or` operator:

```php
Cassandra::ssl()
    ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY)
    ->build();
```

See [TLS and SSL](/guide/tls).

## Batch types

| Constant | Meaning |
| --- | --- |
| `Cassandra::BATCH_LOGGED` | Atomic across partitions, at the cost of a batch log write |
| `Cassandra::BATCH_UNLOGGED` | No batch log |
| `Cassandra::BATCH_COUNTER` | Required for counter updates |

```php
new Cassandra\BatchStatement(Cassandra::BATCH_UNLOGGED);
```

See [batches](/guide/batches).

## Log levels

| Constant | Value in `php.ini` |
| --- | --- |
| `Cassandra::LOG_DISABLED` | — |
| `Cassandra::LOG_CRITICAL` | `CRITICAL` |
| `Cassandra::LOG_ERROR` | `ERROR` |
| `Cassandra::LOG_WARN` | `WARN` |
| `Cassandra::LOG_INFO` | `INFO` |
| `Cassandra::LOG_DEBUG` | `DEBUG` |
| `Cassandra::LOG_TRACE` | `TRACE` |

The log level is set in `php.ini`, not in code:

```ini
cassandra.log_level = ERROR
```

See [metrics and logging](/guide/observability#logging).

## CQL type names

String constants used by the collection constructors.

| Constant | Value |
| --- | --- |
| `Cassandra::TYPE_ASCII` | `ascii` |
| `Cassandra::TYPE_TEXT` | `text` |
| `Cassandra::TYPE_VARCHAR` | `varchar` |
| `Cassandra::TYPE_TINYINT` | `tinyint` |
| `Cassandra::TYPE_SMALLINT` | `smallint` |
| `Cassandra::TYPE_INT` | `int` |
| `Cassandra::TYPE_BIGINT` | `bigint` |
| `Cassandra::TYPE_VARINT` | `varint` |
| `Cassandra::TYPE_COUNTER` | `counter` |
| `Cassandra::TYPE_FLOAT` | `float` |
| `Cassandra::TYPE_DOUBLE` | `double` |
| `Cassandra::TYPE_DECIMAL` | `decimal` |
| `Cassandra::TYPE_BOOLEAN` | `boolean` |
| `Cassandra::TYPE_BLOB` | `blob` |
| `Cassandra::TYPE_INET` | `inet` |
| `Cassandra::TYPE_UUID` | `uuid` |
| `Cassandra::TYPE_TIMEUUID` | `timeuuid` |
| `Cassandra::TYPE_TIMESTAMP` | `timestamp` |

```php
new Cassandra\Collection(Cassandra::TYPE_TEXT);
new Cassandra\Map(Cassandra::TYPE_UUID, Cassandra::TYPE_INT);
```

For a composite element type, use the [type factory](/reference/types) instead.

## Static methods

```php
Cassandra::cluster(): Cassandra\Cluster\Builder;
Cassandra::ssl(): Cassandra\SSLOptions\Builder;
```

These are the two entry points to the driver.
