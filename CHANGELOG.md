# Changelog

## Unreleased

### Changed (breaking)

- The per-worker persistent caches are now bounded by default: 16 clusters, 16 sessions and
  1000 prepared statements, was unlimited for all three. Past a cap the resource is still
  created, it is just not cached, and the driver writes one `E_WARNING` per request. The caps
  are runaway guards set to about ten times normal use, sized from the measured cost per entry
  per worker: a cluster is about 12 KB, a session about 2.2 MB plus 2 sockets per node, and a
  prepared statement about 7.4 KB. Set any of them to `-1` for the old unbounded behaviour.

- The default consistency is now `LOCAL_QUORUM`, was `LOCAL_ONE`. A statement that sets no
  consistency now reads and writes a majority of the replicas in its local datacenter, so a
  read sees the last write. The old default could serve a read from a replica that had not
  seen it yet.

  **What can break.** A query at `LOCAL_QUORUM` fails when too few replicas in the local
  datacenter are up. With replication factor 3 the cluster tolerates one node down instead of
  two. Latency also rises, because the coordinator waits for a majority instead of one replica.

  **How to keep the old behaviour.** Set one line in `php.ini`:

  ```ini
  cassandra.default_consistency = LOCAL_ONE
  ```

  Or set it per cluster with `withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_ONE)`, or per
  statement with the `consistency` execution option. Prefer keeping `LOCAL_QUORUM` and dropping
  to `LOCAL_ONE` only on the queries that value latency over freshness.

  Do not switch to plain `QUORUM` to get a stronger guarantee. `QUORUM` counts replicas across
  every datacenter, so it adds cross-datacenter latency to every query and fails when a remote
  datacenter is unreachable.

### Added

- `Statement::setIdempotent(bool $idempotent = true): static` and
  `Statement::isIdempotent(): ?bool`, on `SimpleStatement`, `PreparedStatement` and
  `BatchStatement`, plus a matching `idempotent` execution option. The driver only retries a
  statement after a timeout, and only runs it speculatively, when the statement is idempotent.
  Until now the extension had no way to set that flag, so the speculative execution policy on
  the cluster builder and on an execution profile never applied to any statement. A statement
  carries no setting by default and `isIdempotent()` then returns `null`. A per-call
  `idempotent` option overrides the statement. For a batch the driver reads the flag off the
  batch, so a flag on a statement added to the batch has no effect.
- `Cassandra\ProtocolVersion`, an int-backed enum with the cases `V1` to `V5`. The backing value
  is the protocol version byte, so it matches `CASS_PROTOCOL_VERSION_V1` to
  `CASS_PROTOCOL_VERSION_V5`. `Cluster\Builder::withProtocolVersion()` accepts a case of this
  enum, and still accepts a plain integer for a version the enum does not name.
- `Rows::wasApplied(): bool`, which reads the `[applied]` column of a lightweight transaction
  result. A statement with no condition has no such column, and the method then returns `true`,
  so you can call it on any result. The `[applied]` column stays readable through `first()` and
  array access.
- Bounds on the per-worker persistent caches: `cassandra.max_persistent_clusters`,
  `cassandra.max_persistent_sessions` and `cassandra.max_persistent_prepared_statements`
  (`-1` unlimited, `0` disabled). Past a cap the resource is still created but not cached,
  and the driver writes one `E_WARNING` per request. Before this, an application that built
  CQL by string concatenation grew `EG(persistent_list)` until the worker restarted.
- `cassandra.allow_persistent` (default `1`), an operator kill switch for all three caches.
  With it off, `Cluster\Builder::withPersistentSessions(true)` cannot re-enable caching.
- `php.ini` seeds for every `Cluster\Builder` default: `cassandra.contact_points`, `.port`,
  `.connect_timeout`, `.request_timeout`, `.default_consistency`, `.default_page_size`,
  `.protocol_version`, `.io_threads`, `.core_connections_per_host`, `.max_connections_per_host`,
  `.reconnect_interval`, `.connection_heartbeat_interval`, `.tcp_keepalive_delay`,
  `.tcp_nodelay`, `.token_aware_routing`, `.latency_aware_routing`, `.schema_metadata`,
  `.hostname_resolution` and `.randomized_contact_points`. Each `with*()` method still
  overrides its seed. All are `PHP_INI_SYSTEM`, because the values form the persistent-cluster
  cache key and a request-scoped `ini_set()` would grow the cache without bound.
- `Cassandra\ExecutionProfile`, a named bundle of execution settings. Register any number of
  them with `Cluster\Builder::withExecutionProfile($name, $profile)` and select one per
  call with the third argument of `execute()` and `executeAsync()`. Both the name and the
  selector accept a string or an enum case: a string-backed enum contributes its value, any
  other enum its case name. Anything a profile does not set
  falls back to the cluster setting. The profile covers all sixteen driver settings:
  consistency, serial consistency, request timeout, retry policy, round-robin and
  datacenter-aware load balancing, token-aware routing and replica shuffling, latency-aware
  routing and its five tuning values, constant and disabled speculative execution, and
  host and datacenter allow and deny lists.
- New `Cluster\Builder` methods for every setting below, so an application can set them in code
  and not only in `php.ini`: `withRackAwareLoadBalancingPolicy()`, `withApplicationName()`,
  `withApplicationVersion()`, `withExponentialReconnect()`,
  `withConstantSpeculativeExecutionPolicy()`, `withNoSpeculativeExecutionPolicy()`,
  `withCoalesceDelay()` and `withNewRequestRatio()`.
- ScyllaDB rack-aware load balancing through `cassandra.local_rack` and `cassandra.local_dc`.
  A non-empty local rack tries live nodes in that rack first, then the local datacenter, then
  remote ones, which keeps traffic inside one cloud availability zone. This policy exists only
  in the ScyllaDB C/C++ driver.
- `cassandra.application_name` and `cassandra.application_version`. The server records both in
  `system.clients.client_options`, so an operator can see which application and version opens
  each connection.
- `cassandra.reconnect_policy` (`constant` or `exponential`) with `cassandra.reconnect_max_interval`.
  Exponential backoff adds jitter, so a whole worker pool no longer retries a recovering node in
  lockstep. The default stays `constant`, so existing behaviour is unchanged.
- `cassandra.speculative_execution_delay` and `cassandra.speculative_execution_max`, off by
  default. Only enable these for idempotent statements.
- `cassandra.coalesce_delay` and `cassandra.new_request_ratio` for IO event loop tuning.
- Fourteen more cluster directives, each defaulting to the C driver's own value so nothing
  changes until an operator sets one: `cassandra.local_address`,
  `cassandra.connection_idle_timeout`, `cassandra.max_schema_wait_time`,
  `cassandra.resolve_timeout`, `cassandra.monitor_reporting_interval`,
  `cassandra.queue_size_io`, `cassandra.prepare_on_all_hosts`,
  `cassandra.prepare_on_up_or_add_host`, `cassandra.shuffle_replicas`,
  `cassandra.no_compact`, `cassandra.beta_protocol`, `cassandra.tracing_consistency`,
  `cassandra.tracing_max_wait_time` and `cassandra.tracing_retry_wait_time`.
- A bad directive value is ignored in favour of the documented default, with one `E_WARNING` at
  startup naming the value that was dropped. `ini_get()` and `phpinfo()` report that default, so
  what they show is always what the driver uses.
- New guide page: [php.ini configuration](website/guide/configuration.md).
- Experimental support for [scylladb/cpp-rs-driver](https://github.com/scylladb/cpp-rs-driver)
  as a third backend, selectable at build time via the new `PHP_DRIVER_BACKEND` CMake
  cache variable (`cassandra | scylla-cpp | scylla-rust`). Install the Rust-backed
  driver with `scripts/compile-cpp-rs-driver.sh --prefix <path>`, then configure with
  `cmake -DPHP_DRIVER_BACKEND=scylla-rust` (or use a `*ScyllaRust` preset).
- New CMake presets `*PHP*<ver><ts>ScyllaRust` covering the third backend across PHP
  versions and build types.

### Dependencies

- The Cassandra backend now builds from `github.com/apache/cassandra-cpp-driver`, was
  `github.com/datastax/cpp-driver`. Both point at the same commits, but Apache is where the
  driver now lives, and the local build script already used it.
- All three drivers now track their default branch: `trunk` for
  `apache/cassandra-cpp-driver`, `master` for `scylladb/cpp-driver` and
  `scylladb/cpp-rs-driver`. `cpp-rs-driver` was pinned to `v1.0.0`; its `master` is currently
  the same commit as `v1.1.1`, which is the release series that adds ScyllaDB tablet support.
  Tablet keyspace information comes from `system_schema.scylla_keyspaces`, tablets on
  materialized views are fixed, and the driver backs off from advanced shard awareness on a
  shard mismatch instead of retrying pointlessly behind NAT.
- Tags on the two C++ drivers are stale, which is why the default branch is the right target:
  `scylladb/cpp-driver` last tagged `2.16.2b` in March 2022, and `apache/cassandra-cpp-driver`
  last tagged `2.17.1` in October 2023. Both branches are still active.
- `libuv` moves from `v1.50.0` to `v1.52.1` in `scripts/compile-libuv.sh`. libuv tags real
  releases, so it stays pinned rather than tracking a branch.

### Changed

- Casting a `Cluster\Builder` to an array now gives a `Cassandra\ProtocolVersion` case for the
  `protocolVersion` key, was an integer. A value that no case names stays an integer.
- The `scylla-cpp` backend now builds the vendored driver in `third-party/cpp-driver` and links
  it into `cassandra.so`. Upstream no longer maintains the ScyllaDB cpp-driver, so it ships with
  the extension and gets fixed here. A checkout builds with no C/C++ driver installed. To link a
  driver installed on the system, pass `-DPHP_SCYLLADB_USE_SYSTEM_DRIVER=ON`. The `cassandra` and
  `scylla-rust` backends are unchanged and still come from the system.

### Fixed

- A dropped session could abort the process with `pure virtual method called`. `cass_session_free()`
  returned from its internal close and started the destructor while a driver event-loop thread still
  dispatched a host-down event, so the call landed on a listener whose derived part was gone. It hit
  the extension whenever PHP released a non-persistent session soon after a failed query, and it
  failed CI runs at random points. Backported from `apache/cassandra-cpp-driver` (CASSCPP-3).
- Nine more fixes taken from `apache/cassandra-cpp-driver`, which kept getting them after the
  ScyllaDB fork stopped tracking it. The notable ones: a trusted certificate BIO with more than one
  CA now loads every certificate instead of only the first, a race in `monotonic_timestamp()` is
  gone, and `cass_cluster_set_load_balance_dc_aware_n()` accepts a NULL or empty local data center.
- `phpversion('cassandra')` and `Cassandra::VERSION` report the released version now. Every
  build reported `1.4.0-dev`, because the version came from a `project(VERSION ...)` line that
  no release ever raised, and the `-dev` suffix was unconditional. The release workflow passes
  the tag into the build, refuses to build when the tag and `project(VERSION ...)` disagree,
  and raises the patch version on the released branch afterwards.
- The `Cassandra\Future*` classes no longer accept direct construction. `new Cassandra\FutureRows()`
  built an object with no underlying `CassFuture`, and `get()` then crashed the process on a GCC
  build. The constructor is private now, so the call fails with an `Error`. Get a future from
  `Session::executeAsync()`, `prepareAsync()`, `closeAsync()` or `Cluster::connectAsync()` instead.
  ([#155](https://github.com/he4rt/scylladb-php-driver/issues/155))
- `cassandra.log = syslog` now writes through `syslog(3)`. It used to create a file named
  `syslog` in the process working directory. `cassandra.log = stderr` is also accepted now.
- `DefaultCluster::connect()` leaked the connect `CassFuture` when the session was not
  persistent. Only the error paths freed it. Confirmed with `leaks`: one 192-byte root
  leak per call, now zero.
- `Cluster\Builder::withConnectionHeartbeatInterval()` and `withTCPKeepalive()` passed a
  value 1000 times too large to the C driver. Both `cass_cluster_set_connection_heartbeat_interval()`
  and `cass_cluster_set_tcp_keepalive()` take seconds, but the builder stored milliseconds.
  `withConnectionHeartbeatInterval(30.0)` asked for a 30000-second heartbeat, which disabled
  heartbeats instead of sending one every 30 seconds. Both now match the documented seconds
  contract. The compiled default of 30 seconds was always correct, because it never went
  through the setter.

### Deprecated

- `USE_LIBCASSANDRA` CMake option is now an alias for `PHP_DRIVER_BACKEND=cassandra`
  and will be removed in a future release. Use `PHP_DRIVER_BACKEND` explicitly.

### Known limitations (scylla-rust backend)

- `Cassandra\Cluster\Builder::withMaxConnectionsPerHost()` is a no-op (removed upstream).
- Nine of the new `php.ini` directives have no effect, because `cpp-rs-driver` marks the
  matching setter `UNIMPLEMENTED` in its `api.rs` manifest: `cassandra.new_request_ratio`,
  `cassandra.queue_size_io`, `cassandra.monitor_reporting_interval`,
  `cassandra.prepare_on_all_hosts`, `cassandra.prepare_on_up_or_add_host`,
  `cassandra.no_compact`, `cassandra.tracing_consistency`, `cassandra.tracing_max_wait_time`
  and `cassandra.tracing_retry_wait_time`. Each keeps its driver default there. Every other
  directive, including execution profiles and rack-aware routing, works on all three backends.
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
