# Changelog

## Unreleased

### Added

- Experimental support for [scylladb/cpp-rs-driver](https://github.com/scylladb/cpp-rs-driver)
  as a third backend, selectable at build time via the new `PHP_DRIVER_BACKEND` CMake
  cache variable (`cassandra | scylla-cpp | scylla-rust`). Install the Rust-backed
  driver with `scripts/compile-cpp-rs-driver.sh --prefix <path>`, then configure with
  `cmake -DPHP_DRIVER_BACKEND=scylla-rust` (or use a `*ScyllaRust` preset).
- New CMake presets `*PHP*<ver><ts>ScyllaRust` covering the third backend across PHP
  versions and build types.

### Deprecated

- `USE_LIBCASSANDRA` CMake option is now an alias for `PHP_DRIVER_BACKEND=cassandra`
  and will be removed in a future release. Use `PHP_DRIVER_BACKEND` explicitly.

### Known limitations (scylla-rust backend)

- `Cassandra\Cluster\Builder::withMaxConnectionsPerHost()` is a no-op (removed upstream).
- Schema introspection of column clustering order, keyspace metadata via name, and
  table/materialized-view options will return empty values or throw — the upstream
  declarations of `cass_*_meta_field_by_name` and `cass_iterator_fields_from_*` are
  intentionally no-ops in the Rust driver (see upstream "Functions intentionally not
  implemented" docs). Our call sites null-check the returned pointer/iterator so the
  extension keeps working; affected PHP-level operations just yield empty results.

## [v1.3.17](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2026-04-25)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.16...v1.3.17)

## [v1.3.16](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2026-04-25)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.15...v1.3.16)

**Implemented enhancements:**

- Builds using a Dockerfile [\#116](https://github.com/he4rt/scylladb-php-driver/issues/116)
- PECL Support [\#90](https://github.com/he4rt/scylladb-php-driver/issues/90)

**Closed issues:**

- 1.3.11 release doesn't have a changelog [\#109](https://github.com/he4rt/scylladb-php-driver/issues/109)

## [v1.3.15](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2026-04-25)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.14...v1.3.15)

## [v1.3.14](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2026-04-25)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.13...v1.3.14)

**Closed issues:**

- Unnecessary symbols marked as PHP\_SCYLLADB\_API [\#119](https://github.com/he4rt/scylladb-php-driver/issues/119)
- Conflicts with php8.4-ds [\#118](https://github.com/he4rt/scylladb-php-driver/issues/118)
- Persistent Sessions aren't cleaned up or reused properly [\#110](https://github.com/he4rt/scylladb-php-driver/issues/110)

**Merged pull requests:**

- refactor: extension modernization wave 1 \(test consolidation + macro purge + correctness fixes\) [\#122](https://github.com/he4rt/scylladb-php-driver/pull/122) ([CodeLieutenant](https://github.com/CodeLieutenant))

## [v1.3.13](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2025-01-27)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.12...v1.3.13)

**Implemented enhancements:**

- Building, Packaging and Dristribution of Extension [\#13](https://github.com/he4rt/scylladb-php-driver/issues/13)

## [v1.3.12](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2025-01-26)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.11...v1.3.12)

**Implemented enhancements:**

- When compiled version of extension [\#98](https://github.com/he4rt/scylladb-php-driver/issues/98)
- feature\(scripts\): compilation scripts for libuv and cpp drivers [\#104](https://github.com/he4rt/scylladb-php-driver/pull/104) ([CodeLieutenant](https://github.com/CodeLieutenant))
- feature\(ci\): add release github actions [\#103](https://github.com/he4rt/scylladb-php-driver/pull/103) ([CodeLieutenant](https://github.com/CodeLieutenant))
- feature\(dockerimage\): better docker image [\#102](https://github.com/he4rt/scylladb-php-driver/pull/102) ([CodeLieutenant](https://github.com/CodeLieutenant))

**Fixed bugs:**

- \[bug\] undefined symbol in latest [\#100](https://github.com/he4rt/scylladb-php-driver/issues/100)
- Fixing issue with undefined symbol 'php\_stat' -\> External C Linking E… [\#101](https://github.com/he4rt/scylladb-php-driver/pull/101) ([CodeLieutenant](https://github.com/CodeLieutenant))

**Merged pull requests:**

- Add bounds checking to prevent overflow warnings during build. [\#105](https://github.com/he4rt/scylladb-php-driver/pull/105) ([ciaran-moore](https://github.com/ciaran-moore))

## [v1.3.11](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2024-06-09)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.10...v1.3.11)

## [v1.3.10](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2024-04-12)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.9...v1.3.10)

**Implemented enhancements:**

- Prepare extension for 8.3 Release [\#65](https://github.com/he4rt/scylladb-php-driver/issues/65)
- parser\_state and token\_type One Definition C++ Rule warnings [\#94](https://github.com/he4rt/scylladb-php-driver/pull/94) ([ciaran-moore](https://github.com/ciaran-moore))

**Fixed bugs:**

- cmake Release build fails on Debian 11 [\#93](https://github.com/he4rt/scylladb-php-driver/issues/93)
- Php 8.2 issue [\#88](https://github.com/he4rt/scylladb-php-driver/issues/88)

**Merged pull requests:**

- Fix missing symbols from math.cpp [\#97](https://github.com/he4rt/scylladb-php-driver/pull/97) ([ecsv](https://github.com/ecsv))

## [v1.3.9](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2023-11-23)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.8...v1.3.9)

**Implemented enhancements:**

- \[ERROR\] Could not retrieve sharding info from connection [\#73](https://github.com/he4rt/scylladb-php-driver/issues/73)
- Upgrade Pest to V2 [\#71](https://github.com/he4rt/scylladb-php-driver/issues/71)
- Dynamic linking of external libraries [\#70](https://github.com/he4rt/scylladb-php-driver/issues/70)
- Improvements: Create a Enum Class for CL  [\#61](https://github.com/he4rt/scylladb-php-driver/issues/61)
- Remove PHPIZE from CMake [\#20](https://github.com/he4rt/scylladb-php-driver/issues/20)
- Staticly link LibCassandra and LibUV [\#19](https://github.com/he4rt/scylladb-php-driver/issues/19)
- Use PHP Stubs [\#15](https://github.com/he4rt/scylladb-php-driver/issues/15)
- Better IDE support [\#14](https://github.com/he4rt/scylladb-php-driver/issues/14)
- Document Driver [\#12](https://github.com/he4rt/scylladb-php-driver/issues/12)
- Add ScyllaDB support [\#11](https://github.com/he4rt/scylladb-php-driver/issues/11)
- feat\(stubs\): Retry Policy [\#85](https://github.com/he4rt/scylladb-php-driver/pull/85) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Ready for 8.3 [\#83](https://github.com/he4rt/scylladb-php-driver/pull/83) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Feat/libcassandra support [\#81](https://github.com/he4rt/scylladb-php-driver/pull/81) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Feat/cmake opts [\#74](https://github.com/he4rt/scylladb-php-driver/pull/74) ([CodeLieutenant](https://github.com/CodeLieutenant))
- test: cassandra tuples test [\#58](https://github.com/he4rt/scylladb-php-driver/pull/58) ([Canhassi12](https://github.com/Canhassi12))
- Migration Result Paging test [\#53](https://github.com/he4rt/scylladb-php-driver/pull/53) ([danielhe4rt](https://github.com/danielhe4rt))
- Move every file to C++ [\#51](https://github.com/he4rt/scylladb-php-driver/pull/51) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Optimizing cluster builder [\#50](https://github.com/he4rt/scylladb-php-driver/pull/50) ([CodeLieutenant](https://github.com/CodeLieutenant))
- feat: migrating tests to PestPHP [\#49](https://github.com/he4rt/scylladb-php-driver/pull/49) ([danielhe4rt](https://github.com/danielhe4rt))
- Setup Github Actions as CI server [\#48](https://github.com/he4rt/scylladb-php-driver/pull/48) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Feat/pest testing [\#45](https://github.com/he4rt/scylladb-php-driver/pull/45) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Optimize builder [\#44](https://github.com/he4rt/scylladb-php-driver/pull/44) ([maxm86545](https://github.com/maxm86545))

**Fixed bugs:**

- \[ERROR\] Undefined symbol: \_Z8php\_statP12\_zend\_stringiP12\_zval\_struct [\#80](https://github.com/he4rt/scylladb-php-driver/issues/80)
- \[ERROR\] CMAKE not find PHP\_CONFIG\_EXECUTABLE [\#79](https://github.com/he4rt/scylladb-php-driver/issues/79)
- \[ERROR\] Fatal: The remote end hung up unexpectedly [\#77](https://github.com/he4rt/scylladb-php-driver/issues/77)
- Timestamp "toDateTime\(\)" function is not working [\#75](https://github.com/he4rt/scylladb-php-driver/issues/75)
- Missing information on build steps [\#69](https://github.com/he4rt/scylladb-php-driver/issues/69)
- withConnectionsPerHost\(\) has "core" and "max" arguments mixed up [\#64](https://github.com/he4rt/scylladb-php-driver/issues/64)
- \Cassandra\Timeuuid::toDateTime\(\) does not work [\#63](https://github.com/he4rt/scylladb-php-driver/issues/63)
- Problems with getting a custom index metadata [\#59](https://github.com/he4rt/scylladb-php-driver/issues/59)
- Fix deprication issues for Countable implementation [\#10](https://github.com/he4rt/scylladb-php-driver/issues/10)
- Fix core and max parameter comparison [\#67](https://github.com/he4rt/scylladb-php-driver/pull/67) ([evll](https://github.com/evll))
- Fixing issue with DateTime conversion from Cassandra Date, Time, Time… [\#66](https://github.com/he4rt/scylladb-php-driver/pull/66) ([CodeLieutenant](https://github.com/CodeLieutenant))

**Closed issues:**

- clone error - "Please make sure you have the correct access rights and the repository exists." [\#76](https://github.com/he4rt/scylladb-php-driver/issues/76)
- Can i install it on windows  [\#62](https://github.com/he4rt/scylladb-php-driver/issues/62)
- Extension Testing [\#43](https://github.com/he4rt/scylladb-php-driver/issues/43)
- Improvements to Installation and project docs [\#40](https://github.com/he4rt/scylladb-php-driver/issues/40)
- PHP 8.2 support [\#39](https://github.com/he4rt/scylladb-php-driver/issues/39)
- Contributing? [\#37](https://github.com/he4rt/scylladb-php-driver/issues/37)
- Drop Support for PHP 7 [\#18](https://github.com/he4rt/scylladb-php-driver/issues/18)

**Merged pull requests:**

- Feat/cassandra 4.0 support [\#84](https://github.com/he4rt/scylladb-php-driver/pull/84) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Update FindPHP and FindPHPConfig [\#82](https://github.com/he4rt/scylladb-php-driver/pull/82) ([CodeLieutenant](https://github.com/CodeLieutenant))
- Fix: Git module url sanitizers-cmake [\#78](https://github.com/he4rt/scylladb-php-driver/pull/78) ([iamrafaelmelo](https://github.com/iamrafaelmelo))
- removing warnings and adding tests to uuids [\#72](https://github.com/he4rt/scylladb-php-driver/pull/72) ([danielhe4rt](https://github.com/danielhe4rt))
- Adding new build Type with address and ub sanitizer [\#60](https://github.com/he4rt/scylladb-php-driver/pull/60) ([CodeLieutenant](https://github.com/CodeLieutenant))
- test: migrating ClientSideTimestampsTest to pest [\#57](https://github.com/he4rt/scylladb-php-driver/pull/57) ([PedroPMS](https://github.com/PedroPMS))
- chore: installation docs \(WIP\) [\#56](https://github.com/he4rt/scylladb-php-driver/pull/56) ([danielhe4rt](https://github.com/danielhe4rt))
- feat: Using Cassandra user defined types from schema metadata test [\#55](https://github.com/he4rt/scylladb-php-driver/pull/55) ([Canhassi12](https://github.com/Canhassi12))
- docs: readme improvements with installation instructions [\#52](https://github.com/he4rt/scylladb-php-driver/pull/52) ([danielhe4rt](https://github.com/danielhe4rt))
- Remove php build system in favour of CMake [\#42](https://github.com/he4rt/scylladb-php-driver/pull/42) ([CodeLieutenant](https://github.com/CodeLieutenant))
- chore\(docs\): improving project base readme [\#41](https://github.com/he4rt/scylladb-php-driver/pull/41) ([danielhe4rt](https://github.com/danielhe4rt))

## [v1.3.8](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2022-09-17)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.7...v1.3.8)

**Implemented enhancements:**

- Using a parameterised value for TTL throws an exception [\#4](https://github.com/he4rt/scylladb-php-driver/issues/4)

**Closed issues:**

- PECL ownership \[discussion\] [\#30](https://github.com/he4rt/scylladb-php-driver/issues/30)
- Error when compiling extension [\#26](https://github.com/he4rt/scylladb-php-driver/issues/26)

**Merged pull requests:**

- PHP 8.2 compatibility [\#36](https://github.com/he4rt/scylladb-php-driver/pull/36) ([remicollet](https://github.com/remicollet))
- use pkg-config and allow build when cpp-driver use CASS\_INSTALL\_HEADE… [\#28](https://github.com/he4rt/scylladb-php-driver/pull/28) ([remicollet](https://github.com/remicollet))

## [v1.3.7](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2022-06-16)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.6...v1.3.7)

**Implemented enhancements:**

- PHP 8.1 support [\#8](https://github.com/he4rt/scylladb-php-driver/issues/8)

**Closed issues:**

- Docker Images [\#16](https://github.com/he4rt/scylladb-php-driver/issues/16)

**Merged pull requests:**

- fix proto for PHP 8.1 [\#29](https://github.com/he4rt/scylladb-php-driver/pull/29) ([remicollet](https://github.com/remicollet))

## [v1.3.6](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2022-03-29)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.5...v1.3.6)

## [v1.3.5](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2022-03-29)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.4...v1.3.5)

## [v1.3.4](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2022-03-29)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.3...v1.3.4)

**Closed issues:**

- Extension compiling error [\#7](https://github.com/he4rt/scylladb-php-driver/issues/7)
- Update readme/manuals [\#5](https://github.com/he4rt/scylladb-php-driver/issues/5)
- Build status [\#2](https://github.com/he4rt/scylladb-php-driver/issues/2)

## [v1.3.3](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2021-06-10)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.2...v1.3.3)

## [v1.3.2](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2017-08-11)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.1...v1.3.2)

## [v1.3.1](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2017-05-16)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.3.0...v1.3.1)

## [v1.3.0](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2017-03-13)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.2.2...v1.3.0)

## [v1.2.2](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2016-08-08)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.2.1...v1.2.2)

## [v1.2.1](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2016-07-28)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.2.0...v1.2.1)

## [v1.2.0](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2016-07-18)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.1.0...v1.2.0)

## [v1.1.0](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2016-02-11)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.0.1...v1.1.0)

## [v1.0.1](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2015-11-20)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.0.0...v1.0.1)

## [v1.0.0](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2015-09-14)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.0.0-rc...v1.0.0)

## [v1.0.0-rc](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2015-07-28)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.0.0-beta...v1.0.0-rc)

## [v1.0.0-beta](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2015-05-26)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/v1.0.0-alpha...v1.0.0-beta)

## [v1.0.0-alpha](https://github.com/he4rt/scylladb-php-driver/releases/tag/v1.3.17) (2015-04-02)

[Full Changelog](https://github.com/he4rt/scylladb-php-driver/compare/b8465f84360cd92e4e259a3f986cc4e35afa90de...v1.0.0-alpha)



\* *This Changelog was automatically generated by [github_changelog_generator](https://github.com/github-changelog-generator/github-changelog-generator)*
