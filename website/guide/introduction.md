# Introduction

The ScyllaDB PHP driver is a PHP extension written in C. It connects PHP applications to
[ScyllaDB](https://www.scylladb.com) and [Apache Cassandra](https://cassandra.apache.org) over the
native CQL binary protocol.

The extension name is `cassandra`. All classes live in the `Cassandra\` namespace, plus one global
`Cassandra` class that acts as the entry point and constant holder.

## How it works

The extension does not implement the CQL protocol itself. It wraps the
[ScyllaDB C/C++ driver](https://github.com/scylladb/cpp-driver), which owns the sockets, the
connection pools, and the IO threads.

```
┌───────────────────────┐
│  Your PHP application │
└──────────┬────────────┘
           │  Cassandra\Session::execute()
┌──────────▼────────────┐
│  cassandra.so  (C23)  │  zvals ⇄ CQL values, object handlers, exceptions
└──────────┬────────────┘
           │  cass_session_execute()
┌──────────▼────────────┐
│  ScyllaDB C/C++ driver│  IO threads, connection pools, routing, retries
└──────────┬────────────┘
           │  CQL binary protocol (TCP, optional TLS)
┌──────────▼────────────┐
│  ScyllaDB / Cassandra │
└───────────────────────┘
```

Two consequences matter for application code:

1. **Row decoding happens in C.** A `SELECT` returns a `Cassandra\Rows` object that decodes each
   column into the correct PHP value. You never parse a wire format.
2. **The connection pool lives below PHP.** One `Cassandra\Session` holds many TCP connections to
   many nodes. Reuse the session. Do not build one per request when you can avoid it. See
   [performance](/guide/performance).

## What the driver gives you

| Capability | Entry point |
| --- | --- |
| Cluster configuration | [`Cassandra::cluster()`](/reference/cluster-builder) |
| Query execution | [`Session::execute()`](/reference/session) |
| Prepared statements | [`Session::prepare()`](/guide/queries#prepared-statements) |
| Batches | [`Cassandra\BatchStatement`](/guide/batches) |
| Paging | [`Rows::nextPage()`](/guide/results) |
| Concurrency | [`Session::executeAsync()`](/guide/async) |
| Schema introspection | [`Session::schema()`](/guide/schema-metadata) |
| TLS | [`Cassandra::ssl()`](/guide/tls) |
| Metrics | [`Session::metrics()`](/guide/observability) |

## Compatibility

| Component | Supported versions |
| --- | --- |
| PHP | 8.2, 8.3, 8.4, 8.5 |
| ScyllaDB | 4.4.x, 5.x, 6.x |
| Apache Cassandra | 3.0 and later, through DataStax libcassandra |
| Architecture | x86-64, 64-bit only |
| Thread safety | NTS and ZTS |
| Compilers | GCC 13 or later, Clang 16 or later |
| Operating system | Linux, macOS |

## Relation to the old DataStax driver

The API follows the DataStax PHP driver that this project forked from. Most code that ran on the old
extension runs here. Three differences catch people out:

- Value classes are flat. Use `Cassandra\Bigint`, not `Cassandra\Numbers\Bigint`. The sub-namespaces
  in the source tree organize files, not classes.
- Signatures are strictly typed. The stubs declare union types such as `string|Statement`. A wrong
  argument type raises a `TypeError` instead of a silent cast.
- `Cassandra\ExecutionOptions` as a constructor is deprecated. Pass a plain array instead. See
  [queries and statements](/guide/queries#execution-options).

## Project status

The extension is being ported from C++ to C23. The port is incremental and does not change the PHP
API. `src/Cluster/` is the reference module for the new style.

Read [CONTRIBUTING.md](https://github.com/he4rt/scylladb-php-driver/blob/trunk/CONTRIBUTING.md)
before you send a patch.
