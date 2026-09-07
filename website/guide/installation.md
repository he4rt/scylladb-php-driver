# Installation

The extension is compiled from source. It links against two native libraries:

- **libuv** — the event loop used by the C/C++ driver.
- **The ScyllaDB C/C++ driver**, or the DataStax `libcassandra` driver as an alternative.

Install both before you build the extension.

::: warning 64-bit only
The extension supports x86-64 only. A 32-bit PHP build cannot load it.
:::

## 1. Install the native dependencies

The repository ships build scripts for both libraries. Pick a prefix that your user can write to.

```bash
git clone https://github.com/he4rt/scylladb-php-driver.git
cd scylladb-php-driver

./scripts/compile-libuv.sh --prefix ~/.local
./scripts/compile-cpp-driver.sh --driver scylladb --prefix ~/.local
```

To build against Apache Cassandra through the DataStax driver, change the `--driver` value:

```bash
./scripts/compile-cpp-driver.sh --driver cassandra --prefix ~/.local
```

Export the `pkg-config` path so the build can find both libraries:

```bash
export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

Add that line to your shell profile. The build fails with a "cpp driver not found" error when the
path is missing.

### System packages

::: code-group

```bash [Debian / Ubuntu]
sudo apt install -y build-essential ninja-build cmake \
    libssl-dev libgmp-dev zlib1g-dev libpcre3-dev
```

```bash [Fedora / RHEL]
sudo dnf install -y gcc gcc-c++ ninja-build cmake \
    openssl-devel gmp-devel zlib-devel pcre2-devel
```

```bash [macOS]
brew install cmake ninja openssl@3 gmp zlib pcre2
```

:::

GMP is required. The driver uses it for `varint` and `decimal` arithmetic.

## 2. Install the extension

### With PIE (recommended)

[PIE](https://github.com/php/pie) is the official PHP extension installer. It downloads, builds, and
enables the extension in one command.

```bash
pie install codelieutenant/scylla-driver
```

To link against DataStax `libcassandra` instead of the ScyllaDB driver:

```bash
pie install codelieutenant/scylla-driver --enable-libcassandra
```

PIE puts the compiled `cassandra.so` in the PHP extension directory and enables it.

Other configure options that PIE accepts:

| Option | Effect |
| --- | --- |
| `--enable-debug` | Build with debug symbols and assertions |
| `--enable-lto` | Enable link time optimization |
| `--enable-avx` / `--enable-avx2` | Enable the matching instruction set |
| `--with-cpu-type=<type>` | Target a micro-architecture, for example `x86-64-v3` |
| `--enable-driver-static` | Link the C/C++ driver statically |
| `--enable-sanitizers` | Link AddressSanitizer and UndefinedBehaviorSanitizer |

### With CMake, from source

The repository uses CMake presets. A preset name follows the pattern
`<BuildType>PHP<Version><ThreadModel>`, for example `ReleasePHP8.4NTS`.

```bash
cmake --list-presets
cmake --preset ReleasePHP8.4NTS
cmake --build out/ReleasePHP8.4NTS
```

The build produces `out/<preset>/cassandra.so` on Linux and `cassandra.dylib` on macOS. Copy it to
your PHP extension directory, or load it by absolute path:

```bash
php -d extension="$PWD/out/ReleasePHP8.4NTS/cassandra.so" -m | grep cassandra
```

To find the extension directory:

```bash
php -i | grep '^extension_dir'
```

After you add a new PHP version, regenerate the presets:

```bash
php generate-presets.php
```

#### Optional build features

A preset covers the PHP version, the build type and the thread model. Everything else is a CMake
cache variable you pass on the configure line.

| Variable | Default | What it adds |
| --- | --- | --- |
| `PHP_SCYLLADB_ENABLE_POLL_API` | `OFF` | `Cassandra\Async\Poll` and `Cassandra\Async\PollHandle` over PHP 8.6 `Io\Poll`. `AUTO` enables it when the PHP provides `main/php_poll.h`, `ON` requires that header. |
| `PHP_SCYLLADB_ENABLE_SWOOLE` | `OFF` | Coroutine-aware `Future::get()` under Swoole. Needs `PHP_SCYLLADB_SWOOLE_SRC`. |
| `PHP_SCYLLADB_ENABLE_OPENSWOOLE` | `OFF` | The same for OpenSwoole. |
| `PHP_SCYLLADB_SWOOLE_SRC` | empty | Path to the `(open)swoole-src` tree that provides `swoole.h`. |
| `PHP_SCYLLADB_ENABLE_LEGACY_SCHEMA_META` | `OFF` | `Cassandra\Function_`, `Aggregate` and `Index`. The Rust backend leaves the matching `cass_*_meta_*` calls unimplemented. |
| `PHP_SCYLLADB_ENABLE_CUSTOM_TYPE` | `OFF` | `Cassandra\Custom` and `Cassandra\Type\Custom`. The Rust backend does not plan to implement `cass_*_bind_custom`. |
| `PHP_SCYLLADB_BACKEND` | `scylla-cpp` | `cassandra`, `scylla-cpp` or `scylla-rust`. |
| `PHP_SCYLLADB_STATIC` | `OFF` | Link the C/C++ driver statically. |

```bash
cmake --preset ReleasePHP8.6NTS -DPHP_SCYLLADB_ENABLE_POLL_API=AUTO
cmake --build out/ReleasePHP8.6NTS
```

See [event loops](/guide/event-loops) for what the async options give you.

### With phpize

```bash
phpize
./configure --with-cpu-type=x86-64-v3
make -j"$(nproc)"
sudo make install
```

## 3. Enable and verify

Add the extension to your `php.ini`, or to a file in the `conf.d` directory:

```ini
extension=cassandra.so

[cassandra]
; Log level: CRITICAL | ERROR | WARN | INFO | DEBUG | TRACE
cassandra.log_level = ERROR

; Log file path. An empty value sends the log to stderr.
cassandra.log = /var/log/php-cassandra.log

; Show the real password in the Cluster\Builder properties. Development only.
cassandra.expose_credentials = Off
```

Every setting is `PHP_INI_SYSTEM`. See [metrics and logging](/guide/observability#logging).

Check the result:

```bash
php -m | grep cassandra
php -r 'echo Cassandra::VERSION, " on cpp-driver ", Cassandra::CPP_DRIVER_VERSION, PHP_EOL;'
```

The second command prints the extension version and the version of the C/C++ driver it links
against.

## A local ScyllaDB node

To develop against a real node, start one with the helper script. It runs the Compose file in
`docker/`.

```bash
./scripts/run-scylladb.sh
```

The node listens on `127.0.0.1:9042`. The default ScyllaDB image accepts any credentials, so
`withCredentials()` is optional against a local node.
