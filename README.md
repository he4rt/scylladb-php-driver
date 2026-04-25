<div align="center">

# ScyllaDB PHP Driver

**A modern PHP extension for ScyllaDB and Apache Cassandra**

[![Tests](https://github.com/he4rt/scylladb-php-driver/actions/workflows/tests.yml/badge.svg?branch=v1.3.x)](https://github.com/he4rt/scylladb-php-driver/actions/workflows/tests.yml)
[![Build Docker Image](https://github.com/he4rt/scylladb-php-driver/actions/workflows/docker-image.yml/badge.svg)](https://github.com/he4rt/scylladb-php-driver/actions/workflows/docker-image.yml)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![PHP](https://img.shields.io/badge/PHP-8.2%20%7C%208.3%20%7C%208.4%20%7C%208.5-8892BF)](https://www.php.net)
[![ScyllaDB](https://img.shields.io/badge/ScyllaDB-Developers-4C388C)](https://discord.gg/B6rutCXvgp)

</div>

A high-performance PHP extension for [ScyllaDB](https://www.scylladb.com) and [Apache Cassandra](https://cassandra.apache.org) 3.0+, built on top of the [ScyllaDB C/C++ Driver](https://github.com/scylladb/cpp-driver). Communicates exclusively over the native CQL binary protocol.

The extension is actively being **migrated from C++ to C23** for improved performance, safety, and maintainability.

---

## Compatibility

| Component | Supported versions |
|---|---|
| **PHP** | 8.2, 8.3, 8.4, 8.5 |
| **ScyllaDB** | 4.4.x, 5.x, 6.x |
| **Apache Cassandra** | 3.0+ (via DataStax libcassandra) |
| **Architecture** | x86-64 (64-bit only) |
| **Thread safety** | NTS and ZTS |
| **Compilers** | GCC 13+, Clang 16+ |
| **OS** | Linux, macOS |

---

## Quick Start

```php
<?php

$session = Cassandra::cluster()
    ->withContactPoints('127.0.0.1')
    ->withPort(9042)
    ->withCredentials('cassandra', 'cassandra')
    ->withTokenAwareRouting(true)
    ->build()
    ->connect('my_keyspace');

// Simple string query
$session->execute("INSERT INTO users (id, name) VALUES (uuid(), 'Alice')");

// Prepared statement with bound values
$prepared = $session->prepare('SELECT * FROM users WHERE id = ?');
$result = $session->execute($prepared, ['arguments' => [$id]]);

foreach ($result as $row) {
    printf("User: %s\n", $row['name']);
}
```

---

## Installation

### Prerequisites

The driver requires **libuv** and the **ScyllaDB C/C++ driver** (or DataStax libcassandra). Use the provided scripts to build them from source:

```bash
# Install libuv (latest stable)
./scripts/compile-libuv.sh --prefix ~/.local

# Install the ScyllaDB C/C++ driver
./scripts/compile-cpp-driver.sh --driver scylladb --prefix ~/.local

# Build PHP with debug symbols (optional, for development)
./scripts/compile-php.sh -v 8.4 -d -o ./php
```

> **System packages (Debian/Ubuntu)**
> ```bash
> apt install -y build-essential ninja-build cmake \
>     libssl-dev libgmp-dev zlib1g-dev libpcre3-dev
> ```

---

## Building the Extension

The project uses **CMake** with presets for common configurations. Preset names follow the pattern `<BuildType>PHP<Version><ThreadModel>`, e.g. `DebugPHP8.4NTS`.

### Configure and compile

```bash
# List all available presets
cmake --list-presets

# Configure (e.g. debug build, PHP 8.4, non-thread-safe)
cmake --preset DebugPHP8.4NTS

# Compile
cmake --build out/DebugPHP8.4NTS
```

### Available build types

| Preset prefix | Description |
|---|---|
| `Debug` | Debug symbols, no optimisations |
| `Release` | Fully optimised |
| `RelWithDebugInfo` | Optimised with debug info |

### CMake options

```cmake
option(ENABLE_SANITIZERS   "Enable AddressSanitizer + UndefinedSanitizer" OFF)
option(ENABLE_AVX          "Enable AVX instruction set"                   OFF)
option(ENABLE_AVX2         "Enable AVX2 instruction set"                  OFF)
option(ENABLE_LTO          "Enable Link-Time Optimisation"                OFF)

set(CPU_TYPE "x86-64-v3" CACHE STRING
    "x86-64 micro-arch: x86-64 | x86-64-v2 | x86-64-v3 | x86-64-v4 | native")

# PHP
set(PHP_VERSION_FOR_PHP_CONFIG "8.4" CACHE STRING "PHP version")
option(PHP_DEBUG       "Debug build of PHP"    ON)
option(PHP_THREAD_SAFE "ZTS (thread-safe) PHP" OFF)

# Linking
option(LINK_LIBUV_STATIC    "Statically link libuv"             OFF)
option(PHP_DRIVER_STATIC    "Statically link the PHP driver"    OFF)
option(USE_LIBCASSANDRA     "Use DataStax libcassandra instead" OFF)
```

### Regenerating CMake presets

After adding a new PHP version, regenerate `CMakePresets.json`:

```bash
php generate-presets.php
```

---

## Running Tests

Start a local ScyllaDB node, then run the Pest test suite:

```bash
# 1. Start ScyllaDB via Docker Compose
./scripts/run-scylladb.sh

# 2. Build the extension
cmake --preset DebugPHP8.4NTS
cmake --build out/DebugPHP8.4NTS

# 3. Install Composer dependencies
composer install

# 4. Run tests
php ./vendor/bin/pest
```

Environment variables for the test suite:

| Variable | Default | Description |
|---|---|---|
| `SCYLLADB_HOSTS` | `127.0.0.1` | Comma-separated list of nodes |
| `SCYLLADB_PORT` | `9042` | CQL native port |
| `SCYLLADB_USERNAME` | `cassandra` | Username |
| `SCYLLADB_PASSWORD` | `cassandra` | Password |
| `SCYLLADB_KEYSPACE` | _(empty)_ | Default keyspace |

---

## Development Status

The extension is undergoing an incremental **C++ → C23 migration**. The `src/Cluster/` module is the canonical reference implementation.

| Module | Status |
|---|---|
| `Cluster` | Refactored (C23 reference) |
| `RetryPolicy` | Partial — stubs done, handlers need C port |
| `DateTime` | Partial — stubs done |
| `SSLOptions` | Partial — stubs done |
| `Database` | Legacy |
| `Type` | Legacy |
| `Numbers` | Legacy |
| `TimestampGenerator` | Legacy |
| `Exception` | Legacy (thin wrappers) |

---

## Contributing

- **Bug reports** — please include driver version, PHP version, ScyllaDB version, dependency versions, a full stack trace, and steps to reproduce.
- **Pull requests** — fork the repository and open a PR. See [CONTRIBUTING.md](CONTRIBUTING.md) for the full contribution guide.
- **Questions / discussions** — join the [ScyllaDB Developers Discord](https://discord.gg/B6rutCXvgp).

---

## License

Copyright &copy; DataStax, Inc. and contributors.

Licensed under the [Apache License, Version 2.0](LICENSE).
