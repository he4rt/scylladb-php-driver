# Metrics and logging

## Metrics

`Session::metrics()` returns the C driver's live counters as a nested array. It reads process
memory, so it costs nothing and is safe to call on a hot path.

```php
$metrics = $session->metrics();
```

The array has three keys.

### `requests`

Latency in **microseconds**, plus throughput rates.

| Key | Meaning |
| --- | --- |
| `min`, `max`, `mean`, `median`, `stddev` | Latency summary |
| `p75`, `p95`, `p98`, `p99`, `p999` | Latency percentiles |
| `mean_rate` | Requests per second since start |
| `m1_rate`, `m5_rate`, `m15_rate` | One, five, and fifteen minute rates |

```php
$r = $metrics['requests'];

printf("p50=%.1fms p99=%.1fms %.0f req/s\n",
    $r['median'] / 1000,
    $r['p99'] / 1000,
    $r['m1_rate'],
);
```

### `stats`

Connection pool state.

| Key | Meaning |
| --- | --- |
| `total_connections` | Connections the pool holds |
| `available_connections` | Connections ready to take a request |
| `exceeded_pending_requests_water_mark` | Times the pending queue crossed its limit |
| `exceeded_write_bytes_water_mark` | Times the write buffer crossed its limit |

A gap between `total_connections` and `available_connections` means connections are busy or
reconnecting. A rising `exceeded_pending_requests_water_mark` means the client is queueing, which
calls for more connections per host or less concurrency.

### `errors`

| Key | Meaning |
| --- | --- |
| `connection_timeouts` | Connections that failed to establish in time |
| `pending_request_timeouts` | Requests that timed out while queued on the client |
| `request_timeouts` | Requests that timed out after they were sent |

The split matters. `pending_request_timeouts` points at a client-side bottleneck.
`request_timeouts` points at the cluster.

### Exporting

Counters are cumulative per session. Export them at the end of a request, or on a timer in a long
running process.

```php
function exportMetrics(Cassandra\Session $session, StatsdClient $statsd): void
{
    $m = $session->metrics();

    $statsd->gauge('cassandra.latency.p50', $m['requests']['median'] / 1000);
    $statsd->gauge('cassandra.latency.p99', $m['requests']['p99'] / 1000);
    $statsd->gauge('cassandra.rate.1m', $m['requests']['m1_rate']);

    $statsd->gauge('cassandra.connections.total', $m['stats']['total_connections']);
    $statsd->gauge('cassandra.connections.available', $m['stats']['available_connections']);

    $statsd->gauge('cassandra.errors.request_timeouts', $m['errors']['request_timeouts']);
    $statsd->gauge('cassandra.errors.connection_timeouts', $m['errors']['connection_timeouts']);
}
```

### What to alert on

| Signal | Meaning |
| --- | --- |
| p99 rises while the median stays flat | One slow node, or client-side queueing |
| `available_connections` below `total_connections` for a long time | Nodes flapping |
| `pending_request_timeouts` above zero | The client cannot keep up. Raise the pool or lower concurrency |
| `request_timeouts` rising | The cluster is slow or overloaded |
| `m1_rate` drops to zero | The application stopped querying. Check the application, not the cluster |

## Logging

The C driver writes its own log. Two `php.ini` settings control it.

```ini
[cassandra]
cassandra.log_level = ERROR
cassandra.log = /var/log/php-cassandra.log
```

| Setting | Default | Values |
| --- | --- | --- |
| `cassandra.log_level` | `ERROR` | `CRITICAL`, `ERROR`, `WARN`, `INFO`, `DEBUG`, `TRACE` |
| `cassandra.log` | `cassandra.log` | An absolute path, `syslog`, `stderr`, or an empty value for stderr |
| `cassandra.expose_credentials` | `Off` | `On` shows the real password in the builder properties |

With `syslog` the driver writes through `syslog(3)` under the `cassandra` ident, with `LOG_USER` and
the process id. The severity maps to `LOG_CRIT`, `LOG_ERR`, `LOG_WARNING`, `LOG_INFO` and `LOG_DEBUG`.
See [php.ini configuration](/guide/configuration) for every other setting.

::: warning Every setting is PHP_INI_SYSTEM
They can be changed in `php.ini` or with `php -d` only. `ini_set()` at runtime has no effect. For
the log settings, the callback runs on driver IO threads that have no PHP context. For
`cassandra.expose_credentials`, the restriction is deliberate: a library or a debug handler must not
be able to unredact a credential.
:::

### `cassandra.expose_credentials`

`Cassandra\Cluster\Builder` exposes its configuration as object properties, which `var_dump()`,
`print_r()`, and framework debug panels read. The password is shown as `***` by default. Turn this
setting on to see the real value while you debug a connection problem.

```ini
cassandra.expose_credentials = On
```

Leave it off everywhere except a development machine. See the
[`Cluster\Builder` reference](/reference/cluster-builder#reading-the-configuration-back).

::: tip Set an absolute path
The default is the relative name `cassandra.log`, which resolves against the current working
directory of the process. Under PHP-FPM that is rarely where you expect. Always set an absolute path
that the PHP user can write, or leave the value empty so the log goes to stderr and your process
supervisor collects it.
:::

### Log format

To a file:

```
05-08-2026 14:23:11 CEST [ERROR] Connection error 'Connection timeout' (src/connector.cpp:120)
```

To stderr:

```
cassandra | [ERROR] Connection error 'Connection timeout' (src/connector.cpp:120)
```

### Choosing a level

| Level | Use |
| --- | --- |
| `CRITICAL` | Almost nothing is logged |
| `ERROR` | The production default |
| `WARN` | Adds node up and down events, and retry notices |
| `INFO` | Adds connection and schema events. Useful during a rollout |
| `DEBUG` | Per-request detail. Development only |
| `TRACE` | Very high volume. Short debugging sessions only |

`DEBUG` and `TRACE` produce a lot of output on a busy process. Do not leave them on.

### Logging retry decisions

The driver log does not record retries by default. Wrap the retry policy to get them:

```php
$cluster = Cassandra::cluster()
    ->withRetryPolicy(
        new Cassandra\RetryPolicy\Logging(new Cassandra\RetryPolicy\DefaultPolicy())
    )
    ->build();
```

The decisions go to the same driver log. See [retry policies](/guide/retry-policies).

## Correlating with application logs

The driver log has no request identifier, so it cannot be joined to your application log
automatically. Log the driver exception on the application side, where the request context exists:

```php
try {
    $rows = $session->execute($statement, $options);
} catch (Cassandra\Exception $e) {
    $this->logger->error('cassandra query failed', [
        'request_id' => $this->requestId,
        'exception'  => $e::class,
        'code'       => $e->getCode(),
        'message'    => $e->getMessage(),
    ]);
    throw $e;
}
```

Use the driver log for cluster-level events, and the application log for per-request failures.

## Server-side view

The client view is one half. Compare it with the cluster:

```bash
nodetool tpstats          # dropped messages and queued tasks
nodetool proxyhistograms  # coordinator latency
nodetool tablehistograms shop users
```

Client p99 far above the coordinator p99 points at the network or at the client. The two matching
points at the cluster.
