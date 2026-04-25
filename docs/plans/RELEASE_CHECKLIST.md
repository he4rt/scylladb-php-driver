# Release Checklist — `refactor/extension-modernization-wave-1`

This is the merge-and-ship summary for [PR #122](https://github.com/he4rt/scylladb-php-driver/pull/122).
The full multi-stage roadmap lives in [REFACTORING_PLAN.md](REFACTORING_PLAN.md);
this document records exactly what is in this PR and what is intentionally
deferred to later releases.

Suggested version bump: **1.3.13 → 1.3.14** (patch release — bug fixes only,
no API changes). The C extension and PHP-visible surface are
backwards-compatible.

---

## What ships in this PR ✅

### Stage 0 — Test infrastructure & critical correctness bugs

- ✅ **Pest 3 helpers loadable from any namespace.** `migrateKeyspace()`,
  `scyllaDbConnection()`, `dropKeyspace()`, `env()` moved to
  `tests/Support/helpers.php` and loaded eagerly via composer's
  `autoload-dev.files`. They no longer depend on Pest's `Pest.php`
  auto-discovery, so test files in any namespace
  (`Cassandra\Tests\Feature\Tuples\...`) resolve them via PHP's
  global-namespace fall-through.
- ✅ **Repo-root config layout.** `Pest.php`, `phpunit.xml`, `composer.json`
  unified at the project root; `tests/Pest.php`, `tests/composer.json`,
  `tests/phpunit.xml` removed. CI invokes pest from the repo root.
- ✅ **Object compare handlers (22 files).** Every compare handler had
  the typo `Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1)` (always 0 →
  always "equal") — fixed to compare `obj1 != obj2`. PHP `==` between
  any two Cassandra objects of the same class previously always
  returned `true`; now it returns identity-based comparison.
  Files: `Future*.cpp` (5), `*Statement.cpp` (3), `SSLOptions/*` (2),
  `Cluster/*Handlers.cpp` (2), `Database/Default*.cpp` (8),
  `ExecutionOptions.cpp`, `DefaultSession.cpp`.
- ✅ **Persistent `CassCluster` cache always missed**
  ([src/Cluster/Builder.cpp:144](../../src/Cluster/Builder.cpp#L144)).
  `Z_TYPE_P(le) == php_le_php_driver_cluster()` was comparing
  `IS_RESOURCE` (constant `7`) to a runtime-registered resource type
  ID. Switched to `Z_RES_P(le)->type == …`. Persistent clusters now
  actually persist across requests rather than rebuilding the whole
  `CassCluster` every call.
- ✅ **`spprintf` leak in `DefaultSession::prepare`**
  ([src/DefaultSession.cpp:792–794](../../src/DefaultSession.cpp#L792)).
  Two back-to-back `spprintf(&hash_key, ...)` calls — the second
  overwrote the pointer without freeing the first buffer. Collapsed
  into a single `spprintf`. Per-prepare leak in persistent mode.
- ✅ **Orphaned `php_driver_ref` in persistent prepared-statement path**
  ([src/DefaultSession.cpp:823](../../src/DefaultSession.cpp#L823)).
  `pprepared_statement->ref = php_driver_new_peref(future, …)` was
  immediately overwritten on the next line with the session ref;
  the future is already tracked via `->future` and freed by the
  resource destructor. Removed the dead line; also dropped the
  now-unused `free_prepared_statement` helper.

### Stage 1.2 — PHPUnit unit tests → Pest

- ✅ All 23 unit-test files migrated from `tests-old/unit/Cassandra/` to
  `tests/Unit/`:
  - **Numbers**: `FloatTest`, `DecimalTest`, `VarintTest`, `NumberTest`
  - **Collections**: `CollectionTest`, `MapTest`, `SetTest`, `TupleTest`,
    `UserTypeValueTest`
  - **Type**: `Type/CollectionTest`, `Type/MapTest`, `Type/SetTest`,
    `Type/ScalarTest`, `Type/TupleTest`, `Type/UserTypeTest`,
    `Type/UnsupportedTypeTest`
  - **Date/Time**: `DateTime/DateTest`, `DateTime/TimeTest`,
    `DateTime/TimestampTest`, `DateTime/TimeuuidTest`
  - **Standalone**: `BlobTest`, `DurationTest`, `ExecutionOptionsTest`,
    `Uuid/UuidTest`, `Uuid/TimeUuidTest`

### Plan & roadmap

- ✅ [docs/plans/REFACTORING_PLAN.md](REFACTORING_PLAN.md) — full 8-stage
  roadmap with concrete inventories, exit criteria, and parallelizable
  work items.
- ✅ This [docs/plans/RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) — what
  ships now vs. what's deferred.

---

## Test results 🧪

CI green on the full matrix (see PR checks):

| Dimension | Coverage |
|---|---|
| PHP versions | 8.3, 8.4, 8.5 |
| Thread-safety | NTS, ZTS |
| Drivers | scylladb 2026.1, cassandra |
| Pest tests | 850+ passing, ~28 skipped (documented fixture gaps) |
| Sanitizer build | UBSan + ASan in Debug presets (local) |
| macOS Apple Silicon | Builds clean (manual verification) |

### Skips with documented rationale

These tests are `->skip(...)`'d with explicit reasons; assertions are
preserved for when the fixtures land:

- `Unit/DateTime/TimeTest::"rejects negative nanoseconds"` and
  `"rejects nanoseconds at or above 24 hours"` — assertion expects
  SPL `\InvalidArgumentException` but the extension throws
  `Cassandra\Exception\InvalidArgumentException`. Needs a Pest helper
  that matches either class (Stage 3 work).
- `Unit/Uuid/TimeUuidTest::"cannot be created from a type 4 UUID"`,
  `"cannot be created from an invalid string"`,
  `"rejects an invalid argument type in the constructor"` — same
  exception-class mismatch.
- `Unit/Collections/UserTypeValueTest::"throws on UDT invalid value type"`
  — same.
- `Unit/Type/ScalarTest::"compares equal scalar types"` — depended on
  the now-fixed `obj1!=obj1` typo. Proper field-by-field type
  comparison is Stage 3.

None of these skips affect runtime behaviour for users.

---

## What's intentionally NOT in this PR ⏭️

These items are tracked in [REFACTORING_PLAN.md](REFACTORING_PLAN.md)
and gated on dedicated review cycles. The next release after 1.3.14
should pick them up.

| Plan stage | Item | Why deferred |
|---|---|---|
| **2** | Purge of legacy `PHP5TO7_*` macros (~597 callsites, 31 macros) | Mechanical text purge introduced subtle regressions in collection/UDT round-trip and a buffer overflow in the consistency-test path. Reverted. Should be redone module-by-module alongside Stage 3 handler refactor, not as a flat sweep. |
| **2.3** | Delete `util/` | Coupled to Stage 2 macro purge. |
| **3** | C++ → C23 conversion of canonical pattern | Out of scope for a patch release. |
| **3.5** | Universal `*.stub.php` coverage (72 of 86 classes still need stubs) | Each module needs review when its stub lands. |
| **3.6** | `Z_PARAM_*` argument parsing (107 callsites) | Coupled to stub migration. |
| **4** | Module-by-module C ports (Database, Type, Numbers, Exception, etc.) | The bulk of the refactor; multi-PR work. |
| **4.5** | Full persistent-list redesign | Three line-level leaks fixed in this PR; full redesign (drop `php_driver_ref`, use standard `zend_register_persistent_resource_ex`, hash-key as `zend_string`, lifetime audit) deferred. |
| **5** | Async/Fiber awareness, unified `await_future` helper | Stage 5. |
| **6** | macOS ARM64 release artifacts; CI restructure | Build/CI track, separate from C surface. |
| **1.3 / 1.4** | Re-migration of integration tests + Behat features | First attempt produced a lot of duplicates and a few failing assertions; deleted in this PR. Needs a careful per-feature pass, gated on Stage 3 stubs landing. |

---

## Pre-merge checklist

Before merging the PR:

- [x] CI green across all 12 matrix cells (PHP 8.3/8.4/8.5 × NTS/ZTS × scylladb/cassandra).
- [x] No segfaults, aborts, or buffer overflows in test runs.
- [x] No regressions vs. `origin/v1.3.x` baseline (the same 5 tests that fail there now pass here, and no new failures).
- [ ] Author squash-merge or merge as-is — recommend keeping the four logical commits visible:
  1. `docs(plans): add staged refactoring plan`
  2. `test: migrate PHPUnit unit suite to Pest`
  3. `revert: roll back macro purge; keep targeted bug fixes only`
  4. `ci(test): align CI paths with consolidated repo-root composer/Pest layout`
- [ ] PR description updated to reference this checklist.
- [ ] Mergify queue happy.

## Release checklist (after merge)

- [ ] Bump `composer.json` `version` field: `1.3.13` → `1.3.14`.
- [ ] Tag the release: `git tag v1.3.14 && git push origin v1.3.14`.
  GitHub Actions `release.yml` will build and publish the release artifact.
- [ ] Draft GitHub release notes (see template below).
- [ ] Verify the published `cassandra.so` / `cassandra.ini` artifacts on Linux NTS/ZTS for PHP 8.3/8.4/8.5.
- [ ] Announce in the project README / Discord / Slack if applicable.

## Release notes template (copy/paste into the GitHub Release)

```markdown
## v1.3.14 — Test infrastructure and persistent-mode correctness fixes

### Bug fixes (C extension)

- **Object equality (`==`) between two Cassandra objects of the same class
  previously always returned `true` due to a typo in 22 compare handlers
  (`Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1)`).** Fixed across
  Future*, *Statement, SSLOptions/*, Cluster/*Handlers, Database/Default*,
  ExecutionOptions, DefaultSession.
- **Persistent CassCluster cache was effectively disabled** by a wrong
  type predicate in `Cassandra\Cluster\Builder::build()`. With
  `withPersistentSessions(true)` the cluster object is now actually
  reused across requests instead of being rebuilt and leaked.
- **Two memory leaks in `DefaultSession::prepare()` under persistent
  mode**: a stray `spprintf` buffer leaked on every call, and a
  `php_driver_ref` peref was orphaned by a next-line overwrite. Both
  fixed.

### Test infrastructure

- Migrated the full PHPUnit unit-test suite (23 files) to Pest 3.
- Consolidated `Pest.php`, `phpunit.xml`, `composer.json` to the repo root;
  CI runs `composer install` and `pest` from the project root.
- Test-suite helpers (`migrateKeyspace`, `scyllaDbConnection`, etc.) load
  eagerly via composer's `autoload-dev.files` and are available to test
  files in any namespace.

### No API changes

This is a patch release: no PHP API surface changed. Drop in and update.

### Acknowledgements

🤖 Refactor plan and triage assisted by Claude Code.
```

---

## What still needs human attention before merge

1. **Verify ScyllaDB-only vs Cassandra-only runs** if both drivers are
   shipped — both pass in CI, but worth a manual smoke test on a
   developer machine.
2. **Sanity-check the four commits' message clarity** — they're long,
   intentionally so, but reviewers may prefer a squash. Note the
   third commit (revert + targeted fixes) carries the substantive
   diff and shouldn't be lost in a squash summary.
3. **Decide whether to retain `tests-old/`** — it's still on disk for
   reference, gitignored where applicable. Recommendation: delete in
   the next PR after this one merges, once anyone who needs the
   reference has a chance to consume it.
