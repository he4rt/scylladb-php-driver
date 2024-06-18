# ScyllaDB/CassandraDB Driver for PHP

[![Build Status](https://github.com/he4rt/scylladb-php-driver/actions/workflows/tests.yml/badge.svg?branch=v1.3.x)](https://github.com/he4rt/scylladb-php-driver/actions/workflows/tests.yml)

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
option(PHP_SCYLLADB_ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(PHP_SCYLLADB_ENABLE_COVERAGE "Enable coverage" OFF)
option(PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE "Optimise for current machine" OFF)


option(PHP_SCYLLADB_LIBUV_STATIC "Statically link libuv" OFF)
option(PHP_SCYLLADB_LIBUV_FROM_SRC "Build LibUV from Source" OFF)
option(PHP_SCYLLADB_LIBSCYLLADB_STATIC "Statically link LibScyllaDB" OFF)
option(PHP_SCYLLADB_LIBSCYLLADB_FROM_SRC "Build LibScyllaDB from Source" OFF)

option(PHP_SCYLLADB_USE_LIBCASSANDRA "Use DataStax LibCassandra instead of LibScyllaDB" OFF)
option(PHP_SCYLLADB_LIBCASSANDRA_STATIC "Statically link LibCassandra" OFF)
option(PHP_SCYLLADB_LIBCASSANDRA_FROM_SRC "Build LibCassandra from Source" OFF)

````

### Compiling Release Build

This build you can use it for Production purposes.

To build your Driver, you should first download a few dependencies:

#### Debian/Ubuntu

````sh
apt install -y python3 python3-pip unzip mlocate build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev php-dev
pip3 install cmake

# Single Line
apt install -y python3 python3-pip unzip mlocate build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev php-dev && pip3 install cmake
````

After that, you can run the build command inside the repository root folder:

````sh
cmake --preset Release  && cd out/Release && sudo ninja install
````

> `ninja install` needs root privileges.

After compiled, you will be at directory `scylladb-php-driver/out/Release` and at this folder you will need to move
the `cassandra.so` and `cassandra.ini` to PHP respective folders.

````shell
# current directory: scylladb-php-driver/out/Release

# PHP 8.1
sudo cp ../../cassandra.ini /etc/php/8.1/cli/conf.d/10-cassandra.ini
sudo cp cassandra.so /usr/lib/php/20210902/cassandra.so

# PHP 8.2
sudo cp ../../cassandra.ini /etc/php/8.2/cli/conf.d/10-cassandra.ini
sudo cp cassandra.so /usr/lib/php/20220829/cassandra.so
````

### Compiling Development Build

If you want to contribute to the project, you should follow the steps below.

#### Debian/Ubuntu

```shell
apt update -y
apt upgrade -y
apt install -y python3 python3-pip unzip mlocate build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev
pip3 install cmake cqlsh # CQL Shell - to connect into your ScyllaDB Cluster
install-php-extensions intl zip pcntl gmp composer
apt-get clean

# Single Line
apt update -y && apt upgrade -y && apt install -y python3 python3-pip unzip mlocate build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev && pip3 install cmake cqlsh && install-php-extensions intl zip pcntl gmp composer && apt-get clean

```

After that, you can run the build command inside the repository root folder:

````sh
cmake --preset Debug && cd out/Debug && ninja
````

We have a "debug.php" file in the root folder that you can use it for try connection with
the ScyllaDB Cluster (localhost) and check if is everything ok after change the source code.

Also you can run the PestPHP test suits inside the project and check if the functions are working as expected.

````shell
## Debug Base Command
php -d "extension=$(pwd)/out/Debug/cassandra.so" debug.php

## PestPHP Test Suite
./vendor/bin/pest -d "extension=$(pwd)/out/Debug/cassandra.so"
````

### Compiling Optimized Build

The "optimized build" brings a flag that makes the driver runs perfectly with your CPU. So, if you will use it, remember
to build it inside the environment that you will use it.

#### Debian/Ubuntu

````sh
apt install -y python3 python3-pip unzip mlocate build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev php-dev
pip3 install cmake

# Single Line
apt install -y python3 python3-pip unzip mlocate build-essential ninja-build libssl-dev libgmp-dev zlib1g-dev openssl libpcre3-dev php-dev && pip3 install cmake
````

The only difference between the `Release` and `Optimized` is a flag
called `-DPHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE=ON`.

````shell
cmake --preset Release -DPHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE=ON && cd out/Release && sudo ninja install
````

> `ninja install` needs root privileges.

After compiled, you will be at directory `scylladb-php-driver/out/Release` and at this folder you will need to move
the `cassandra.so` and `cassandra.ini` to PHP respective folders.

````shell
# current directory: scylladb-php-driver/out/Release

# PHP 8.1
sudo cp ../../cassandra.ini /etc/php/8.1/cli/conf.d/10-cassandra.ini
sudo cp cassandra.so /usr/lib/php/20210902/cassandra.so

# PHP 8.2
sudo cp ../../cassandra.ini /etc/php/8.2/cli/conf.d/10-cassandra.ini
sudo cp cassandra.so /usr/lib/php/20220829/cassandra.so
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
