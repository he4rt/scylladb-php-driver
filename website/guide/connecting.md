# Clusters and sessions

Connecting has three steps. Each one produces a different object with a different lifetime.

```
Cassandra::cluster()
        ▼
Cassandra\Cluster\Builder    mutable, cheap, one per configuration
        │ build()
        ▼
Cassandra\Cluster            immutable, holds no sockets
        │ connect($keyspace)
        ▼
Cassandra\Session            owns the connection pool
```

## The builder

`Cassandra::cluster()` returns a fresh `Cassandra\Cluster\Builder`. Every `with*()` method mutates
the builder and returns it, so calls chain in any order.

```php
$builder = Cassandra::cluster()
    ->withContactPoints('10.0.0.1', '10.0.0.2', '10.0.0.3')
    ->withPort(9042)
    ->withCredentials('app_user', getenv('SCYLLA_PASSWORD'))
    ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
    ->withTokenAwareRouting(true);

$cluster = $builder->build();
```

`build()` copies the configuration into an immutable `Cassandra\Cluster`. Later changes to the
builder do not affect a cluster that is already built, so one builder can produce several clusters.

## Contact points

Contact points are the addresses the driver dials first. After the first connection succeeds, the
driver reads the full node list from the cluster and connects to the rest.

```php
->withContactPoints('10.0.0.1', '10.0.0.2', '10.0.0.3')
```

The method is variadic. Spread an array when the list comes from configuration:

```php
$hosts = explode(',', getenv('SCYLLA_HOSTS') ?: '127.0.0.1');

$builder->withContactPoints(...$hosts);
```

Guidance:

- **Give two or three contact points, not one.** If the single contact point is down, the driver
  cannot bootstrap, even when the rest of the cluster is healthy.
- **Do not list every node.** The driver discovers them. A long list only slows down startup.
- **Host names work.** Turn on `withHostnameResolution(true)` when you want the driver to resolve
  peer IP addresses back to host names, which matters for TLS host name verification.
- **Randomize the order** with `withRandomizedContactPoints(true)` so that many application
  processes do not all hit the same node first. This is on by default.

## Port

```php
->withPort(9042)
```

9042 is the CQL native port. Change it only when the cluster listens elsewhere. The port applies to
every node, so a cluster with mixed ports is not supported.

## Building and connecting

```php
$cluster = $builder->build();

$session  = $cluster->connect();          // no keyspace
$session  = $cluster->connect('shop');    // sets the keyspace
$session  = $cluster->connect('shop', 10); // 10 second connect timeout
```

`connect()` blocks until the pool is ready. It throws on failure. The most common failures are
`Cassandra\Exception\RuntimeException` for an unreachable host, and
`Cassandra\Exception\AuthenticationException` for bad credentials.

::: warning Empty keyspace strings
Pass `null`, or omit the argument, when you have no keyspace. An empty string produces an invalid
`USE` statement on newer drivers.

```php
$keyspace = getenv('SCYLLA_KEYSPACE') ?: null;
$session  = $cluster->connect($keyspace);
```
:::

### Connecting asynchronously

```php
$future  = $cluster->connectAsync('shop');
// ... other startup work ...
$session = $future->get(5.0);   // blocks up to 5 seconds
```

`connectAsync()` returns a `Cassandra\FutureSession`. Use it to overlap the connection handshake
with other startup work. See [asynchronous queries](/guide/async).

## The session

A `Cassandra\Session` is the object you keep. It owns:

- the connection pool to every node it knows about,
- the prepared statement cache,
- the schema metadata,
- the load balancing and retry policy state.

```php
$session->execute($statement, $options);      // returns Rows
$session->executeAsync($statement, $options); // returns FutureRows
$session->prepare($cql);                      // returns PreparedStatement
$session->prepareAsync($cql);                 // returns FuturePreparedStatement
$session->schema();                           // returns Schema
$session->metrics();                          // returns array
$session->close();                            // blocks until closed
$session->closeAsync();                       // returns FutureClose
```

### Session lifetime

Opening a session is expensive. It performs a TCP handshake per connection, an optional TLS
handshake, authentication, and a schema fetch. Do not do that per HTTP request.

| Runtime | Recommendation |
| --- | --- |
| CLI script, worker, daemon | One session for the whole process. Reuse it. |
| FrankenPHP, RoadRunner, Swoole | One session per worker, created at boot. |
| PHP-FPM | Turn on persistent sessions. See below. |

### Persistent sessions

```php
->withPersistentSessions(true)
```

With persistent sessions on, the driver caches the session in process memory and keys it by the
connection configuration. A later `connect()` in the same PHP worker process returns the cached
session instead of opening a new pool. This is what makes the driver usable under PHP-FPM, where
each request starts a fresh PHP request but reuses the worker process.

Two consequences:

- Sessions live until the worker process exits. A configuration change needs a worker restart.
- `close()` on a persistent session is a no-op for the pool. It stays available to the next request.

Turn it off for tests and short lived scripts so each run starts clean.

## Full connection example

```php
<?php

declare(strict_types=1);

function makeSession(): Cassandra\Session
{
    static $session = null;

    if ($session !== null) {
        return $session;
    }

    $ssl = Cassandra::ssl()
        ->withTrustedCerts('/etc/ssl/certs/scylla-ca.pem')
        ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY)
        ->build();

    $cluster = Cassandra::cluster()
        ->withContactPoints(...explode(',', getenv('SCYLLA_HOSTS')))
        ->withPort((int) (getenv('SCYLLA_PORT') ?: 9042))
        ->withCredentials(getenv('SCYLLA_USER'), getenv('SCYLLA_PASSWORD'))
        ->withSSL($ssl)
        ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
        ->withDatacenterAwareRoundRobinLoadBalancingPolicy('eu-west-1', 0, false)
        ->withTokenAwareRouting(true)
        ->withConnectTimeout(5.0)
        ->withRequestTimeout(12.0)
        ->withConnectionsPerHost(2, 8)
        // Heartbeat left at its 30 second default: see the defect note in the guide.
        ->withTCPNodelay(true)
        ->withPersistentSessions(true)
        ->build();

    return $session = $cluster->connect(getenv('SCYLLA_KEYSPACE') ?: null);
}
```

## Where to go next

| Topic | Page |
| --- | --- |
| Username, password, and secrets | [Authentication](/guide/authentication) |
| Certificates and verification | [TLS and SSL](/guide/tls) |
| Which node gets the request | [Load balancing and routing](/guide/load-balancing) |
| Pool size, timeouts, heartbeats | [Connection pool and timeouts](/guide/connection-tuning) |
| What happens after a failure | [Retry policies](/guide/retry-policies) |
| Every method, in one table | [`Cluster\Builder` reference](/reference/cluster-builder) |
