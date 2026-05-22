# v1.3.x Branch — Issues, Risks & Improvement Plan

Snapshot of correctness, memory-safety, Zend-API compatibility and performance issues found on `v1.3.x` as of 2026-05-21. Each item carries a tier, a file:line citation and a suggested fix direction. Use this as a punch list — not every item is in scope for the current refactor, but everything here should be triaged before a release tag.

> **Verification pass (2026-05-21):** several findings from the first-pass agents turned out to be false positives — flagged inline with **[FALSE POSITIVE]** below. The truly broken items have been fixed and marked **[FIXED]**.

Tiers:

- **Critical** — leak, UAF, crash, or breaks the build on a supported PHP version.
- **High** — probable leak / wrong behaviour on the error path / deprecation that will bite within one PHP minor.
- **Medium** — best-practice violation, perf cost, or modernisation that improves maintainability.
- **Low** — opportunistic / forward-looking.

---

## 1. Memory safety & leaks

### 1.1 ~~Critical — `free_obj` handler defined but never registered~~ [FALSE POSITIVE]

The audit agent missed that [include/php_driver.h:66-74](include/php_driver.h:66) — the `PHP5TO7_ZEND_OBJECT_INIT_EX` macro used in every legacy `_new()` constructor — sets `handlers.offset` and `handlers.free_obj` on the global handlers struct during the first object creation. So even when the `define_*()` function doesn't explicitly assign `free_obj`, the resource IS freed. The "Pattern B" modules (DefaultCluster, BatchStatement, FutureSession, FutureValue) set it explicitly at MINIT; the "Pattern A" modules (everything else) set it lazily through the macro. Both work. Original false-flagged file list kept below for posterity:

| File | Symptom |
|---|---|
| [src/FutureClose.cpp:63](src/FutureClose.cpp:63), setup at [:89](src/FutureClose.cpp:89) | `cass_future_free` never called |
| [src/FutureRows.cpp:110](src/FutureRows.cpp:110), setup at [:148](src/FutureRows.cpp:148) | `cass_future_free` + result zval leaked |
| [src/FuturePreparedStatement.cpp:75](src/FuturePreparedStatement.cpp:75), setup at [:108](src/FuturePreparedStatement.cpp:108) | `cass_future_free` + prepared zval leaked |
| [src/DefaultSession.cpp:1001](src/DefaultSession.cpp:1001), setup at [:1045](src/DefaultSession.cpp:1045) | persistent session ref never decremented |
| [src/PreparedStatement.cpp:56](src/PreparedStatement.cpp:56), setup at [:88](src/PreparedStatement.cpp:88) | `cass_prepared_free` skipped |
| [src/SimpleStatement.cpp:95](src/SimpleStatement.cpp:95) | no `free_obj` at all — CQL string leaks |
| [src/Database/Rows.cpp:455](src/Database/Rows.cpp:455) | ref counting + result zval skipped |
| [src/Collection.cpp:425](src/Collection.cpp:425) | HashTable destroy skipped |
| [src/Set.cpp:422](src/Set.cpp:422) | HashTable destroy skipped |
| [src/Map.cpp:573](src/Map.cpp:573) | HashTable destroy skipped |
| [src/Tuple.cpp:408](src/Tuple.cpp:408) | HashTable destroy skipped |
| [src/UserTypeValue.cpp:431](src/UserTypeValue.cpp:431) | HashTable destroy skipped |

**Fix:** in every `initialize_handlers()` add `handlers.free_obj = php_driver_<type>_free;`. Reference: [src/Cluster/DefaultClusterHandlers.cpp:75](src/Cluster/DefaultClusterHandlers.cpp:75), [src/BatchStatement.cpp:148](src/BatchStatement.cpp:148), [src/FutureSession.cpp:171](src/FutureSession.cpp:171), [src/FutureValue.cpp:89](src/FutureValue.cpp:89) — these already do it correctly.

### 1.2 High — session leaked on synchronous connect failure

[src/Cluster/DefaultCluster.cpp:89](src/Cluster/DefaultCluster.cpp:89) — `cass_session_new()` succeeds; if `cass_session_connect*()` then errors or times out, the non-persistent path at [:123](src/Cluster/DefaultCluster.cpp:123)/[:138](src/Cluster/DefaultCluster.cpp:138) frees the future but not the session.

### 1.3 High — cached persistent future is never invalidated on timeout

[src/Cluster/DefaultCluster.cpp:112](src/Cluster/DefaultCluster.cpp:112) — when a persistent connect times out, the cache entry is dropped at [:118](src/Cluster/DefaultCluster.cpp:118) for the non-existing path but a previously cached entry from [:82](src/Cluster/DefaultCluster.cpp:82) is reused on the next call, so a stuck future stays live indefinitely.

### 1.4 Medium — error message extracted after future is freed

[src/DefaultSession.cpp:620](src/DefaultSession.cpp:620)–[626](src/DefaultSession.cpp:626): `php_driver_future_is_error()` reads the future's error string after `cass_future_free()`. Copy via `cass_future_error_message()` into a `zend_string` first, then free.

---

## 2. Zend Engine API correctness (PHP 8.3 → 8.5)

### 2.1 ~~Critical — `zend_object` is not last in five structs~~ [FALSE POSITIVE]

Verified at [include/php_driver_types.h:150-220](include/php_driver_types.h:150): `zend_object zendObject;` IS the last member in `collection`, `map`, `set`, `tuple`, and `user_type_value`. The audit agent misread the struct ordering.

### 2.2 ~~Critical — `zend_register_internal_class_ex()` removed in PHP 8.4~~ [FALSE POSITIVE]

Verified against [php/8.4-debug-nts/src/Zend/zend_API.h:392](php/8.4-debug-nts/src/Zend/zend_API.h:392) and [php/8.5-debug-nts/include/php/Zend/zend_API.h:394](php/8.5-debug-nts/include/php/Zend/zend_API.h:394) — `zend_register_internal_class_ex` is still exported in **both** PHP 8.4 and 8.5. `zend_register_internal_class_with_flags` is a new *addition* in 8.4, not a replacement. The current gen_stub-emitted code is correct. Commit `a1087182` (the prior arginfo regen) already chose the 8.3 gen_stub for the broadest compat — leave as-is.

### 2.3 High — handler offset/free_obj inconsistency

Some modules assign `.std.offset` / `.std.free_obj`, others assign `.offset` / `.free_obj` directly. PHP 8.1+ collapsed these to a flat struct, but the codebase mixes both — e.g. [src/Blob.cpp:194](src/Blob.cpp:194) (`.std.offset`) vs [src/ExecutionOptions.cpp:359](src/ExecutionOptions.cpp:359) (`.offset`). Pick one (`offset`/`free_obj` directly on `zend_object_handlers`) and apply globally.

### 2.4 Medium — legacy `zend_parse_parameters()` (20 sites)

Not a deprecation — `zend_parse_parameters` is still in [php/8.4-debug-nts/src/Zend/zend_API.h:362](php/8.4-debug-nts/src/Zend/zend_API.h:362) and [php/8.5-debug-nts/include/php/Zend/zend_API.h:364](php/8.5-debug-nts/include/php/Zend/zend_API.h:364). It's a CLAUDE.md style preference: modern API (`ZEND_PARSE_PARAMETERS_START/END`) is faster, type-checked at compile time, and supports try/catch flow. 20 call sites across DefaultSession, Collection, Map, Set, Tuple, UserTypeValue — slated for stage-3 macro purge, not a build blocker.

### 2.5 ~~High — `zend_hash_*_internal_pointer*` / `move_forward` removed in 8.4~~ [FALSE POSITIVE]

Verified at [php/8.4-debug-nts/src/Zend/zend_hash.h:257](php/8.4-debug-nts/src/Zend/zend_hash.h:257) and [php/8.5-debug-nts/include/php/Zend/zend_hash.h:260](php/8.5-debug-nts/include/php/Zend/zend_hash.h:260) — `zend_hash_internal_pointer_reset_ex` and friends still exist. Migrating to `ZEND_HASH_FOREACH_*` is a style improvement, not a fix.

### 2.6 ~~Medium — `zend_call_method_with_*_params()` removed in 8.4~~ [FALSE POSITIVE]

Verified at [php/8.4-debug-nts/src/Zend/zend_interfaces.h:43](php/8.4-debug-nts/src/Zend/zend_interfaces.h:43) and [php/8.5-debug-nts/include/php/Zend/zend_interfaces.h:43](php/8.5-debug-nts/include/php/Zend/zend_interfaces.h:43) — both `zend_call_method_with_0_params` and `_1_params` are still defined (as `static zend_always_inline`). No migration needed for build correctness.

### 2.7 Medium — pointer-arithmetic object fetch macros

[include/php_driver_types.h:102](include/php_driver_types.h:102)–[126](include/php_driver_types.h:126) use raw `((char*)obj - sizeof(...))`. Replace with `XtOffsetOf` (or `offsetof`) — see [src/Cluster/Cluster.cpp:240](src/Cluster/Cluster.cpp:240) for the correct pattern.

### 2.8 Medium — old `PHP_ME` / manual ARG_INFO_EX tables

Stub migration covers most modules; legacy table style remains in [src/Database/](src/Database/), [src/Type/](src/Type/), and [src/Numbers/](src/Numbers/). These were called out as "Legacy" in CLAUDE.md — keep porting until empty.

---

## 3. ScyllaDB C driver (`cass_*`) usage

### 3.1 Critical — wrong API for private key [FIXED]

[src/SSLOptions/Builder.cpp:104](src/SSLOptions/Builder.cpp:104) called `cass_ssl_set_cert_n()` on the private key. Fixed to use `cass_ssl_set_private_key_n()` with the passphrase from `builder->passphrase` (which the stub already accepts as a second arg to `withPrivateKey`). SSL client auth now actually works.

### 3.2 Critical — `cass_ssl_new()` NULL not checked [FIXED]

[src/SSLOptions/SSLOptions.cpp:44](src/SSLOptions/SSLOptions.cpp:44) — now throws `RuntimeException` on allocation failure instead of returning an object whose first SSL setter would null-deref.

### 3.3 High — `cass_statement_new()` / `cass_batch_new()` NULL not checked [PARTIALLY FIXED]

[src/DefaultSession.cpp:389](src/DefaultSession.cpp:389) — `cass_statement_new()` was already guarded at line 399 (audit was wrong). [src/DefaultSession.cpp:415](src/DefaultSession.cpp:415) — `cass_batch_new()` is now NULL-checked.

### 3.4 High — `cass_iterator_from_*()` NULL not checked [FIXED]

[util/src/result.cpp:190](util/src/result.cpp:190), [:217](util/src/result.cpp:217), [:246](util/src/result.cpp:246), [:280](util/src/result.cpp:280), [:314](util/src/result.cpp:314) — all five collection/map/set/tuple/UDT iterator constructors now skip the loop if NULL (collection-of-wrong-type or null value).

### 3.5 ~~High — synchronous `cass_future_wait()` with no timeout~~ [BY DESIGN]

[util/src/future.cpp:37](util/src/future.cpp:37) only takes the unbounded `cass_future_wait` path when the caller passes `null` or omits the `$timeout` argument. That is the documented contract (matches DataStax PHP/Python driver semantics: `null` = wait forever). Users opt in to bounded waits via `Cluster::withDefaultTimeout()` or by passing a `$timeout` to `execute()`. The cpp-driver itself enforces a separate `request_timeout` at the socket level. Changing this would break long-running queries by surprise.

### 3.6 ~~Medium — `cass_value_type()` not checked before `cass_value_get_*`~~ [NEEDS DESIGN]

[util/src/result.cpp:58-180](util/src/result.cpp:58) dispatches on the column's *declared* `cass_data_type_type()`. If the server's actual value type ever diverges from the declared schema type, the C driver itself returns `CASS_ERROR_LIB_INVALID_VALUE_TYPE` from the `cass_value_get_*` call — which we already check via `ASSERT_SUCCESS_BLOCK`. So the audit's claim of "silent corruption" doesn't hold: the cpp-driver catches it. An extra `cass_value_type()` probe per cell would add measurable overhead with no behavioural benefit.

### 3.7 Medium — paging-state set return code ignored

[src/Database/Rows.cpp:272](src/Database/Rows.cpp:272). Bubble the `CassError` up to the user instead of executing with an unset paging state.

---

## 4. Performance & modernisation

### 4.1 Quick wins (1–2 days each)

1. **Cache interned option keys.** [src/ExecutionOptions.cpp:50](src/ExecutionOptions.cpp:50)–[184](src/ExecutionOptions.cpp:184) does 8 `zend_hash_str_find` per execute. Intern once at MINIT (`zend_string_init_interned`) and use `zend_hash_find` thereafter.
2. **Avoid `estrndup` of CQL.** [src/SimpleStatement.cpp:35](src/SimpleStatement.cpp:35) and [src/ExecutionOptions.cpp:113](src/ExecutionOptions.cpp:113) copy strings that arrive as `zend_string*`. Store a refcounted `zend_string*` via `zend_string_copy`.
3. **Collapse `__get` if-chain.** [src/ExecutionOptions.cpp:238](src/ExecutionOptions.cpp:238)–[294](src/ExecutionOptions.cpp:294) does 8 `zend_string_equals_literal` comparisons. Switch on `ZSTR_LEN` first, or move the whole class to PHP 8.4 property hooks.
4. **Bind dispatch table.** [src/DefaultSession.cpp:55](src/DefaultSession.cpp:55)–[300](src/DefaultSession.cpp:300) currently does 41 `instanceof_function()` calls per bind. Replace with a `HashTable` keyed by `zend_class_entry*` → bind-function pointer.

### 4.2 Larger refactors

5. **CassValue → zval pipeline.** [util/src/result.cpp:40](util/src/result.cpp:40)–[150](util/src/result.cpp:150): big switch with `emalloc`+`memcpy` per row. For Blob, prefer `zend_string_init` directly. For UUID/Bigint, consider per-request object pools.
6. **Async via `cass_future_set_callback`.** Today everything blocks on `cass_future_wait`. A callback-driven future that integrates with Fibers would unlock real async — large scope, but the single biggest perf win for high-fan-out workloads.
7. **UserType field index map.** [src/UserTypeValue.cpp:56](src/UserTypeValue.cpp:56), [:161](src/UserTypeValue.cpp:161), [:199](src/UserTypeValue.cpp:199), [:226](src/UserTypeValue.cpp:226) all re-lookup field names. Pre-compute an `name → index` table at type-resolution time.

### 4.3 PHP 8.4 features worth adopting

- **Property hooks** to replace `__get`-driven options classes (`ExecutionOptions`, the various `Default*` wrappers).
- **Asymmetric visibility** (`public get, private set`) for value types — Blob, Uuid, Inet, Duration — replaces hand-written setters.
- **Lazy objects** for `Future*` types so result materialisation defers to first use, freeing memory in high-cardinality result sets.

### 4.4 PHP 8.5 features

- **Persistent class entries** — verify LTO is on so the linker dedupes them (no code change, just CMake).
- **Pipe operator** — mostly a user-facing ergonomics win, not driver-internal.

### 4.5 Build & link

- Add `-fno-plt` in [cmake/TargetOptimizations.cmake:52](cmake/TargetOptimizations.cmake:52) — extension already uses `-fvisibility=hidden`, the PLT indirection is pure overhead.
- Add `-ffunction-sections -fdata-sections` for `Release`/`RelWithDebInfo` + `-Wl,--gc-sections` to drop unused code from the `.so`.
- Switch IPO to thin-LTO on Linux: `target_compile_options(${target} PRIVATE -flto=thin)` next to the `INTERPROCEDURAL_OPTIMIZATION` set in root [CMakeLists.txt:168](CMakeLists.txt:168).
- Consider a version script (`-Wl,--version-script=…`) so only `get_module` is exported on ELF targets.

---

## 5. What was actually fixed (this branch)

Verification + remediation pass on 2026-05-21:

**[FIXED]** real bugs:
- 3.1 — `cass_ssl_set_cert_n` → `cass_ssl_set_private_key_n` (with passphrase). Client-cert SSL was silently broken.
- 3.2 — `cass_ssl_new()` NULL deref guard.
- 3.3 — `cass_batch_new()` NULL guard (`cass_statement_new` was already guarded).
- 3.4 — Five `cass_iterator_from_*` NULL guards in result.cpp.

**[FALSE POSITIVE]** debunked against the installed PHP 8.4 and 8.5 headers in [php/](php/):
- 1.1 (free_obj registrations) — macro at [include/php_driver.h:66](include/php_driver.h:66) handles it.
- 2.1 (struct layout) — `zendObject` IS the last member.
- 2.2, 2.5, 2.6 — APIs all still exist in 8.4 + 8.5.
- 3.5 (timeout) — `null` = wait-forever is the documented contract.
- 3.6 (value type validation) — cpp-driver already returns `CASS_ERROR_LIB_INVALID_VALUE_TYPE`.

**Remaining work items (modernization, not bugs):**
- 2.4 — 20 sites of legacy `zend_parse_parameters` to migrate to `ZEND_PARSE_PARAMETERS_START` (CLAUDE.md style).
- Section 4 — all perf and 8.4/8.5 feature adoption opportunities.

These are tracked here for future PRs but did not need to land with this audit.

## 6. Methodology / provenance

Findings were produced by four parallel read-only Explore agents on commit `8ed8c35c` covering: memory safety (`free_obj`, `cass_*_free`, globals), Zend API correctness (deprecations, struct layout, parameter parsing), `cass_*` lifecycle (NULL checks, futures, SSL), and performance/modernisation (8.4/8.5 features, build flags). Reports cross-referenced and de-duplicated by hand. No code was modified.
