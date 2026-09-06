# Connection pool and timeouts

These settings control how many sockets the driver opens, how long it waits, and how it detects a
dead connection. The defaults work for a small cluster and moderate traffic. Change them when you
measure a reason to.

## Timeouts

All timeout values are given in **seconds**, as a float. `0.25` is 250 milliseconds.

| Setting | Default | What it bounds |
| --- | --- | --- |
| `withConnectTimeout(float $seconds)` | 5.0 | The TCP and TLS handshake for one connection. |
| `withRequestTimeout(float $seconds)` | 12.0 | One request, measured inside the C driver. |
| `withDefaultTimeout(float $seconds)` | none | The default timeout applied by `execute()` when the call passes none. |

```php
->withConnectTimeout(5.0)
->withRequestTimeout(12.0)
->withDefaultTimeout(10.0)
```

Three timeouts exist because they answer different questions:

- `withConnectTimeout()` bounds pool setup. Raise it on a high-latency link, lower it when you want
  a dead node marked down quickly.
- `withRequestTimeout()` is the hard ceiling the C driver enforces. Nothing can exceed it.
- `withDefaultTimeout()` and the per-query `timeout` option bound the wait on the PHP side.

A per-query timeout overrides the default:

```php
$rows = $session->execute($statement, ['timeout' => 2.5]);
```

::: tip Set the request timeout below the client timeout
Give the application a shorter deadline than the server-side write timeout of your cluster.
Otherwise the client gives up while the coordinator is still working, and a retry can duplicate a
non-idempotent write.
:::

## Connections per host

```php
->withConnectionsPerHost(core: 1, max: 2)
```

| Parameter | Default | Range |
| --- | --- | --- |
| `core` | 1 | 1 to 128 |
| `max` | 2 | 1 to 128 |

`core` connections stay open to every node. The pool grows toward `max` under load and shrinks back.

Each connection multiplexes many in-flight requests, so a single connection carries a lot of
traffic. Raise the core count when:

- Latency rises while node CPU stays low, which points at request queueing on the client.
- Connection setup cost shows in the latency tail, which TLS makes worse.

Remember the multiplier. `core: 4` against a 12-node cluster with 20 PHP-FPM workers is 960 open
sockets from one host.

## IO threads

```php
->withIOThreads(1)   // default 1, range 1 to 128
```

Each IO thread runs an event loop and owns a share of the connections. One thread is right for
PHP-FPM and CLI scripts, because a PHP process handles one request at a time.

Raise it only when one PHP process drives many concurrent queries, for example a worker that fans
out with [`executeAsync()`](/guide/async), or a Swoole or RoadRunner runtime. Two to four threads
covers almost every such case.

## Heartbeats and reconnection

```php
->withConnectionHeartbeatInterval(30.0)   // seconds, default 30
->withReconnectInterval(2.0)              // seconds, default 2
```

::: tip Fixed since 1.4.x
Up to 1.4.x, `withConnectionHeartbeatInterval()` and `withTCPKeepalive()` passed a value 1000 times
too large to the C driver, so `withConnectionHeartbeatInterval(30.0)` asked for 30000 seconds and
heartbeats effectively stopped. Both now take seconds, as documented. If you divided by 1000 to work
around the old behaviour, remove that workaround. See [upgrading from 1.4.x](/guide/upgrading).
:::

The heartbeat sends a lightweight request on an idle connection. It has two jobs: it detects a
connection that a firewall or a load balancer silently dropped, and it stops idle timeouts from
closing the connection.

Lower the interval when a stateful firewall sits between the application and the cluster and cuts
idle flows. A value below the firewall idle timeout keeps the connection alive.

The reconnect interval is the constant delay between attempts to restore a connection to a node that
is marked down.

## TCP options

```php
->withTCPNodelay(true)      // default true
->withTCPKeepalive(60.0)    // seconds; pass null to disable (default)
```

`TCP_NODELAY` disables Nagle's algorithm. Leave it on. CQL frames are small and latency-sensitive,
and Nagle adds delay for no gain.

TCP keepalive is the kernel-level probe. It is off by default because the driver heartbeat covers
the same failure at the protocol level. Turn it on when a network device drops idle flows below the
protocol layer.

`withTCPKeepalive()` takes seconds, and `null` disables it. See the note under
[heartbeats and reconnection](#heartbeats-and-reconnection) and
[upgrading from 1.4.x](/guide/upgrading).

## Protocol version

```php
->withProtocolVersion(Cassandra\ProtocolVersion::V4)   // default V4
```

`Cassandra\ProtocolVersion` names the five native protocol versions, `V1` to `V5`. The method also
takes a plain integer, for a version the enum does not name. Reading `protocolVersion` back off the
builder gives an enum case when one matches the value, and an integer when none does.

Version 4 works with every supported server. Lower it only for an old cluster that rejects the
handshake. The driver does not negotiate downward on its own.

## Page size

```php
->withDefaultPageSize(5000)   // default 5000
```

This is the number of rows the server returns per page. Change it per query with the `page_size`
option. See [results and paging](/guide/results).

## Schema metadata

```php
->withSchemaMetadata(true)   // default true
```

The driver fetches the schema at connect time and tracks changes. Metadata drives two things you
probably want: [token-aware routing](/guide/load-balancing#token-aware-routing) and
[`Session::schema()`](/guide/schema-metadata).

Turning it off saves a small amount of memory and one fetch at startup. It costs you token
awareness. That trade is rarely worth it.

## A tuned configuration

```php
$cluster = Cassandra::cluster()
    ->withContactPoints('10.1.0.11', '10.1.0.12', '10.1.0.13')

    // Fail fast on a dead node, allow a slow query to finish.
    ->withConnectTimeout(3.0)
    ->withRequestTimeout(10.0)
    ->withDefaultTimeout(8.0)

    // A pool that absorbs a burst without opening sockets mid-peak.
    ->withConnectionsPerHost(2, 8)
    ->withIOThreads(1)

    // Survive an idle-flow-killing firewall.
    ->withConnectionHeartbeatInterval(20.0)   // 20 seconds
    ->withReconnectInterval(2.0)
    ->withTCPNodelay(true)

    ->build();
```

## Checking the result

`Session::metrics()` reports the live pool state. Watch `total_connections` and
`available_connections` after a change, and watch the error counters for timeouts. See
[metrics and logging](/guide/observability).

```php
$m = $session->metrics();

printf(
    "connections %d/%d, request timeouts %d\n",
    $m['stats']['available_connections'],
    $m['stats']['total_connections'],
    $m['errors']['request_timeouts'],
);
```
