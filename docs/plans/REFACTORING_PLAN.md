# ScyllaDB PHP Driver — Refactoring & Test Migration Plan

Authoritative roadmap for finishing the C++ → C23 migration, consolidating tests on Pest, and hardening build/CI. Stages are ordered by dependency; later stages assume earlier ones are complete.

Status legend: `TODO` / `IN PROGRESS` / `DONE` / `BLOCKED`.

> **Current state (after PR #122 merges as v1.3.14):** Stage 0 (test infra + critical bug fixes) and Stage 1.2 (unit-test migration) are **DONE**. Pick up from the resume sheet immediately below; full stage detail follows further down.

---

## 🚧 Resume from here — Wave 2 backlog (post-v1.3.14)

This is the prioritised TODO for the next development wave. Each item links to its full stage detail later in the document. Treat it as the working checklist; tick boxes as the work lands.

### Round A — High-confidence, low-risk first (target: v1.4.0)

- [ ] **Re-migrate integration tests + Behat features carefully** (Stage 1.3 + 1.4). The first attempt produced duplicates and behavioural drift; redo each file end-to-end against a live ScyllaDB fixture, with random-suffix keyspaces and per-test isolation. Source: `tests-old/integration/Cassandra/*` (16 files) and `tests-old/features/*.feature` (13 files).
- [ ] **Add `tests/Support/CCM.php` lifecycle helper** so the multi-node tests skipped today (`SessionIntegrationTest` cluster-down case, `ConsistencyLevelFeatureTest`, `RetryPolicy*` under-replication) can actually run in CI.
- [ ] **Bring up SSL-configured ScyllaDB fixture in docker-compose**, enabling the SSL feature tests currently `->skip()`'d.
- [ ] **Cassandra-flavour-only runner** for the 2 UDF/UDA scenarios (Scylla doesn't support them).
- [ ] **Fix the unit-test exception-class skips** (Stage 0.2 follow-ups). The skipped `Time`, `Timeuuid`, `UserTypeValue` tests assert `\InvalidArgumentException` but the extension throws `Cassandra\Exception\InvalidArgumentException`. Either accept both or change the test assertions. Affected: `tests/Unit/DateTime/TimeTest.php`, `tests/Unit/Uuid/TimeUuidTest.php`, `tests/Unit/Collections/UserTypeValueTest.php`.
- [ ] **Delete `tests-old/`** once everything above lands.

### Round B — Persistent-list redesign (target: v1.4.0 or v1.5.0)

Stage 4.5 in the full plan. Three line-level fixes shipped in v1.3.14; the structural redesign is what's left.

- [ ] Switch the three resource types (cluster, session, prepared statement) to the standard `zend_register_persistent_resource[_ex]` API instead of hand-crafted `pecalloc` + `ZVAL_NEW_PERSISTENT_RES` + manual `EG(persistent_list)` writes.
- [ ] Replace hash-key `char *` + `size_t` with `zend_string *` (interned where possible).
- [ ] Audit `session->hash_key` aliasing of `cluster->hash_key` for lifetime correctness when crossing the persistent allocator boundary.
- [ ] Drop `php_driver_ref` indirection — resources have native refcounting.
- [ ] Pest test for: cache hit, cache miss, broken-future invalidation, keyspace-switch reuse, prepared-statement cache hit/miss, persistent counts (`PHP_DRIVER_G(persistent_*)`) consistent across requests.
- [ ] Valgrind/ASan run: zero leaks across N=1000 prepare+execute cycles in persistent mode.

### Round C — Pattern foundation + PHP 8.3 modernisation (target: v1.5.0)

Stage 3 in the full plan. Bigger commitment; gates Stage 4.

- [ ] Bump composer.json `require.php` floor to `>= 8.3`. Drop conditional `#if PHP_VERSION_ID < …` for 8.1/8.2.
- [ ] Complete `src/Cluster` as the C23 reference: zero C++ headers, `.cpp → .c` rename once `ZendCPP::` is excised.
- [ ] Build the shared "type module" macro header for the 6 numeric modules' boilerplate (Stage 3.2).
- [ ] Document the canonical pattern in `docs/MODULE_PATTERN.md`; verify the `/new-module` skill produces matching output.
- [ ] CI quality gates: clang-tidy + cppcheck against the migrated modules; ASan in CI (currently only local Debug presets).
- [ ] Decide on enums (`Cassandra\Consistency`, `BatchType`, etc.) and `final readonly` value classes; ship the stub changes once the module is converted (Stage 3.7).
- [ ] Switch `\DateTime` returns to `\DateTimeImmutable` in `Date`, `Timestamp` stubs.
- [ ] `#[\SensitiveParameter]` on `Builder::withCredentials($username, #[\SensitiveParameter] string $password)`.
- [ ] Add `declare(strict_types=1);` to all 14 existing stubs (some already have it; audit).

### Round D — Universal stub coverage (target: v1.5.0 or v1.6.0)

Stage 3.5 in the full plan. **86 classes total, 14 done, 72 to go.** Order:

- [ ] Round A foundational (12 classes): `Cassandra` facade, `Value`, `Numeric`, `UuidInterface`, `Uuid`, `Inet`, `Blob`, `Duration`, `Bigint`, `Smallint`, `Tinyint`, `Float`, `Decimal`, `Varint`, `Custom`.
- [ ] Round B Type system (8 classes): `Type`, `Type\{Scalar,Collection,Set,Map,Tuple,UserType,Custom}`.
- [ ] Round C Collections (5 classes): `Collection`, `Set`, `Map`, `Tuple`, `UserTypeValue`.
- [ ] Round D Database (14 classes): `Session`, `DefaultSession`, `Statement`, `SimpleStatement`, `PreparedStatement`, `BatchStatement`, `ExecutionOptions`, `Future`, `FutureClose`, `FuturePreparedStatement`, `FutureRows`, `FutureSession`, `FutureValue`, `Rows`.
- [ ] Round E Schema metadata (14 classes): `Schema`, `DefaultSchema`, `Keyspace`, `DefaultKeyspace`, `Table`, `DefaultTable`, `Column`, `DefaultColumn`, `Index`, `DefaultIndex`, `MaterializedView`, `DefaultMaterializedView`, `Function`, `DefaultFunction`, `Aggregate`, `DefaultAggregate`.
- [ ] Round F TimestampGenerator (3 classes).
- [ ] Round G Exceptions (23 classes) — last; mostly mechanical.
- [ ] `make stubs` regeneration target + CI guard against hand-edited `_arginfo.h` drift.
- [ ] Reflection-vs-stub Pest test asserting every Cassandra class's method/property/constant set matches its stub.

### Round E — Modern argument parsing (target: v1.5.0, parallel with Round D)

Stage 3.6 in the full plan. **107 `zend_parse_parameters` callsites across 37 files**; 7 files already on `Z_PARAM_*` (Cluster, DateTime/*, RetryPolicy/Logging, SSLOptions/Builder).

- [ ] Convert per-module alongside its stub PR. Format-string → macro cheat sheet in [Stage 3.6 below](#stage-36--modern-argument-parsing-z_param_).
- [ ] CI guard: grep-based check rejecting any new `zend_parse_parameters(` callsite.
- [ ] Pest tests verifying modern `TypeError`/`ArgumentCountError` messages (different from the legacy `Warning: …`).

### Round F — Macro purge + util/ removal (target: v1.6.0)

Stage 2 in the full plan. Previously attempted as a flat sweep (PR #122 wave-1) and reverted because it caused collection/UDT round-trip regressions and a buffer overflow. Redo *module-by-module alongside Round D's stub work*.

- [ ] Round 1 — pure renames (6 macros, 22 callsites): `PHP5TO7_SMART_STR_VAL/LEN`, `PHP_SCYLLADB_Z_IS_*`, `PHP5TO7_ADD_NEXT_INDEX_STRING`. Lowest risk; can ship standalone.
- [ ] Round 2 — hashtable iteration (8 macros, ~188 callsites): `PHP5TO7_ZEND_HASH_FOREACH_*`, `GET_CURRENT_*`. Medium risk — the `STR_KEY_VAL` `char*` ↔ `zend_string*` adaptation is the historical landmine. Must include round-trip tests for Collection/Map/Set/UDT in CI before merging.
- [ ] Round 3 — hashtable mutation (11 macros, ~161 callsites): `PHP5TO7_ZEND_HASH_FIND/EXISTS/UPDATE/ADD/DEL/INDEX_*`, `ADD_ASSOC_*`, `ZVAL_COPY`.
- [ ] Round 4 — zval lifecycle (1 macro, 58 callsites): `PHP5TO7_ZVAL_MAYBE_DESTROY`. Drop the `do { … } while (0)` wrapper since callsites are inlined code, not macro expansions.
- [ ] Round 5 — load-bearing (4 macros, ~168 callsites): `PHP5TO7_ZEND_OBJECT_GET/ECALLOC/INIT/INIT_EX`. Coupled to handler refactor; do per-module after the module's stub + Z_PARAM port.
- [ ] **Stage 2.3 — delete `util/`** (12 headers + 10 cpp files): inline thin wrappers into owning modules; replace `uthash.h` with `zend_hash`; replace `php_driver_ref` with native resource refcounting (depends on Round B).

### Round G — Module ports to C23 (target: v1.6.0–v2.0.0)

Stage 4 in the full plan. Each module is its own PR, gated on its stub + Z_PARAM port from Rounds D + E.

- [ ] 4c (warm-up): `RetryPolicy`, `DateTime`, `SSLOptions` — already partial; finish handler split, `.cpp → .c` rename.
- [ ] 4a: `src/Database` (largest legacy module — `DefaultSession.cpp` is ~1000 lines).
- [ ] 4b: `src/Type` + `src/Numbers` (use Round C's shared macro for the 6 numeric types).
- [ ] 4d: `src/Exception` (23 thin wrappers — last; mostly mechanical).

### Round H — Async + build/CI hardening (target: parallel throughout)

- [ ] Stage 5: unified `php_driver_await_future(future, timeout)` helper consolidating ~20 ad-hoc await sites with consistent error checking.
- [ ] Stage 5: document the synchronous-await constraint under PHP-FPM, or introduce optional `cass_future_set_callback` path. Decision needed before Stage 7.
- [ ] Stage 6: macOS ARM64 release artifacts (add `macos-14` runner to `release.yml`).
- [ ] Stage 6: commit (or simplify) `generate-presets.php`.
- [ ] Stage 6: pin cpp-driver version in `compile-cpp-driver.sh` (currently clones HEAD `--depth 1`).
- [ ] Stage 6: feature tests in CI gated on the docker-compose ScyllaDB fixture.

### Round I — Final cleanup (target: v2.0.0)

Stage 7 in the full plan. Only after all the above lands.

- [ ] Drop C++ from `cmake/TargetOptimizations.cmake`.
- [ ] Remove all `extern "C"` / `BEGIN_EXTERN_C` macros.
- [ ] Remove `ZendCPP` dependency.
- [ ] `/simplify` and `/security-review` end-to-end.
- [ ] Tag clean **v2.0.0** — extension is C-only.

---

## ✅ What landed in v1.3.14 (PR #122)

Cross-reference of which Stage items are already shipped, so future PRs don't redo them.

- **Stage 0.1 Pest fix** — `Pest.php` and helpers correctly placed; auto-discovery works.
- **Stage 0.2 critical correctness bugs** — compare-handler typo (×22 files), persistent CassCluster cache predicate, two persistent prepared-statement leaks. The 5th audit bug (`DefaultTable.cpp:726`) was within the ×22 sweep.
- **Stage 0.3 Pest helper hardening** — partial: helpers split into `tests/Support/helpers.php`, loaded eagerly via composer's `autoload-dev.files`. Error-throwing on `cqlsh` failure not yet re-enabled (see Round A).
- **Stage 1.2 unit-test migration** — all 23 PHPUnit unit tests → Pest 3.

For the full release notes and squash-merge guidance, see [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md).

---

## Stage 0 — Test Infrastructure & Critical Bug Fixes (PRIORITY)

Goal: get `pest` green again and fix audit-discovered correctness bugs. Without working tests no later stage is verifiable.

### 0.1 Fix `migrateKeyspace()` undefined function — PRIORITY [TODO]

**Root cause.** `tests/tests/Pest.php` defines `migrateKeyspace()`, `dropKeyspace()`, `scyllaDbConnection()` and the `uses(TestCase::class)->in('Feature')` binding. Pest auto-discovers `Pest.php` only at the *tests root* (the directory containing `phpunit.xml`). The current path is one level too deep, so the file is never loaded and every helper resolves as undefined.

**Fix.**
1. Move `tests/tests/Pest.php` → `tests/Pest.php`.
2. Delete the empty `tests/tests/` directory.
3. Verify `tests/composer.json` PSR-4 autoload still resolves (`Cassandra\Tests\` → `./`).
4. Re-enable the Feature suite in [tests/phpunit.xml](tests/phpunit.xml) (currently commented out) — gated on Stage 0.4 fixture availability.
5. Run `composer install && ./vendor/bin/pest --testsuite=Unit` and confirm zero "undefined function" errors.

### 0.2 Critical correctness bugs found in audit [TODO]

1. [src/Database/DefaultTable.cpp:726](../../src/Database/DefaultTable.cpp) — compare handler compares `Z_OBJ_HANDLE_P(obj1)` against itself; equality is broken. Fix and add a Pest unit test.
2. [src/DefaultSession.cpp:104,116,263,273](../../src/DefaultSession.cpp) — raw `free()` on `export_twos_complement()` output. Switch to `efree()` (or pin allocator contract on the producer side).
3. `FutureRows`, `FuturePreparedStatement`, `FutureSession` — add `cass_future_error_code()` checks before reading results. Today async failures produce silent garbage.
4. [src/DefaultSession.cpp:445–459](../../src/DefaultSession.cpp) — batch/statement lifecycle: inconsistent `cass_batch_free` paths; statement freed immediately after `cass_batch_add_statement`. Audit ownership.
5. `Map::get_properties` / `Tuple::get_properties` — double-refcount via `add_next_index_zval` plus manual addref. Pick one path.

### 0.3 Pest helper hardening [TODO]

- [tests/Pest.php](../../tests/Pest.php) `migrateKeyspace()` swallows `cqlsh` failures with a commented-out `throw`. Re-enable; silent failures mask schema bugs. Provide a `migrateKeyspaceOrSkip()` variant for tests that must still run when the cluster is unavailable.
- Centralize keyspace lifecycle in a `KeyspaceFixture` helper: create-on-`beforeEach`, drop-on-`afterEach`, randomized name to allow parallel runs.
- Add `tests/Support/Cluster.php` with a single `connect()` factory; remove duplicated env-parsing in `dropKeyspace`/`migrateKeyspace`/`scyllaDbConnection`.

### 0.4 Bring up ScyllaDB fixtures in CI [TODO]

- The repo already has docker compose for ScyllaDB 2026.1 (commit `77fae3de`). Wire it into `.github/workflows/test.yml` as a `services:` block or a `docker compose up -d` step.
- Run Feature suite only when the cluster is reachable; skip with a clear message otherwise (don't fail).
- Local target: `./scripts/run-scylladb.sh && ./vendor/bin/pest`.

**Exit criteria for Stage 0:** `pest` runs Unit + Feature suites, all five correctness bugs have regression tests, CI green on Linux.

---

## Stage 1 — Test Migration: PHPUnit + Behat → Pest (PRIORITY)

Goal: a single test framework. Behat features and PHPUnit `tests-old/unit` cases are duplicated, drifted, and unmaintained. Pest is the target.

### 1.1 Inventory (snapshot)

| Source | Count | Style | Target |
|---|---|---|---|
| `tests-old/unit/Cassandra/*.php` | 23 files | PHPUnit `extends TestCase` | `tests/Unit/**/*Test.php` (Pest) |
| `tests-old/integration/Cassandra/*.php` | 16 files | PHPUnit + live cluster | `tests/Feature/**/*Test.php` (Pest) |
| `tests-old/features/*.feature` + `bootstrap/FeatureContext.php` | 13 features | Behat Gherkin | `tests/Feature/**/*Test.php` (Pest, scenario-per-`it`) |

No `.phpt` files exist; the legacy unit tests are PHPUnit, not phpt.

### 1.2 Unit-test migration (PRIORITY) [TODO]

Per file in `tests-old/unit/Cassandra/`:

1. Create matching `tests/Unit/<Class>Test.php`.
2. Convert `public function testFoo()` → `it('foo', function () { ... })`.
3. Convert `$this->assertEquals($a, $b)` → `expect($b)->toEqual($a)`. Most assertions map 1:1; document the few that don't in `tests/Pest.php` via custom expectations.
4. Convert `setUp` → `beforeEach`, `tearDown` → `afterEach`, data providers → `->with([...])`.
5. Drop the `tests-old/unit/<file>` only after the new test passes.

Recommended order (smallest first to validate the recipe, then group by domain):
- Round 1 (warm-up): `UuidTest`, `TimeUuidTest`, `BlobTest`, `DateTest`, `TimeTest`, `DurationTest`.
- Round 2 (numeric — collapses with Stage 2 numeric macro work): `FloatTest`, `DecimalTest`, `VarintTest`, `NumberTest`.
- Round 3 (collections): `MapTest`, `SetTest`, `CollectionTest`, `TupleTest`, `UserTypeValueTest`.
- Round 4 (Type/* subdirectory): `Type/CollectionTest`, `Type/MapTest`, `Type/SetTest`, `Type/ScalarTest`, `Type/TupleTest`, `Type/UserTypeTest`, `Type/UnsupportedType`.
- Round 5: `ExecutionOptionsTest`.

### 1.3 Integration-test migration [TODO]

Per file in `tests-old/integration/Cassandra/`:

1. Move target is `tests/Feature/<Domain>/<Class>Test.php` matching existing layout (`Statements/`, `Collections/`, `Results/`, etc.).
2. Replace `Integration::setUp()` boilerplate with `beforeEach(fn () => migrateKeyspace(...))`.
3. Use the `KeyspaceFixture` helper from Stage 0.3 — every test gets an isolated keyspace.
4. Reuse existing Pest tests as templates (e.g. [tests/Feature/Statements/SimpleStatementsTest.php](../../tests/Feature/Statements/SimpleStatementsTest.php)).

Recommended order:
- `BasicIntegrationTest`, `SessionIntegrationTest`, `SimpleStatementIntegrationTest` (foundational).
- `ConsistencyIntegrationTest`, `RetryPolicyIntegrationTest`, `TimestampIntegrationTest` (policy/options).
- `Collection/Set/Map/Tuple/UserType IntegrationTest` (data types — overlap with existing `tests/Feature/Collections`).
- `DatatypeIntegrationTest(s)`, `PagingIntegrationTest`, `SchemaMetadataIntegrationTest`.

### 1.4 Behat → Pest migration [TODO]

Behat is opinionated, slow to bootstrap, and has a single `FeatureContext.php`. Map each scenario to a Pest `it()`:

| `tests-old/features/*` | Pest target |
|---|---|
| `consistency_level.feature` | `tests/Feature/Statements/ConsistencyLevelTest.php` |
| `datatypes.feature` | `tests/Feature/Collections/*` (extend existing) |
| `logging.feature` | `tests/Feature/LoggingTest.php` |
| `materialized_view_metadata.feature` | `tests/Feature/Schema/MaterializedViewTest.php` |
| `prepared_statements.feature` | `tests/Feature/Statements/PreparedStatementsTest.php` |
| `retry_polices.feature` | `tests/Feature/Statements/RetryPolicyTest.php` |
| `schema_metadata.feature` | `tests/Feature/Schema/SchemaMetadataTest.php` |
| `secondary_index_metadata.feature` | `tests/Feature/Schema/SecondaryIndexTest.php` |
| `ssl_encryption.feature` | `tests/Feature/SSL/SslEncryptionTest.php` |
| `sessions/persistent_sessions.feature` | `tests/Feature/Sessions/PersistentSessionTest.php` |
| `sessions/session_management.feature` | `tests/Feature/Sessions/SessionManagementTest.php` |
| `sessions/session_object.feature` | `tests/Feature/Sessions/SessionObjectTest.php` |
| `risky/function_and_aggregate_metadata.feature` | re-evaluate — currently disabled in [tests/Feature/FunctionAggregatesMetadataTest.php](../../tests/Feature/FunctionAggregatesMetadataTest.php). |

For each scenario:
1. Read the `.feature` Gherkin steps.
2. Locate the matching step definitions in `tests-old/features/bootstrap/FeatureContext.php`.
3. Inline the step bodies into one Pest `it()` block per scenario; don't preserve Gherkin abstractions.
4. Reuse `KeyspaceFixture`, `migrateKeyspace`, `scyllaDbConnection`.
5. Delete `.feature` + matching step definitions from `FeatureContext.php`.

### 1.5 Cleanup [TODO]

After each batch of migrations:
- `git rm` the corresponding files in `tests-old/`.
- When `tests-old/` is empty, remove the directory and any Behat-related `composer.json` entries (`behat/behat`, etc. if present).
- Update [README.md](../../README.md) test instructions.

**Exit criteria for Stage 1:** `tests-old/` deleted; all assertions live in `tests/`; `./vendor/bin/pest` is the single command.

---

## Stage 2 — Legacy Macro Purge & `util/` Removal (PRIORITY — first refactor pass)

Goal: delete migration-era cruft before any module-level porting. These macros and `util/` helpers are leftovers from the PHP 5 → 7 transition; they obscure intent, prevent C23 idioms (e.g. `nullptr`, `[[nodiscard]]`), and duplicate functionality already in modern Zend. Removing them shrinks every later stage's diff.

### 2.1 `PHP5TO7_*` macro removal [TODO]

**Scope.** ~468 occurrences across `src/`, `include/`, `util/`. Defined in [include/php_driver.h](../../include/php_driver.h). Each macro expands to a single modern Zend API call (or noop) — they exist only because PHP 5 had a different signature.

**Recipe (per macro group, mechanical):**

| Macro | Replacement |
|---|---|
| `PHP5TO7_ZEND_OBJECT_GET(t, o)` | `php_driver_<t>_fetch(o)` (the C23 inline fn from CLAUDE.md) |
| `PHP5TO7_ZEND_OBJECT_ECALLOC(t, ce)` | direct `ecalloc(1, sizeof(php_driver_<t>_t) + zend_object_properties_size(ce))` |
| `PHP5TO7_ZEND_OBJECT_INIT[_EX]` | `zend_object_std_init` + `object_properties_init` inline |
| `PHP5TO7_SMART_STR_VAL/LEN` | `ZSTR_VAL((ss).s)` / `ZSTR_LEN((ss).s)` (with explicit null-check at callsite) |
| `PHP5TO7_ADD_ASSOC_*` / `PHP5TO7_ADD_NEXT_INDEX_STRING` | the underlying `add_assoc_*` / `add_next_index_string` directly |
| `PHP5TO7_ZEND_HASH_FOREACH_*` | the bare `ZEND_HASH_FOREACH_*` macros |
| `PHP5TO7_ZEND_HASH_GET_CURRENT_DATA[_EX]` | `zend_hash_get_current_data[_ex]` with explicit `!= NULL` |
| `PHP5TO7_ZEND_HASH_FIND/EXISTS/UPDATE/ADD/DEL/INDEX_*` | bare `zend_hash_str_*` / `zend_hash_index_*` |
| `PHP5TO7_ZEND_HASH_ZVAL_COPY` | `zend_hash_copy(dst, src, (copy_ctor_func_t)zval_add_ref)` |
| `PHP5TO7_ZVAL_MAYBE_DESTROY` | `zval_ptr_dtor()` |
| `PHP_SCYLLADB_Z_IS_BOOL_P/TRUE_P/FALSE_P` | `Z_TYPE_P(zv) == IS_TRUE \|\| IS_FALSE` inline |

**Approach.**
1. Pick one macro, `grep -rn` every callsite, expand inline (often `sed`-able), delete the `#define`.
2. Build + Pest after each macro group; keep diffs reviewable.
3. Don't preserve the macro "for a transition period" — flip the switch in one PR per group.
4. Once a header has zero `PHP5TO7_` references, scrub the leftover `#define` block.

**Order (smallest blast radius first):**
- Round 1: `PHP5TO7_SMART_STR_*`, `PHP_SCYLLADB_Z_IS_*`, `PHP5TO7_ADD_ASSOC_*`, `PHP5TO7_ADD_NEXT_INDEX_*` (pure renames).
- Round 2: `PHP5TO7_ZEND_HASH_FOREACH_*`, `PHP5TO7_ZEND_HASH_GET_CURRENT_*`.
- Round 3: `PHP5TO7_ZEND_HASH_FIND/EXISTS/UPDATE/ADD/DEL/INDEX_*` + `ZVAL_COPY`.
- Round 4: `PHP5TO7_ZVAL_MAYBE_DESTROY` (audit each callsite for double-dtor).
- Round 5 (load-bearing — do last): `PHP5TO7_ZEND_OBJECT_GET/ECALLOC/INIT*` — these are coupled to handler refactor work and should be replaced with the canonical C23 fetch/init pattern from CLAUDE.md, not just an inline expansion.

### 2.2 `PHP_DRIVER_CORE_*` and namespace macros [TODO]

- `PHP_DRIVER_CORE_METHOD` / `PHP_DRIVER_CORE_ME` — tied to the `PHP_ME` table style which Stage 4 deletes. Drop together with stub migration of `php_driver.cpp` itself.
- `PHP_DRIVER_NAMESPACE_ZEND_ARG_OBJ_INFO` — replaced by stub-generated arginfo. Delete after the last manual `ZEND_BEGIN_ARG_INFO_EX` is gone.
- `PHP_DRIVER_G(v)` — keep (not legacy; this is the canonical TSRM/global accessor).

### 2.3 Delete `util/` [TODO]

**Scope.** 12 headers + 10 `.cpp` files: `bytes`, `collections`, `consistency`, `future`, `hash`, `inet`, `inline`, `math`, `ref`, `result`, `types`, `uthash`, `uuid_gen`.

**Strategy.** None of these warrant a shared utility layer; they're either:
- (a) thin wrappers around 1–2 `cass_*` calls — inline at the callsite or move into the owning module,
- (b) duplicates of Zend/PHP stdlib (`hash`, `inline`),
- (c) header-only macro grab-bags (`uthash.h` — replace with `zend_hash` if used, otherwise delete),
- (d) Cassandra-specific glue that belongs inside `src/Database` or `src/Type` (`bytes`, `collections`, `result`, `future`).

**Per-file plan:**

| File | Action |
|---|---|
| `util/bytes.{h,cpp}` | Move into `src/Type/Blob.c` and `src/Numbers/Varint.c` (only callers). |
| `util/collections.{h,cpp}` | Move into `src/Type/Collection.c` / `Set.c` / `Map.c`. |
| `util/consistency.h` | Inline the `php_driver_get_consistency()` switch into `src/Database/ExecutionOptions.c`. |
| `util/future.{h,cpp}` | Becomes the unified `php_driver_await_future()` helper from Stage 5.1; move to `src/Database/Future.c`. |
| `util/hash.{h,cpp}` | Audit — most likely replaceable with bare `zend_hash_*`. Delete file. |
| `util/inet.{h,cpp}` | Move into `src/Type/Inet.c` (single owner). |
| `util/inline.h` | Audit each inline; either move to the owning module's private header or delete. |
| `util/math.{h,cpp}` | Move into `src/Numbers/` (single domain). |
| `util/ref.{h,cpp}` | Audit refcounting helpers; replace with `zend_object_addref`/`zend_object_release` directly. Delete file. |
| `util/result.{h,cpp}` | Move into `src/Database/Rows.c`. |
| `util/types.{h,cpp}` | Move into `src/Type/Type.c`. |
| `util/uthash.h` | Verify any callers; replace with `zend_hash`/`HashTable`. Delete. |
| `util/uuid_gen.{h,cpp}` | Move into `src/Type/Uuid.c` and `src/Type/Timeuuid.c`. |
| `util/CMakeLists.txt`, `util/src/` | Delete the directories once empty. |

**Build wiring.** Remove `add_subdirectory(util)` and `target_link_libraries(... ext_scylladb::util ...)` from root [CMakeLists.txt](../../CMakeLists.txt); update each receiving module's `target_sources`.

**Exit criteria for Stage 2:**
- Zero `PHP5TO7_*` references in the repo (`grep` shows nothing).
- `util/` directory deleted.
- All Pest suites still pass.
- Diff is large but mechanical; each round is its own PR for reviewability.

**Why before Stage 3 (pattern foundation).** Every module port in Stage 4 would otherwise have to translate macro forms in addition to restructuring. Doing it once, repo-wide, halves the porting work.

---

## Stage 3 — Pattern & Tooling Foundation

Goal: make the canonical C23 module pattern reproducible and enforceable.

1. **Complete `src/Cluster` as the gold reference** [TODO] — finish stubs for remaining helper classes; remove mixed C++ usage; rename `.cpp` → `.c` once ZendCPP usage is excised.
2. **Shared "type module" macro header** [TODO] — header-only C macros for the 6 numeric modules' boilerplate (`new/free/gc/properties/compare/cast`, arithmetic ops). Reduces each numeric module to ~30 lines.
3. **CI quality gates** [TODO] — clang-tidy + cppcheck against `src/Cluster` first, extend per-module as they migrate. Add ASan job (currently only local Debug presets).
4. **Document the pattern** [TODO] — short `docs/MODULE_PATTERN.md` with concrete diff examples mirroring CLAUDE.md. Verify the `/new-module` skill produces output matching post-Stage-2 Cluster.
5. **`util/` audit** [TODO] — list utilities that must move to pure C before any C-only module compiles cleanly. ZendCPP usage (73 callsites) is the main blocker; tracked separately and removed entirely in Stage 2.3.

### PHP 8.3 + Zend Internals Modernization

The recent test infra change bumped tests to PHP 8.3+ but root [composer.json](../../composer.json) still says `"php": ">= 8.1"`. Align the floor to PHP 8.3 and adopt features that are now ABI-stable.

6. **Bump baseline to PHP 8.3** [TODO]
   - Update root `composer.json` `require.php` to `>= 8.3` (matches `tests/composer.json`).
   - Drop conditional `#if PHP_VERSION_ID < ...` branches for 8.1/8.2.
   - Update CI matrix to drop 8.1/8.2 if still present; standardize on 8.3 / 8.4 / 8.5.
   - Update the macOS/Linux compile-php scripts and CMakePresets accordingly.

7. **Adopt PHP 8.1+ language features in stubs** [TODO]
   - **Enums for `int $consistency`** — [src/Cluster/Builder.stub.php:11](../../src/Cluster/Builder.stub.php) takes `int`; should be a `Cassandra\Consistency` backed enum (`int`). Same for `SerialConsistency`, `BatchType`, `LogLevel`. Generates self-documenting type hints in IDEs.
   - **`readonly` value classes (8.2)** — `Bigint`, `Smallint`, `Tinyint`, `Float_`, `Decimal`, `Varint`, `Inet`, `Uuid`, `Timeuuid`, `Date`, `Time`, `Timestamp`, `Duration`, `Blob` are immutable value objects. Mark stub classes `final readonly class …`.
   - **Return `\DateTimeImmutable`, not `\DateTime`** — [src/DateTime/Date.stub.php:14](../../src/DateTime/Date.stub.php), `Timestamp.stub.php:16` return mutable `\DateTime`. Modern PHP convention is immutable; mutating a returned value silently breaks Cassandra-side semantics.
   - **`#[\SensitiveParameter]` (8.2)** on credentials in `Builder::withCredentials($username, #[\SensitiveParameter] string $password)` — keeps passwords out of stack traces.
   - **Add `declare(strict_types=1)` to all stubs** — currently inconsistent across 14 stubs.
   - **`#[\Deprecated]` attribute (8.4)** for any APIs already documented as deprecated; defer if still on 8.3.

8. **Generated class registration everywhere** [TODO]
   - 12+ files still use `INIT_CLASS_ENTRY` + `zend_register_internal_class()` + `zend_declare_property*` (e.g. [src/Database/Column.cpp](../../src/Database/Column.cpp), `Rows.cpp`, `Schema.cpp`, `MaterializedView.cpp`, `Aggregate.cpp`, `Table.cpp`, `DefaultSchema.cpp`, `DefaultIndex.cpp`, `DefaultColumn.cpp`, `DefaultSession.cpp`, etc.).
   - Replace all with stub-generated `register_class_<FQCN>()`. Properties move into the stub as typed properties.
   - 98 files still carry manual `ZEND_BEGIN_ARG_INFO_EX` / `PHP_ME` / `PHP_FE_END`. All gone after stub migration.

9. **Modern Zend string + hash APIs** [TODO]
   - **Interned/known strings**: zero use of `ZSTR_INIT_LITERAL` / `ZSTR_KNOWN` / `ZEND_STR_*`. Hot-path string keys (`"keyspace"`, `"hosts"`, `"port"`, `"consistency"`) should use known interned strings — avoids per-request allocation and hash recomputation.
   - **`zend_string` for hash keys**: prefer `zend_hash_find` (with pre-built `zend_string`) over `zend_hash_str_find` in tight loops.
   - **`smart_str` over `spprintf`** for accumulation paths in [src/DefaultSession.cpp](../../src/DefaultSession.cpp) (currently `spprintf` + `efree` pattern is fragile under exception throws).

10. **Modern object handler patterns** [TODO]
    - Use `zend_object_handlers.offset` + `XtOffsetOf` (already in CLAUDE.md canonical pattern) — extend everywhere; many legacy modules still use `zend_objects_store_get_object` style.
    - Implement `get_gc` correctly on every object that holds zvals — audit found several numeric-type modules with stub `get_gc` returning empty arrays even when they hold sub-objects.
    - Use `zend_std_get_properties_for(ZEND_PROP_PURPOSE_DEBUG)` to differentiate debug-only vs serialized properties — prevents leaking internal state via `var_dump`.
    - **`zend_call_method`-style → `zend_call_known_function`** where the function is known at compile time (faster, skips name resolution).

11. **Exception construction** [TODO]
    - Switch `zend_throw_exception(ce, "msg %s", arg)` callsites to `zend_throw_exception_ex(ce, code, "msg %s", arg)` for consistency with the rest of the codebase, and include error codes in the `code` field — currently always 0.
    - Make exception classes registered via stubs with proper hierarchy (`Cassandra\Exception\TimeoutException extends \RuntimeException` etc.).

12. **Argument parsing helpers** [TODO]
    - Use `Z_PARAM_OBJ_OF_CLASS` instead of `Z_PARAM_OBJECT` + manual `instanceof` check (saves ~10 LOC per method).
    - Use `Z_PARAM_STR` over `Z_PARAM_STRING` when downstream wants `zend_string *` (avoids double-allocation).
    - Use `Z_PARAM_ARRAY_HT` when only the `HashTable*` is needed.

13. **Observer/fiber awareness** [TODO]
    - PHP 8.1+ Fibers can suspend mid-`cass_future_wait_timed`. Currently the driver blocks the entire thread. Document the constraint, and at minimum register a `ZEND_OBSERVER_FCALL_BEGIN_HANDLER` marker so profilers (Tideways, Datadog, Blackfire) attribute Cassandra wait time correctly.

**Exit criteria:** Cluster is 100% C23, zero C++ headers, baseline PHP 8.3+, all 14 existing stubs use `strict_types` + `readonly`/enums where applicable; one numeric module ported via the shared macro; CI fails on clang-tidy regressions.

---

## Stage 3.5 — Universal Stub Coverage (PRIORITY — blocks every later stage)

Goal: every PHP-visible class in the extension has a `*.stub.php` and matching auto-generated `*_arginfo.h`, exactly like `src/Cluster/`. Stubs become the **single source of truth** for the PHP surface; manual `INIT_CLASS_ENTRY` / `ZEND_BEGIN_ARG_INFO_EX` / `PHP_ME` tables are deleted as each stub lands.

### Why this stage exists

- 14 of 86 classes have stubs today (~16% coverage).
- Stubs unlock typed properties, `readonly`, enums, attributes — most Stage 3 PHP 8.3 modernization items can't land without them.
- Each `.cpp → .c` rename in Stage 4 is gated on the module already having stubs (otherwise the manual arginfo doesn't compile under C23 anyway).
- Stubs document the public API for reflection, IDE autocomplete, and Phpstan/Psalm without `.phpstub` hacks.

### Workflow per class (mechanical)

1. Create `src/<Module>/<Class>.stub.php` next to the implementation. Use [src/Cluster/Builder.stub.php](../../src/Cluster/Builder.stub.php) as the template.
2. Required header in every stub:
   ```php
   <?php
   /** @generate-class-entries */
   declare(strict_types=1);
   namespace Cassandra\<Module>;
   ```
3. Generate arginfo from a built PHP source tree:
   ```bash
   php php/8.3-debug-nts/src/build/gen_stub.php src/<Module>/<Class>.stub.php
   ```
4. Replace in implementation file:
   - `INIT_CLASS_ENTRY(...)` + `zend_register_internal_class(...)` → `register_class_Cassandra_<Module>_<Class>()`
   - `ZEND_BEGIN_ARG_INFO_EX(...)` blocks → delete (now in `_arginfo.h`)
   - `PHP_ME(...)` table → delete (generated method table comes from arginfo)
   - `zend_declare_property_*` → typed properties in stub
5. Commit `*.stub.php` + `*_arginfo.h` together. Never edit `_arginfo.h` by hand.
6. Run `/php-ext-review src/<Module>/` skill on the result.

### Inventory — classes still needing stubs

**Already done (14):** `Cluster\Builder`, `Cluster\ClusterInterface`, `Cluster\DefaultCluster`, `DateTime\Date`, `DateTime\Time`, `DateTime\Timestamp`, `DateTime\Timeuuid`, `RetryPolicy\{DefaultPolicy, DowngradingConsistency, Fallthrough, Logging, RetryPolicy}`, `SSLOptions\{Builder, SSLOptions}`.

**TODO — group by domain. Tackle in this order:**

#### Round A — Top-level interfaces and value types (~12 classes) [TODO]
Foundational; every other module depends on them.
- `Cassandra` (the static facade with `cluster()`, `ssl()`, etc. — top-level entry)
- `Value` (interface)
- `Numeric` (abstract)
- `UuidInterface` (interface)
- `Uuid`
- `Inet`
- `Blob`
- `Duration`
- `Bigint`, `Smallint`, `Tinyint`, `Float`, `Decimal`, `Varint` ← coordinate with Stage 3.2 numeric macro work and Stage 3.7 `readonly`
- `Custom`

#### Round B — Type system (`src/Type/`, 8 classes) [TODO]
- `Type` (abstract base)
- `Type\Scalar`
- `Type\Collection`
- `Type\Set`
- `Type\Map`
- `Type\Tuple`
- `Type\UserType`
- `Type\Custom`

#### Round C — Collections (5 classes) [TODO]
- `Collection`, `Set`, `Map`, `Tuple`, `UserTypeValue`

#### Round D — Database surface (`src/Database/`, 14 classes) [TODO]
Largest single block; aligns with Stage 4a port.
- `Session` (interface), `DefaultSession`
- `Statement` (abstract), `SimpleStatement`, `PreparedStatement`, `BatchStatement`
- `ExecutionOptions`
- `Future` (interface), `FutureClose`, `FuturePreparedStatement`, `FutureRows`, `FutureSession`, `FutureValue`
- `Rows`

#### Round E — Schema metadata (`src/Database/Schema*`, 14 classes) [TODO]
- `Schema` (interface), `DefaultSchema`
- `Keyspace` (interface), `DefaultKeyspace`
- `Table` (abstract), `DefaultTable`
- `Column` (abstract), `DefaultColumn`
- `Index` (abstract), `DefaultIndex`
- `MaterializedView` (abstract), `DefaultMaterializedView`
- `Function` (abstract), `DefaultFunction`
- `Aggregate` (abstract), `DefaultAggregate`

#### Round F — TimestampGenerator (3 classes) [TODO]
- `TimestampGenerator` (interface), `TimestampGenerator\Monotonic`, `TimestampGenerator\ServerSide`

#### Round G — Exceptions (`src/Exception/`, 23 classes) [TODO]
Largest count but mechanical; can be one consolidated stub file `src/Exception/exceptions.stub.php` per gen_stub multi-class support, or one stub per class for clarity.
- `Exception` (base, currently extends `\Exception`)
- `Exception\{AlreadyExistsException, AuthenticationException, ConfigurationException, DivideByZeroException, DomainException, ExecutionException, InvalidArgumentException, InvalidQueryException, InvalidSyntaxException, IsBootstrappingException, LogicException, OverloadedException, ProtocolException, RangeException, ReadTimeoutException, RuntimeException, ServerException, TimeoutException, TruncateException, UnauthorizedException, UnavailableException, UnpreparedException, ValidationException, WriteTimeoutException}`

Per Stage 3.11: each should extend the matching SPL exception (`\RuntimeException`, `\LogicException`, etc.) where semantically appropriate, not just `Cassandra\Exception`.

### Cross-cutting concerns

- **Constants**: many static class constants (`Cassandra::CONSISTENCY_*`, `Cassandra::BATCH_LOGGED`, etc.) currently registered via `zend_declare_class_constant_*`. Stubs support `const FOO = 1;` syntax — move them.
- **Properties**: typed properties in stubs (`public readonly int $port;`) replace `zend_declare_property_*`. Audit existing `get_properties` handlers — many do redundant work that the typed-property machinery handles automatically.
- **Tooling**: add a `make stubs` (or `composer stubs`) target that regenerates every `_arginfo.h` from its stub in one pass; CI fails if regeneration produces a diff (catches manual edits).
- **Reflection sanity check**: after each round, run a small Pest test that uses `ReflectionClass` on every Cassandra class and asserts the method/property/constant set matches the stub. Drives out drift.

### Exit criteria

- `find src -name "*.stub.php" | wc -l` ≥ 86 (one per class, possibly fewer if multi-class stubs are used).
- Zero `INIT_CLASS_ENTRY(...)` callsites in `src/`.
- Zero `ZEND_BEGIN_ARG_INFO_EX` / `ZEND_END_ARG_INFO` in `src/`.
- Zero `PHP_ME(...)` / `PHP_ABSTRACT_ME(...)` / `PHP_FE_END` in `src/`.
- `make stubs` is green; CI enforces no drift.
- Reflection-vs-stub Pest test passes for every class.

---

## Stage 3.6 — Modern Argument Parsing (`Z_PARAM_*`)

Goal: every `ZEND_METHOD` / `ZEND_FUNCTION` parses arguments with the typed `ZEND_PARSE_PARAMETERS_START` / `Z_PARAM_*` macros, matching the Cluster reference. Delete the last `zend_parse_parameters("sl|z", ...)` callsite.

### Scope

- **107 `zend_parse_parameters()` callsites across 37 files.**
- **7 files already migrated** (use as templates): [src/Cluster/Builder.cpp](../../src/Cluster/Builder.cpp), [src/DateTime/Date.cpp](../../src/DateTime/Date.cpp), `Time.cpp`, `Timestamp.cpp`, `Timeuuid.cpp`, [src/RetryPolicy/Logging.cpp](../../src/RetryPolicy/Logging.cpp), [src/SSLOptions/Builder.cpp](../../src/SSLOptions/Builder.cpp).

### Why now

- `Z_PARAM_*` is type-checked at compile time; format-string typos in `zend_parse_parameters` only fail at runtime.
- The new macros emit better error messages (parameter name, expected type, got type), matching what user-space PHP code shows for built-in functions.
- Performance: macro-based parsing skips the format-string interpreter loop entirely.
- Coupling with stubs: stub-generated arginfo declares the *types*, but the implementation still has to parse them. Mismatches between `Stub.php` (`int $port`) and `zend_parse_parameters("s", ...)` are silent today — the new macros line up 1:1 with stub types.
- Required for typed-property and enum support added in Stage 3.7.

### Format string → macro cheat sheet

Based on the 107 callsites, the format characters in use are: `z` (50), `s` (17), `|` (optional separator, many), `*`/`+` (variadic), `l`, `h`, `O`, plus combinations.

| Old format char | Modern macro | Notes |
|---|---|---|
| `s` | `Z_PARAM_STRING(str, str_len)` | Returns `char*` + `size_t`. |
| `s` (when downstream wants `zend_string*`) | `Z_PARAM_STR(zstr)` | Avoids extra alloc. Prefer this. |
| `l` | `Z_PARAM_LONG(n)` | `zend_long`. |
| `d` | `Z_PARAM_DOUBLE(d)` | |
| `b` | `Z_PARAM_BOOL(b)` | |
| `z` | `Z_PARAM_ZVAL(zv)` | Use sparingly; prefer typed variants. |
| `a` | `Z_PARAM_ARRAY(zv)` | Whole zval. |
| `h` | `Z_PARAM_ARRAY_HT(ht)` | When only `HashTable*` is needed — no zval wrapper. |
| `o` | `Z_PARAM_OBJECT(zv)` | Any object. |
| `O` | `Z_PARAM_OBJECT_OF_CLASS(zv, ce)` | Type-checked at parse time, kills the manual `instanceof_function` follow-up. |
| `r` | `Z_PARAM_RESOURCE(zv)` | |
| `f` | `Z_PARAM_FUNC(fci, fcc)` | Callables. |
| `*` (variadic) | `Z_PARAM_VARIADIC('*', argv, argc)` | Final macro in the block. |
| `+` (variadic, ≥1) | `Z_PARAM_VARIADIC('+', argv, argc)` | |
| `\|` | `Z_PARAM_OPTIONAL` | Marker — everything after is optional. |
| `!` (nullable) | append `_OR_NULL` (e.g. `Z_PARAM_STR_OR_NULL`) | Or use `_EX` variants with `check_null=true`. |

### Migration recipe (per `ZEND_METHOD`)

Before:
```c
zend_string *keyspace;
zval *options = NULL;
if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|z",
                          &keyspace, &options) == FAILURE) {
    return;
}
```

After:
```c
zend_string *keyspace;
zval *options = NULL;
ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_STR(keyspace)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(options)
ZEND_PARSE_PARAMETERS_END();
```

Notes:
- The `(min, max)` pair in `ZEND_PARSE_PARAMETERS_START` must match the stub signature; mismatches are now diagnosable.
- On parse failure the macros `RETURN_THROWS()` automatically — no manual `return;` after the END macro.
- For tight loops where the args are already parsed in a parent frame, prefer `Z_PARAM_*_EX` variants that don't re-throw.

### Order of work

Tackle in the same order as Stage 3.5 stub rounds — each module's zpp migration belongs in the same PR as its stub introduction. They have to land together: deleting `PHP_ME` arginfo without modern parsing leaves the implementation looking like it accepts untyped args.

Cross-reference per file (highest count first):
- `src/DefaultSession.cpp` — heaviest user; coordinates with Stage 4a.
- `src/Database/Default*.cpp` — schema metadata block; coordinates with Stage 3.5 Round E.
- `src/Numbers/*.cpp` — 6 files, mostly identical patterns; absorb into the Stage 3.2 numeric macro so the zpp block is generated.
- `src/Type/*.cpp` — 5 files; Stage 3.5 Round B.
- `src/Future*.cpp` — 5 files; Stage 3.5 Round D.
- `src/{Collection,Set,Map,Tuple,UserTypeValue}.cpp` — Stage 3.5 Round C.
- `src/{Inet,Uuid,Blob,Duration}.cpp` — Stage 3.5 Round A.

### Quality gate

- After each round, `grep -rn "zend_parse_parameters\b" src --include="*.cpp" --include="*.c" | wc -l` decreases monotonically toward 0.
- CI gate (clang-tidy custom check or simple grep): fail the build if any `src/**/*.{c,cpp}` introduces a new `zend_parse_parameters(` callsite.
- Pest-level: add a Feature test that exercises each method with wrong types; verify the new macros produce `TypeError`/`ArgumentCountError` matching modern PHP conventions (different from the legacy `Warning: ...expects parameter ...`).

### Exit criteria

- Zero `zend_parse_parameters(` callsites in `src/`.
- All `ZEND_METHOD`/`ZEND_FUNCTION` bodies use `ZEND_PARSE_PARAMETERS_START/END`.
- CI guard rejects regressions.

---

## Stage 4 — Port High-Value Modules (parallelizable)

Each substage produces stubs + arginfo + `.c` + `*Handlers.c` + CMakeLists, with `/php-ext-review` and `/scylladb-review` passing clean.

### 4a. `src/Database` (largest legacy module) [TODO]

- Stubs for `DefaultSession`, `Statement`, `PreparedStatement`, `BatchStatement`, `ExecutionOptions`, `Rows`, `Schema`, `Keyspace`, `Table`, `Column`, `Index`, `MaterializedView`, `Function`, `Aggregate`.
- Replace all `zend_parse_parameters` with `ZEND_PARSE_PARAMETERS_START`.
- Split handlers into `*Handlers.c` per class.
- Verify recent persistent-session work (commit `1fbf124e`: timeout closure + `max_connections_per_host`) has Feature tests.

### 4b. `src/Type` + `src/Numbers` [TODO]

- Numbers: 6 numeric types collapse into ~6 thin `.c` files + shared header from Stage 3.2.
- Type: `Collection`, `Map`, `Set`, `Tuple`, `UserType`, `Custom`, `Scalar` — stubs + handler splits.

### 4c. `src/RetryPolicy`, `src/DateTime`, `src/SSLOptions` [TODO]

Already partial — finish handler split and `.cpp → .c` rename. Smallest effort; ship first within Stage 4 for momentum.

### 4d. `src/Exception` (25 thin wrappers) [TODO]

Mostly mechanical. Consider one consolidated stub file. Lowest priority.

---

## Stage 4.5 — Persistent Session/Cluster Redesign (PRIORITY — correctness)

The current persistent-list integration is the right *idea* (uses `EG(persistent_list)` + `ZVAL_NEW_PERSISTENT_RES` + a registered resource type with destructor) but its plumbing is buggy and fragile. Bugs already fixed in this branch as preconditions:

- ✅ [src/Cluster/Builder.cpp:144–151](../../src/Cluster/Builder.cpp) — `Z_TYPE_P(le) == php_le_php_driver_cluster()` was comparing `IS_RESOURCE` (constant `7`) to a registered resource type ID; the cached-cluster branch **never matched**, so every `connect()` rebuilt the entire `CassCluster`. Fixed to `Z_RES_P(le)->type == php_le_php_driver_cluster()`.
- ✅ [src/DefaultSession.cpp:792–794](../../src/DefaultSession.cpp) — back-to-back `spprintf(&hash_key, ...)` leaked the first buffer on every persistent prepare. Collapsed into one `spprintf`.
- ✅ [src/DefaultSession.cpp:823](../../src/DefaultSession.cpp) — `pprepared_statement->ref` was assigned a peref of the future then immediately overwritten on the next line with the session ref, leaking the peref struct. Future is already tracked via `->future`. Removed the line.

### Remaining redesign work [TODO]

1. **Use the standard `zend_register_persistent_resource[_ex]` path** instead of hand-crafted `pecalloc` + `ZVAL_NEW_PERSISTENT_RES` + manual `EG(persistent_list)` writes. The current pattern reimplements what the resource API already does, and the `php_driver_ref` wrapper duplicates resource refcounting. Three resource types are registered ([src/php_driver.cpp:126–173](../../src/php_driver.cpp)): cluster, session, prepared statement — all should switch to the standard helper.

2. **Hash-key construction**:
   - Move the hash-key format strings out of inline `spprintf` and into `static const char *` keys (or `zend_string` interned at module init).
   - Build keys with a single allocation (the dual-`spprintf` pattern was the leak source — eliminate it everywhere, not just where already fixed).
   - Pin a `zend_string *hash_key` field instead of `char *` + `size_t len` — gives interned-string equality and avoids strlen scans.

3. **Lifetime audit** — `session->hash_key` currently aliases `cluster->hash_key`, which lives in the persistent allocator (set via `spprintf` with no `pestrdup`/`pemalloc`-backed buffer). After Stage 2 macro purge, audit every assignment of `hash_key` and confirm:
   - Either it's a `zend_string *` with proper persistent flag,
   - Or it's `pestrdup`'d when crossing into a persistent struct,
   - Or it's a `static const char *` literal.

4. **Drop the `php_driver_ref` indirection**:
   - Resources already provide refcounting via `zend_list_addref` / `zend_list_delref`.
   - For the session/cluster pointers held inside non-persistent structs, use `zend_resource *` directly with `Z_ADDREF`/`zend_list_close`.
   - This kills `util/ref.{h,cpp}` (already slated for removal in Stage 2.3).

5. **Connection-failure invalidation** (recent commit `1fbf124e`) — currently the broken-future case `HASH_DEL`s the entry and frees `hash_key`. Verify the resource destructor runs cleanly when `cass_future_wait_timed` returned `FAILURE` *before* `cass_future_get_session` was ever called. Add a Pest test that:
   - Connects with a deliberately bad host,
   - Confirms the persistent_list entry is removed,
   - Reconnects with a good host and confirms a fresh `CassSession` is created (not a stale one cached behind the bad future).

6. **Concurrency note**: PHP-FPM workers each have their own `EG(persistent_list)`; cross-worker session sharing is *not* a thing and shouldn't be advertised as such. Document this; the term "persistent session" misleads users who expect process-shared connection pools.

### Exit criteria

- All three resource types registered via the standard PHP API; no more manual `ZVAL_NEW_PERSISTENT_RES` + `pecalloc` choreography in module code.
- `php_driver_ref` deleted (along with `util/ref.*`).
- Pest Feature tests exercise: cache hit, cache miss, broken-future invalidation, keyspace-switch reuse, prepared-statement cache hit/miss, persistent counts (`PHP_DRIVER_G(persistent_*)`) match expectations across requests.
- Valgrind/ASan run shows zero leaks across N=1000 prepare+execute cycles in persistent mode.

---

## Stage 5 — Async & Persistence Hardening

1. **Unified future helper** [TODO] — single `php_driver_await_future(future, timeout)` that funnels error-code checks, freeing, and exception throw. Replaces ~20 ad-hoc await sites.
2. **Persistent-session lifetime** [TODO] — revisit `hash_key` lifetime ([src/DefaultSession.cpp:848–850](../../src/DefaultSession.cpp)); allocate persistently if entry outlives request.
3. **Async dispatch decision** [TODO] — document the synchronous-await constraint in PHP-FPM workers, or introduce optional `cass_future_set_callback` path. Decision needed before Stage 7.

---

## Stage 6 — Build, CI, Distribution (runs in parallel with Stage 4/5)

1. **macOS ARM64 release artifacts** [TODO] — add `macos-14` runner to `release.yml`.
2. **Preset generator** [TODO] — `generate-presets.php` is referenced in CLAUDE.md but not in the repo. Either commit it or simplify the 60-preset matrix.
3. **CMake polish** [TODO] — broaden `CMAKE_SYSTEM_PROCESSOR` matching beyond exact `x86_64`; warn when `ENABLE_AVX*` is requested on ARM; audit other find-modules for the `*_FOUND` ordering pattern fixed in `2965282b`.
4. **Pin cpp-driver** [TODO] — `compile-cpp-driver.sh` clones HEAD `--depth 1`. Pin tag/commit; record in repo.
5. **Feature tests in CI** [TODO] — covered by Stage 0.4.

---

## Stage 7 — Final Cleanup

- Drop C++ from `cmake/TargetOptimizations.cmake` (no more dual `c_std_23` / `cxx_std_20`).
- Remove `extern "C"` blocks and `BEGIN_EXTERN_C` macros.
- Remove ZendCPP dependency.
- Run `/simplify` and `/security-review` end-to-end.
- Tag clean v1.4 release once everything is C-only.

---

## Execution Order

```
Stage 0 (test infra + critical bugs)  ─┐
                                        ├─ PRIORITY — must finish first
Stage 1 (Pest migration)               ─┘
        │
        ▼
Stage 2 (PHP5TO7 macro purge + util/ removal)   ◄── PRIORITY first refactor pass
        │
        ▼
Stage 3 (pattern foundation + PHP 8.3 + Zend modernization)
        │
        ▼
Stage 3.5 (universal stub coverage — all 86 classes get *.stub.php) ◄── PRIORITY
        │
        ▼
Stage 3.6 (modern Z_PARAM_* arg parsing — kills 107 zend_parse_parameters callsites)
        │
        ▼
Stage 4 ┬─ 4c (warm-up: easy wins — RetryPolicy/DateTime/SSLOptions)
        ├─ 4a (Database)
        ├─ 4b (Type/Numbers)        ──┐
        └─ 4d (Exception, last)       │
                                       ▼
                                    Stage 4.5 (persistent-list redesign) ──► Stage 5 ──► Stage 7

Stage 6 (build/CI/distribution) runs in parallel with Stages 4/5
```

---

## Notes

- Update this document as stages complete; prefer striking through and dating completed bullets over deletion until the stage is closed.
- Each PR should reference the stage and bullet it addresses (e.g. `Stage 1.2 round 2: migrate FloatTest to Pest`).
- The audit findings backing this plan live in the conversation transcript that produced this file; capture per-stage findings in their own PR descriptions, not here.
