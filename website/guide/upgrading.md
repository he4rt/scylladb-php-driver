# Upgrading from 1.4.x

Version 1.5 changes two defaults, tightens value validation, and corrects a unit bug in two
builder methods. Read this page before you run 1.5 with an application written against 1.4.

Start from 1.5.1. The `v1.5.0` tag has no release.

## Install libuv on the host

The extension links `libuv.so.1` and no longer carries a private copy of it. Install the libuv
runtime package on every host that loads `cassandra.so`.

```bash
apt-get install -y libuv1        # Debian, Ubuntu
dnf install -y libuv             # RHEL, Fedora, Amazon Linux
```

A host without it fails at `dlopen` with an unresolved `uv_*` symbol.

## The default consistency is LOCAL_QUORUM

A statement that sets no consistency now reads and writes a majority of the replicas in its
local datacenter. Up to 1.4.x the default was `LOCAL_ONE`, which could serve a read from a
replica that had not seen the last write.

**What can break.** A query at `LOCAL_QUORUM` fails when too few replicas in the local
datacenter are up. With replication factor 3 the cluster tolerates one node down, not two.
Latency also rises, because the coordinator waits for a majority.

**How to keep the old behaviour.** Set one line in `php.ini`:

```ini
cassandra.default_consistency = LOCAL_ONE
```

You can also set it per cluster with `withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_ONE)`,
or per statement with the `consistency` execution option. Prefer `LOCAL_QUORUM` and drop to
`LOCAL_ONE` only on the queries that value latency over freshness.

::: warning
Do not switch to plain `QUORUM` for a stronger guarantee. `QUORUM` counts replicas across every
datacenter. It adds cross-datacenter latency to every query and fails when a remote datacenter
is unreachable.
:::

## The persistent caches are bounded

Each worker now caches at most 16 clusters, 16 sessions and 1000 prepared statements. All three
were unlimited. Past a cap the resource is still created, it is just not cached, and the driver
writes one `E_WARNING` per request.

Raise a cap, or set it to `-1`, when the warning appears and the growth is intended:

```ini
cassandra.max_persistent_clusters = -1
cassandra.max_persistent_sessions = -1
cassandra.max_persistent_prepared_statements = -1
```

An application that builds CQL by string concatenation is the usual cause. Prepare a statement
with bound parameters instead, so one cache entry serves every call.

## Heartbeat and TCP keepalive take seconds

Up to 1.4.x, `withConnectionHeartbeatInterval()` and `withTCPKeepalive()` passed a value 1000
times too large to the C driver. `withConnectionHeartbeatInterval(30.0)` asked for a
30000-second heartbeat, so heartbeats stopped.

Both now take seconds, as documented. Remove any division by 1000 that worked around the old
behaviour. See [connection pool and timeouts](/guide/connection-tuning) for the current values.

## Value constructors reject what they used to accept

The numeric and temporal constructors validate their input. Code that passed a malformed string
and got a value now gets an exception.

| Input | Up to 1.4.x | 1.5 |
| --- | --- | --- |
| `new Cassandra\Smallint('- 3')` | `-3` | throws |
| `new Cassandra\Date('12abc')` | `12` | throws |
| `new Cassandra\Time(-1)` | the current time of day | throws |
| `new Cassandra\Time('86400000000000')` | accepted | throws |
| a value with a NUL byte | the part before the NUL | throws |

Validate the input before you construct the value, or catch
`Cassandra\Exception\InvalidArgumentException` around the constructor.

## Futures are no longer constructible

`new Cassandra\FutureRows()` and every other `Cassandra\Future*` class throw. Take a future from
`executeAsync()`, `connectAsync()` or `closeAsync()`.

## A Builder dump redacts the password

`var_dump()`, `print_r()`, `get_object_vars()` and an `(array)` cast over a `Cluster\Builder`
report `***` for the password. This landed in 1.4.2.

Set `cassandra.expose_credentials = 1` in `php.ini` to put the real value back. The directive is
`PHP_INI_SYSTEM`, so only the operator can turn the redaction off.

## Two array casts changed shape

`(array)` over a `Cluster\Builder` gives a `Cassandra\ProtocolVersion` case for the
`protocolVersion` key. It was an integer. A version that no case names stays an integer.

`(array)` over a `Cassandra\Type\UserType` gives a `keyspace` key and a `name` key beside
`types`. Both are `null` for a user type that names neither.

## syslog logging reaches syslog

`cassandra.log = syslog` writes through `syslog(3)`. It used to create a file named `syslog` in
the working directory.

## Next

Read the [changelog](https://github.com/he4rt/scylladb-php-driver/blob/trunk/CHANGELOG.md) for
the full list, including the additions this page does not cover: execution profiles, event loop
integration, `php.ini` seeds for every builder default, statement idempotence and
`Rows::wasApplied()`.
