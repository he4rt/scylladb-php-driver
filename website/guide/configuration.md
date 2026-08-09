# php.ini configuration

The driver reads its configuration from `php.ini`, or from a file in the `conf.d` directory. Every
setting is `PHP_INI_SYSTEM`. `ini_set()` cannot change any of them at runtime.

::: warning Why PHP_INI_SYSTEM and not PHP_INI_ALL
The `Cluster\Builder` seeds below become part of the identity of a cached `CassCluster`. A
request-scoped `ini_set()` would give each request a different identity, and the persistent cache
would grow with no limit inside the worker. The read-only scope is what keeps the cache bounded.
:::

## Logging

| Setting | Default | Values |
| --- | --- | --- |
| `cassandra.log_level` | `ERROR` | `CRITICAL`, `ERROR`, `WARN`, `INFO`, `DEBUG`, `TRACE` |
| `cassandra.log` | `cassandra.log` | A file path, `syslog`, `stderr`, or an empty value for stderr |

See [Metrics and logging](/guide/observability) for the log format.

## Credentials

| Setting | Default | Values |
| --- | --- | --- |
| `cassandra.expose_credentials` | `0` | `1` prints the password in `Cluster\Builder` debug output |

Keep this off in production. See [Authentication](/guide/authentication).

## Persistent resources

The driver caches clusters, sessions and prepared statements per PHP-FPM worker. Nothing bounds
those caches by default. An application that builds CQL with string concatenation gets one cache
slot per distinct query text, so the cache grows until the worker restarts.

| Setting | Default | Meaning |
| --- | --- | --- |
| `cassandra.allow_persistent` | `1` | Master switch for all three caches |
| `cassandra.max_persistent_clusters` | `-1` | Cached clusters per worker |
| `cassandra.max_persistent_sessions` | `-1` | Cached sessions per worker |
| `cassandra.max_persistent_prepared_statements` | `-1` | Cached prepared statements per worker |

`-1` means unlimited. `0` disables the cache for that resource.

Past a cap the driver still creates the resource, but does not keep it. The application keeps
working, at the cost of one rebuild per request. The driver writes one `E_WARNING` per request when
this first happens, so a saturated cache is visible in the error log.

```ini
[cassandra]
; Bound the prepared-statement cache to 512 entries per worker.
cassandra.max_persistent_prepared_statements = 512
```

`cassandra.allow_persistent = 0` is a floor, not a default. With it off,
`withPersistentSessions(true)` cannot turn caching back on. Use it to shut off persistence for a
worker pool without a change to the application.

## Cluster\Builder defaults

Each setting below is the value a new `Cassandra\Cluster\Builder` starts with. The matching
`with*()` method still overrides it, so these are deployment defaults, not a lock.

### Contact points

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.contact_points` | `127.0.0.1` | Comma-separated hosts |
| `cassandra.port` | `9042` | 1 to 65535 |
| `cassandra.randomized_contact_points` | `1` | Boolean |
| `cassandra.hostname_resolution` | `0` | Boolean |

### Timeouts

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.connect_timeout` | `5000` | Milliseconds |
| `cassandra.request_timeout` | `12000` | Milliseconds |
| `cassandra.reconnect_interval` | `2000` | Milliseconds |

### Queries

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.default_consistency` | `LOCAL_QUORUM` | See the list below |
| `cassandra.default_page_size` | `5000` | Rows |
| `cassandra.protocol_version` | `4` | 1 to 5 |

Consistency accepts `ANY`, `ONE`, `TWO`, `THREE`, `QUORUM`, `ALL`, `LOCAL_QUORUM`, `EACH_QUORUM`,
`SERIAL`, `LOCAL_SERIAL` and `LOCAL_ONE`. The name is not case-sensitive.

::: warning Do not set this to QUORUM
`QUORUM` counts replicas in every datacenter. It adds cross-datacenter latency to every query and
fails when a remote datacenter is unreachable. `LOCAL_QUORUM`, the default, gives the same strong
read-your-writes guarantee at local latency.
:::

::: tip Keep consistency in the application
`cassandra.default_consistency` is a correctness decision, not a deployment knob. Prefer
`withDefaultConsistency()` in code, where a reader can see it next to the query.
:::

### Connection pool

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.io_threads` | `1` | Threads |
| `cassandra.core_connections_per_host` | `1` | Connections |
| `cassandra.max_connections_per_host` | `2` | Connections |
| `cassandra.connection_heartbeat_interval` | `30` | Seconds, `0` disables |
| `cassandra.tcp_keepalive_delay` | `0` | Seconds, `0` disables |
| `cassandra.tcp_nodelay` | `1` | Boolean |

See [Connection pool and timeouts](/guide/connection-tuning).

### ScyllaDB rack-aware routing

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.local_dc` | empty | Datacenter name |
| `cassandra.local_rack` | empty | Rack name |

A non-empty `cassandra.local_rack` turns on rack-aware load balancing. The driver tries live nodes in
the local rack first, then the rest of the local datacenter, then remote datacenters. Use it to keep
traffic inside one cloud availability zone and cut the cross-zone bill.

```ini
cassandra.local_dc   = eu-west-1
cassandra.local_rack = eu-west-1a
```

Leave both empty to keep the current policy. If you set only `cassandra.local_rack`, the driver
infers the datacenter from the first contact point it reaches, so list contact points from the local
rack only.

This setting needs the ScyllaDB C/C++ driver. The upstream DataStax driver does not have it.

### Application identity

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.application_name` | empty | Free text |
| `cassandra.application_version` | empty | Free text |

The server records both. Query them to see which application and which version opens the
connections:

```sql
SELECT address, client_options FROM system.clients;
```

`client_options` then contains `APPLICATION_NAME` and `APPLICATION_VERSION`.

### Reconnection

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.reconnect_policy` | `constant` | `constant` or `exponential` |
| `cassandra.reconnect_max_interval` | `60000` | Milliseconds |

With `constant`, the driver waits `cassandra.reconnect_interval` between every attempt. With
`exponential`, that value becomes the base delay and the wait grows to
`cassandra.reconnect_max_interval`, plus or minus 15 percent of jitter.

Prefer `exponential` for a large worker pool. A constant delay makes every worker retry a recovering
node at the same moment.

### Speculative execution

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.speculative_execution_delay` | `0` | Milliseconds, `0` disables |
| `cassandra.speculative_execution_max` | `2` | Extra attempts |

The driver sends the same request to another replica when the first one does not answer within the
delay. It cuts tail latency caused by one slow replica.

::: warning Only for idempotent statements
A speculative attempt runs the statement more than once. Never turn this on for a counter update, a
lightweight transaction, or an append to a list. It also multiplies load, so keep it off while the
cluster is already saturated.
:::

### Event loop tuning

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.coalesce_delay` | `200` | Microseconds |
| `cassandra.new_request_ratio` | `50` | 1 to 100 |
| `cassandra.queue_size_io` | `8192` | Queued requests per IO thread |

`coalesce_delay` is how long the driver waits to batch writes into one system call.
`new_request_ratio` splits IO thread time between accepting new requests and finishing outstanding
ones.

::: warning Change these against a measurement
The defaults come from the C driver and suit most deployments. PHP-FPM handles one request per
process, so there is little to batch. Do not change either without a benchmark of your own workload.
:::

### Connection lifetime

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.connection_idle_timeout` | `60` | Seconds an unused connection stays open |
| `cassandra.max_schema_wait_time` | `10000` | Milliseconds to wait for schema agreement |
| `cassandra.resolve_timeout` | `2000` | Milliseconds for a hostname lookup |
| `cassandra.monitor_reporting_interval` | `300` | Seconds, `0` disables |
| `cassandra.local_address` | empty | Source address for outgoing connections |

Set `cassandra.local_address` on a host with several interfaces, to pick the one that reaches the
cluster. An empty value lets the kernel choose.

### Prepared statements

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.prepare_on_all_hosts` | `1` | Boolean |
| `cassandra.prepare_on_up_or_add_host` | `1` | Boolean |

The driver prepares a statement on every node, not only on the coordinator, and prepares again when
a node comes up or joins. Leave both on. With them off, a query that lands on a node which never saw
the prepare pays an extra round trip.

### Protocol and routing

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.shuffle_replicas` | `1` | Boolean |
| `cassandra.no_compact` | `0` | Boolean |
| `cassandra.beta_protocol` | `0` | Boolean |

`shuffle_replicas` spreads reads of one partition over its replicas instead of always choosing the
first. Turn it off only to make a benchmark repeatable.

`no_compact` asks the server to show `COMPACT STORAGE` tables in their non-compact form. It matters
only for a cluster that still has such tables.

::: warning beta_protocol is for testing
A beta native protocol can change between server releases. Never set this in production.
:::

### Request tracing

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.tracing_consistency` | `ONE` | Consistency name |
| `cassandra.tracing_max_wait_time` | `15` | Milliseconds |
| `cassandra.tracing_retry_wait_time` | `3` | Milliseconds |

These apply only to statements that turn tracing on. The server writes the trace after it answers
the query, so the driver polls for it. A wait that is too short returns an incomplete trace.

### Routing and metadata

| Setting | Default | Unit |
| --- | --- | --- |
| `cassandra.token_aware_routing` | `1` | Boolean |
| `cassandra.latency_aware_routing` | `1` | Boolean |
| `cassandra.schema_metadata` | `1` | Boolean |

See [Load balancing and routing](/guide/load-balancing).

## Bad values

The driver ignores a bad value and keeps the documented default. It does not clamp the value, and it
does not refuse to start. Each rejected directive writes one `E_WARNING` at startup that names the
value it ignored.

`ini_get()` and `phpinfo()` report the default in that case, so what they show is always what the
driver uses.

```ini
cassandra.port = 99999
```

```
Warning: cassandra | cassandra.port must be between 1 and 65535, ignoring '99999' and using the default
```

`ini_get('cassandra.port')` then returns `9042`, and the driver connects on port 9042.

## Example

```ini
[cassandra]
extension=cassandra.so

cassandra.log = syslog
cassandra.log_level = WARN

cassandra.contact_points = db-1.internal,db-2.internal,db-3.internal
cassandra.connect_timeout = 3000
cassandra.request_timeout = 8000

cassandra.io_threads = 4
cassandra.core_connections_per_host = 2
cassandra.max_connections_per_host = 8

cassandra.max_persistent_prepared_statements = 512
```
