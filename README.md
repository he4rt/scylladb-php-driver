<div align="center">

[![Build Docker Image](https://github.com/he4rt/scylladb-php-driver/actions/workflows/docker-image.yml/badge.svg)](https://github.com/he4rt/scylladb-php-driver/actions/workflows/docker-image.yml)
[![ScyllaDB Unnoficial Discord Server](https://img.shields.io/badge/ScyllaDB_Developers-Discord_Server-4C388C)](https://discord.gg/B6rutCXvgp)

</div>


# ScyllaDB/CassandraDB Driver for PHP

[![Build Status](https://github.com/he4rt/scylladb-php-driver/actions/workflows/tests.yml/badge.svg?branch=v1.3.x)](https://github.com/he4rt/scylladb-php-driver/actions/workflows/test.yml)

A modern, [feature-rich][Features] and highly tunable PHP client library
for [ScyllaDB](https://github.com/scylladb/scylladb) and
[Apache Cassandra] 3.0+ using exclusively Cassandra's binary protocol.

This is a wrapper around the [ScyllaDB C/C++ Driver].

## Getting the Driver

Binary versions of the driver, available for Linux systems and officialy supported versions of PHP (8.1 and 8.2), can be
obtained from GitHub Releases (soon).

You're also can compile the driver by yourself or use Dockerfile with a pre-set environment to run your tests.

## What's new in v1.2.0/v1.3.8

* ScyllaDB C/C++ Shard Aware driver implemented
* Support for [`duration`]
* `Session::execute()` and `Session::executeAsync()` now support a
  [simple string] for the query CQL and a simple array for the query execution option
* Full support for Apache Cassandra 3.0+
* Support for [`tinyint` and `smallint`]
* Support for [`date`] and [`time`]
* Support for [user-defined function and aggregate] metadata
* Support for [secondary index] and [materialized view] metadata

## Last Development Status

### v1.3.8

- Migration from C to C++
- Removing PHP Build system in favor of CMake
- Upgraded `Cassandra\Cluster\Builder` class to new PHP argument parsing API
- Reduce memory usage from `Cassandra\Cluster\Builder`
- Migrate from Behat to PestPHP
- Migrated from TravisCI to Github Actions.

## Compatibility

This driver works exclusively with the Cassandra Query Language v3 (CQL3) and
Cassandra's native protocol. The current version works with:

* ScyllaDB 4.4.x and 5.x +
* Apache Cassandra versions 3.0+
* PHP 8.1 and 8.2
* 64-bit (x64)
* Thread safe (TS) and non-thread safe (NTS)
* Compilers: GCC 13.0+, Clang 16+ and c++23

<!-- ## Documentation

* [Home]
* [API]
* [Features] -->

## Getting Help

* If you're able to fix a bug yourself, you can [fork the repository](https://help.github.com/articles/fork-a-repo/)
  and [submit a pull request](https://help.github.com/articles/using-pull-requests/) with the fix.
* If you're not able to fix a bug yourself,
  please [open an issue](https://github.com/he4rt/scylladb-php-driver/issues) , describe it with the most details
  possible and wait until one of our maintainers join the conversation.

## Quick Start

```php
<?php
$cluster   = Cassandra::cluster()                 // connects to localhost by default
                 ->build();
$keyspace  = 'system';
$session   = $cluster->connect($keyspace);        // create session, optionally scoped to a keyspace
$statement = new Cassandra\SimpleStatement(       // also supports prepared and batch statements
    'SELECT keyspace_name, columnfamily_name FROM schema_columnfamilies'
);
$querySent = $session->execute($statement);
$result    = $querySent->get();                      // wait for the result, with an optional timeout

foreach ($result as $row) {                       // results and rows implement Iterator, Countable and ArrayAccess
    printf("The keyspace %s has a table called %s\n", $row['keyspace_name'], $row['columnfamily_name']);
}
```

## Installation

Before you compile your driver, first check if your `php` and `php-config` matches the supported versions. If not,
please checkout to the available versions.

```sh
sudo update-alternatives --config php
sudo update-alternatives --config php-config
```

> Supported Versions: 8.1 and 8.2.

Next you will learn how to build the driver to:

* Release/Production
* Debug/Development
* Optimized for Production (CAREFUL!!)

## BEFORE YOU START

**!Make sure first you cloned the repository with --recursive flag!**

**!Make sure you first install LibUV and LibCassandra/LibScyllaDB!**

#### Installing LibUV

> Install LibUV from your package manager. -> Preferred Way

> Installing From Source

```sh
git clone --depth 1 -b v1.46.0 https://github.com/libuv/libuv.git \
    && cd libuv \
    && mkdir build \
    && cd build \
    && cmake -DBUILD_TESTING=OFF -DBUILD_BENCHMARKS=OFF -DLIBUV_BUILD_SHARED=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
    && ninja install
```

#### Installing LibCassandra/LibScyllaDB

> Installing From Source (Only Way to get the latest version)

```sh

# The only difference between libscylladb and libcassandra is the name of the git repository.

git clone --depth 1 https://github.com/scylladb/cpp-driver.git scyladb-driver \
  && cd scyladb-driver \
  && mkdir build \
  && cd build \
  && cmake -DCASS_CPP_STANDARD=17 -DCASS_BUILD_STATIC=ON -DCASS_BUILD_SHARED=ON -DCASS_USE_STD_ATOMIC=ON -DCASS_USE_TIMERFD=ON -DCASS_USE_LIBSSH2=ON -DCASS_USE_ZLIB=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC -Wno-error=redundant-move" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
  && ninja install
```

#### CMAKE Options and Presets

We provide common setups using CMake Presets. You can use them by passing the preset name to CMake:

* Release
* RelWithInfo
* CI (Used by GitHub Actions)
* Debug
* ReleaseLibCassandra (Release with LibCassandra)

````
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_AVX "Enable AVX" OFF)
option(ENABLE_AVX2 "Enable AVX2" OFF)
option(ENABLE_LTO "Enable LTO" OFF)
set(CPU_TYPE "x86-64-v3" CACHE STRING "For x86_64 = x86-64|x86-64-v2|x86-64-v3|x86-64-v4|native")

# PHP Options
set(CUSTOM_PHP_CONFIG "" CACHE STRING "Custom PHP config path")
set(PHP_VERSION_FOR_PHP_CONFIG "8.3" CACHE STRING "PHP version")
option(PHP_DEBUG "Debug or Release" ON)
option(PHP_THREAD_SAFE "ZTS(zts) or NTS(nts)" OFF)

option(LINK_LIBUV_STATIC "Statically link libuv" OFF)
option(BUILD_LIBUV_FROM_SRC "Build LibUV from Source" OFF)

option(PHP_DRIVER_STATIC "Statically link PHP Driver" OFF)
option(USE_LIBCASSANDRA "Use DataStax LibCassandra instead of LibScyllaDB" OFF)

````

### Compiling Release Build

This build you can use it for Production purposes.

To build your Driver, you should first download a few dependencies:

#### Debian/Ubuntu

````sh
apt install -y pipx build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev php-dev && pipx install cmake
````

After that, you can run the build command inside the repository root folder:

````sh
phpize
./configure
make -j$(nproc)
sudo make install
````

## Contributing

[Read our contribution policy][contribution-policy] for a detailed description
of the process.

## Running tests

Soon.

## Copyright

&copy; DataStax, Inc.

Licensed under the Apache License, Version 2.0 (the “License”); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

[Apache Cassandra]: http://cassandra.apache.org

[DSE PHP driver]: http://docs.datastax.com/en/developer/php-driver-dse/latest

[DataStax Enterprise]: http://www.datastax.com/products/datastax-enterprise

[DataStax C/C++ Driver for Apache Cassandra]: http://docs.datastax.com/en/developer/cpp-driver/latest

[DataStax download server]: http://downloads.datastax.com/php-driver

[GitHub]: https://github.com/datastax/php-driver

[Home]: http://docs.datastax.com/en/developer/php-driver/latest

[API]: http://docs.datastax.com/en/developer/php-driver/latest/api

[installing-details]: https://github.com/datastax/php-driver/blob/master/ext/README.md

[contribution-policy]: https://github.com/datastax/php-driver/blob/master/CONTRIBUTING.md

[Behat Framework]: http://docs.behat.org

[Features]: /tests/features

[`duration`]: /tests/features/duration.feature

[simple string]: /tests/features/simple_string_queries.feature

[`tinyint` and `smallint`]: /tests/features/datatypes.feature#L92

[`date`]: /tests/features/datatypes.feature#L135

[`time`]: /tests/features/datatypes.feature#L170

[user-defined function and aggregate]: /tests/features/function_and_aggregate_metadata.feature

[secondary index]: /tests/features/secondary_index_metadata.feature

[materialized view]: /tests/features/materialized_view_metadata.feature
