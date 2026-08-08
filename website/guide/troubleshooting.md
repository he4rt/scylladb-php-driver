# Troubleshooting

## Installation

### `Class "Cassandra" not found`

The extension is not loaded.

```bash
php -m | grep cassandra              # is it listed?
php -i | grep '^extension_dir'       # where does PHP look?
php --ini                            # which ini files are read?
```

The CLI and the web server often read different ini files. Check both.

### `cannot open shared object file: libscylla-cpp-driver.so`

The extension loaded, but the dynamic linker cannot find the C/C++ driver.

```bash
ldd "$(php -i | grep '^extension_dir' | cut -d' ' -f3)/cassandra.so"
```

Fix it by adding the library path:

```bash
echo "$HOME/.local/lib" | sudo tee /etc/ld.so.conf.d/scylla.conf
sudo ldconfig
```

On macOS, set `DYLD_LIBRARY_PATH`, or install the driver to a standard prefix.

### `symbol not found in flat namespace '__emalloc_512'`

The extension was built against a different PHP than the one loading it. The Zend memory manager ABI
differs between a debug build and a release build, and between NTS and ZTS.

Rebuild against the exact PHP binary you run:

```bash
php-config --version
php-config --php-binary
```

### The build cannot find the C/C++ driver

```bash
export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH"
pkg-config --modversion scylla-cpp-driver
```

Set `PKG_CONFIG_PATH` in your shell profile so every build sees it.

## Connecting

### `connect()` hangs, then throws

The contact point resolves but nothing listens, or a firewall drops the packets silently.

```bash
nc -vz 10.0.0.1 9042
```

Lower `withConnectTimeout()` to fail faster while you debug.

### `No hosts available`

Every contact point failed. Common causes:

| Cause | Check |
| --- | --- |
| Wrong port | 9042 for plain CQL, often 9142 for TLS |
| The node advertises an unreachable address | `SELECT rpc_address FROM system.local` |
| A host or datacenter filter excludes everything | Review `withWhiteList*` and `withBlackList*` |
| TLS mismatch | See below |

The advertised address matters. A node behind NAT or inside Docker may advertise an internal
address that the client cannot reach. The driver connects to the contact point, learns the peer
list, and then fails on the peers. Set `broadcast_rpc_address` on the node.

### Authentication fails

```
Cassandra\Exception\AuthenticationException
```

Verify the credentials with `cqlsh`:

```bash
cqlsh 10.0.0.1 9042 -u app_user -p "$SCYLLA_PASSWORD"
```

A cluster running `AllowAllAuthenticator` ignores credentials entirely, so a failure means the
authenticator is on and the credentials are wrong.

### TLS handshake fails

See [TLS and SSL](/guide/tls#debugging-a-failed-handshake). The quick checks are: right port, right
CA file, and whether `VERIFY_PEER_IDENTITY` can match the certificate against the address you dial.

## Queries

### `Validation failed for type ... LongType: got 4 bytes`

A plain PHP `int` was bound to a `bigint` or `counter` column.

```php
// Wrong
['arguments' => [$id, 42]]

// Right
['arguments' => [$id, new Cassandra\Bigint(42)]]
```

The same class of error affects `smallint`, `tinyint`, `varint`, `float`, and `blob`. See
[data types](/guide/data-types#the-four-traps).

### `Class Cassandra\Numbers\Bigint not found`

Value classes are flat. Use `Cassandra\Bigint`.

```php
var_dump(array_filter(get_declared_classes(), fn ($c) => str_starts_with($c, 'Cassandra')));
```

### `InvalidQueryException: unconfigured table`

The keyspace is wrong, or the table does not exist. Connect with the keyspace, or qualify the table
name:

```php
$session = $cluster->connect('shop');
// or
$session->execute('SELECT * FROM shop.users');
```

### Queries are slow but the cluster is idle

Almost always an unprepared statement plus a coordinator hop. Prepare it. See
[performance](/guide/performance).

Confirm with metrics: a high `median` with a low server-side coordinator latency points at the extra
hop or at the client.

### A timeout on a query that used to work

Check the page size first. A `SELECT` over a growing partition transfers more each day until it
crosses the request timeout.

```php
$session->execute($statement, ['page_size' => 500, 'timeout' => 10.0]);
```

## Memory

### Memory grows while paging a large result

Hold one page at a time. Do not accumulate `Rows` objects.

```php
// Wrong: keeps every page alive.
$pages = [];
while (! $rows->isLastPage()) {
    $pages[] = $rows;
    $rows = $rows->nextPage();
}
```

Process each page, extract plain PHP values, then move on. See
[performance](/guide/performance#value-objects-and-memory).

### Memory grows across requests under PHP-FPM

Persistent sessions live for the worker lifetime. That is by design. What should not grow is the
number of sessions. Creating a session with a different configuration on each request creates a new
cached entry each time, because the cache key covers the configuration.

Build the configuration from constants, not from per-request data.

## Environment

### Different behavior between CLI and the web server

They read different ini files and often different PHP builds.

```bash
php --ini
php -i | grep -E '^(Thread Safety|Debug Build)'
```

Compare with `phpinfo()` from the web server.

### The extension works locally and fails in Docker

The image needs the runtime libraries, not just the build ones. A multi-stage build must copy the
C/C++ driver and libuv shared objects into the final image, and run `ldconfig`.

## Getting help

Include this in a bug report:

```php
<?php
printf("driver:      %s\n", Cassandra::VERSION);
printf("cpp-driver:  %s\n", Cassandra::CPP_DRIVER_VERSION);
printf("php:         %s (%s, %s)\n",
    PHP_VERSION,
    PHP_ZTS ? 'ZTS' : 'NTS',
    PHP_DEBUG ? 'debug' : 'release',
);
printf("os:          %s %s\n", PHP_OS, php_uname('r'));
```

Add the ScyllaDB or Cassandra version, the full exception with its stack trace, and a minimal script
that reproduces the problem.

Open an issue at
[github.com/he4rt/scylladb-php-driver](https://github.com/he4rt/scylladb-php-driver/issues), or ask
in the [ScyllaDB Developers Discord](https://discord.gg/B6rutCXvgp).
