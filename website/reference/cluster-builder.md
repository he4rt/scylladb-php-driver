# `Cassandra\Cluster\Builder`

Obtained from `Cassandra::cluster()`. Every method returns the builder, so calls chain. `build()`
produces an immutable `Cassandra\Cluster`.

```php
$cluster = Cassandra::cluster()
    ->withContactPoints('10.0.0.1')
    ->build();
```

## Every method

| Method | Parameters | Default | Description |
| --- | --- | --- | --- |
| `withContactPoints` | `string ...$host` | `127.0.0.1` | Addresses the driver dials first |
| `withPort` | `int $port` | `9042` | The CQL native port on every node |
| `withCredentials` | `string $username, string $password` | none | Password authentication. The password is a `SensitiveParameter` |
| `withSSL` | `Cassandra\SSLOptions $options` | none | TLS settings from `Cassandra::ssl()` |
| `withDefaultConsistency` | `int $consistency` | `CONSISTENCY_LOCAL_QUORUM` | Consistency for statements that set none |
| `withDefaultPageSize` | `int $pageSize` | `5000` | Rows per page for statements that set none |
| `withDefaultTimeout` | `float $timeout` | none | Seconds. Timeout for statements that set none |
| `withConnectTimeout` | `float $timeout` | `5.0` | Seconds. Bounds one connection handshake |
| `withRequestTimeout` | `float $timeout` | `12.0` | Seconds. The hard ceiling on one request |
| `withRoundRobinLoadBalancingPolicy` | — | active | Cycle through every node in the cluster |
| `withDatacenterAwareRoundRobinLoadBalancingPolicy` | `string $localDatacenter, int $hostPerRemoteDatacenter, bool $useRemoteDatacenterForLocalConsistencies` | — | Prefer the local datacenter. All three arguments are required |
| `withRackAwareLoadBalancingPolicy` | `string $localDatacenter = '', string $localRack = ''` | — | ScyllaDB only. Prefer the local rack, then the local datacenter. Empty strings let the driver infer both from the first contact point |
| `withTokenAwareRouting` | `bool $enabled = true` | `true` | Send a request straight to a replica that owns the data |
| `withLatencyAwareRouting` | `bool $enabled = true` | `true` | Push consistently slow nodes down the candidate list |
| `withWhiteListHosts` | `string ...$hosts` | none | Use only these nodes |
| `withBlackListHosts` | `string ...$hosts` | none | Never use these nodes |
| `withWhiteListDCs` | `string ...$dcs` | none | Use only these datacenters |
| `withBlackListDCs` | `string ...$dcs` | none | Never use these datacenters |
| `withConnectionsPerHost` | `int $core, int $max = 2` | `1`, `2` | Pool size per node. Each value must be 1 to 128 |
| `withIOThreads` | `int $count` | `1` | Event loop threads in the C driver. 1 to 128 |
| `withReconnectInterval` | `float $interval` | `2.0` | Seconds between attempts to reach a node that is down |
| `withConnectionHeartbeatInterval` | `float $interval` | `30.0` | Seconds between keepalive requests on an idle connection |
| `withExponentialReconnect` | `float $baseInterval, float $maxInterval` | — | Back off from base to max, in seconds, with jitter. Replaces the constant delay |
| `withApplicationName` | `string $name` | — | Reported as `APPLICATION_NAME` in `system.clients.client_options` |
| `withApplicationVersion` | `string $version` | — | Reported as `APPLICATION_VERSION` in the same map |
| `withConstantSpeculativeExecutionPolicy` | `float $delay, int $maxSpeculativeExecutions = 2` | off | Re-send a slow request to another replica after `$delay` seconds. Idempotent statements only |
| `withNoSpeculativeExecutionPolicy` | — | default | Turn speculative execution off |
| `withCoalesceDelay` | `int $microseconds` | `200` | How long the driver batches writes into one system call |
| `withNewRequestRatio` | `int $ratio` | `50` | Split IO thread time between new and outstanding requests, 1 to 100 |
| `withExecutionProfile` | `string\|\UnitEnum $name, ExecutionProfile $profile` | — | Register a named profile. Select it with the third argument of `execute()`. See [execution profiles](/guide/execution-profiles) |
| `withTCPNodelay` | `bool $enabled = true` | `true` | Disable Nagle's algorithm |
| `withTCPKeepalive` | `?float $delay` | disabled | Seconds. Pass `null` to disable |
| `withProtocolVersion` | `int $version` | `4` | CQL protocol version |
| `withPersistentSessions` | `bool $enabled = true` | `true` | Cache the session in the PHP worker process |
| `withSchemaMetadata` | `bool $enabled = true` | `true` | Fetch and track the schema. Required for token awareness |
| `withHostnameResolution` | `bool $enabled = true` | `false` | Resolve peer addresses to host names |
| `withRandomizedContactPoints` | `bool $enabled = true` | `true` | Shuffle the contact point order |
| `withRetryPolicy` | `Cassandra\RetryPolicy $policy` | `DefaultPolicy` | See [retry policies](/guide/retry-policies) |
| `withTimestampGenerator` | `Cassandra\TimestampGenerator $generator` | server side | Client-side write timestamps |
| `build` | — | — | Produce the `Cassandra\Cluster` |

All time values are seconds, given as a float. `0.25` is 250 milliseconds.

## Grouped by purpose

### Where to connect

```php
->withContactPoints('10.0.0.1', '10.0.0.2', '10.0.0.3')
->withPort(9042)
->withRandomizedContactPoints(true)
->withHostnameResolution(false)
```

See [clusters and sessions](/guide/connecting).

### Security

```php
->withCredentials('app_user', getenv('SCYLLA_PASSWORD'))
->withSSL($sslOptions)
```

See [authentication](/guide/authentication) and [TLS and SSL](/guide/tls).

### Which node gets the request

```php
->withDatacenterAwareRoundRobinLoadBalancingPolicy('eu-west-1', 0, false)
->withTokenAwareRouting(true)
->withLatencyAwareRouting(true)
->withWhiteListDCs('eu-west-1')
```

See [load balancing and routing](/guide/load-balancing).

### Pool and timeouts

```php
->withConnectionsPerHost(2, 8)
->withIOThreads(1)
->withConnectTimeout(5.0)
->withRequestTimeout(12.0)
->withReconnectInterval(2.0)
->withConnectionHeartbeatInterval(30.0)   // 30 seconds
->withTCPNodelay(true)
->withTCPKeepalive(null)
```

See [connection pool and timeouts](/guide/connection-tuning).

### Query defaults

```php
->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
->withDefaultPageSize(5000)
->withDefaultTimeout(10.0)
->withRetryPolicy(new Cassandra\RetryPolicy\DefaultPolicy())
->withTimestampGenerator(new Cassandra\TimestampGenerator\Monotonic())
```

Every one of these is overridable per statement. See
[queries and statements](/guide/queries#execution-options).

## Validation

Invalid values raise `Cassandra\Exception\InvalidArgumentException` at the call, not at `build()`.

| Method | Rule |
| --- | --- |
| `withDefaultPageSize` | 0 or greater |
| `withIOThreads` | 1 to 128 |
| `withConnectionsPerHost` | Each value 1 to 128 |
| `withProtocolVersion` | 1 or greater |
| `withDatacenterAwareRoundRobinLoadBalancingPolicy` | `hostPerRemoteDatacenter` 0 or greater |
| Every timeout | 0 or greater |

## Reading the configuration back

The builder exposes its state as read-only properties, which is convenient in tests.

```php
$builder = Cassandra::cluster()
    ->withContactPoints('10.0.0.1')
    ->withTokenAwareRouting(true);

get_object_vars($builder);
// ['contactPoints' => '10.0.0.1', 'useTokenAwareRouting' => true, ...]
```

The exposed names are: `contactPoints`, `loadBalancingPolicy`, `localDatacenter`,
`hostPerRemoteDatacenter`, `useRemoteDatacenterForLocalConsistencies`, `useTokenAwareRouting`,
`username`, `password`, `connectTimeout`, `requestTimeout`, `sslOptions`, `defaultConsistency`,
`defaultPageSize`, `defaultTimeout`, `usePersistentSessions`, `protocolVersion`, `ioThreads`,
`coreConnectionPerHost`, `maxConnectionsPerHost`, `reconnectInterval`, `latencyAwareRouting`,
`tcpNodelay`, `tcpKeepalive`, `retryPolicy`, `timestampGenerator`, `schemaMetadata`,
`blacklist_hosts`, `whitelist_hosts`, `blacklist_dcs`, `whitelist_dcs`, `hostnameResolution`,
`randomizedContactPoints`, and `connectionHeartbeatInterval`. The port is not among them.

::: warning The password is redacted, unless an operator turns that off
`password` returns `***`. `var_dump()`, `print_r()`, and framework debug panels therefore print the
placeholder, not the credential.

The `cassandra.expose_credentials` INI setting puts the real value back:

```ini
cassandra.expose_credentials = On
```

The setting is `PHP_INI_SYSTEM`, so `ini_set()` inside a request cannot turn the redaction off. Only
an operator can, through `php.ini` or `php -d`. Use it in development only.
:::

## Persistent session cache

With `withPersistentSessions(true)`, `build()` hashes the whole configuration and reuses a cached
cluster when the hash matches. Two builders with identical settings share one underlying cluster
inside a PHP worker process.

This is why the configuration must come from constants, not from per-request values. A configuration
that varies per request fills the cache. See [performance](/guide/performance#1-reuse-the-session).
