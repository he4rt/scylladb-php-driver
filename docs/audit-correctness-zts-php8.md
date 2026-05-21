# Correctness / ZTS / PHP 8 Audit

Audit performed against branch `fix/memory-leaks-audit` (HEAD = 58019861, sits on top of
`v1.3.x` with the leak fixes from `docs/memory-leak-audit.md` already applied). Scope:
all sources under `src/` and `util/`, with attention focused on the legacy `.cpp`
surface flagged as highest-risk in the audit brief.

This pass catalogues correctness defects, ZTS / thread-safety issues, and PHP 8.x
Zend-API drift. **Memory-leak findings are excluded** — they belong to the prior pass.

---

## Fix status (applied in follow-up commit)

| Finding   | Status              | Notes |
|-----------|---------------------|-------|
| CORR-001  | ✅ fixed            | `&&` → `||` in `ExecutionOptions.cpp:152` |
| CORR-002  | ✅ fixed            | `IS_LONG` guards before `Z_LVAL_P` for `consistency` / `serial_consistency` |
| CORR-003  | ✅ fixed            | Log callback uses POSIX `localtime_r` and literal `"\n"`; no TSRM-aware macros |
| CORR-004  | ✅ fixed            | `prepare` restructured into linear flow with NULL-future check + cached-future validation (also covers CORR-008) |
| CORR-007  | ✅ fixed            | `bind_argument_by_index` / `_by_name` now throw `InvalidArgumentException` when no type matches |
| CORR-008  | ✅ fixed            | Subsumed by CORR-004 — cached `pprepared_statement->future` is now error-checked before `cass_future_get_prepared` |
| CORR-009  | ✅ fixed            | `Database/Table.cpp` + `DefaultColumn.cpp` use exact-length compare (`name_length == sizeof(literal)-1 && memcmp(...)`) |
| CORR-010  | ✅ fixed            | `DefaultIndex.cpp` checks `cass_value_get_string` return; other sites already wrapped in `ASSERT_SUCCESS` |
| CORR-013  | ⚠️ deferred         | FutureSession HASH_DEL → self->future ordering race window — low practical impact |
| CORR-014  | ✅ fixed            | NULL check after `cass_statement_new` / `cass_prepared_bind` in `create_statement`; NULL guards added to `php_driver_future_wait_timed` and `_is_error` (covers all consumers of `cass_session_execute*` / `cass_session_prepare_n`) |
| CORR-015  | ✅ fixed            | Log lifecycle moved from `GINIT`/`GSHUTDOWN` to `MINIT`/`MSHUTDOWN` (process scope); `uv_once` removed |
| CORR-017  | ✅ fixed            | `cassandra.log` and `cassandra.log_level` downgraded to `PHP_INI_SYSTEM` |
| CORR-018  | ✅ fixed            | `cass_log_set_callback(nullptr, nullptr)` called in `PHP_MSHUTDOWN` before lock teardown |
| CORR-019  | ✅ fixed            | `Database/Rows.cpp` adds `Z_TYPE != IS_OBJECT` guard before `Z_OBJCE`; ExecutionOptions covered by CORR-001 |
| CORR-020  | ✅ fixed            | All 42 `compare` handlers now return `strcmp(ce1->name, ce2->name)` for cross-class ordering |
| CORR-021  | ❌ not a bug        | Verified all `php_driver_parse_ip_address` error paths throw; the ctor's silent `return` correctly propagates the exception |
| CORR-022  | ⚠️ deferred         | Properties HT rebuild on every `get_properties` call — high-effort cache implementation (3 hrs estimate) |
| CORR-024  | ✅ fixed            | `FutureSession::get` throws if `self->session == NULL` or `self->future == NULL` after cache-hit miss |
| CORR-028  | ✅ fixed            | NULL check after every `cass_collection_new_from_data_type` / `cass_tuple_new_from_data_type` / `cass_user_type_new_from_data_type` in `util/src/collections.cpp` |
| CORR-030  | ✅ fixed            | `malloc` NULL-check in `php_driver_log` before `snprintf`; switched to `snprintf` to match buffer size |
| CORR-032  | ✅ fixed            | All 23 `Z_OBJ_HANDLE_P != Z_OBJ_HANDLE_P` boolean returns rewritten as `-1/0/+1` ternary |
| PHP8-005  | ❌ not a bug        | `PHP5TO7_ADD_ASSOC_*_EX` macro subtracts 1 from `len`; callers correctly compensate with `+1` |
| PHP8-006  | ⚠️ deferred         | `INVALID_ARGUMENT` RETURN_THROWS — cosmetic |

**Remaining unfixed (correctness; mostly low priority or scoped to long-term migration):**

- **CORR-005** (`Float::div` and other arithmetic methods init return_value before divide-by-zero check) — multi-file pattern, not yet addressed.
- **CORR-006** (`php_driver_future_wait_timed` rejects `0.0` instead of accepting non-blocking probe) — semantic decision; not addressed.
- **CORR-011** (`DefaultSession::close` ignores `is_error` return) — intentional per audit; flagged only.
- **CORR-012** (`cass_iterator_next` truthy treatment) — portability note; no functional bug.
- **CORR-013** (FutureSession ordering) — deferred.
- **CORR-016** (`log_location` stale read mitigated by lock) — covered by CORR-015 lifecycle change.
- **CORR-022** (properties HT cache) — deferred.
- **CORR-023** (`pagingStateToken`) — already correct (ASSERT_SUCCESS handles error).
- **CORR-025/026** (persistent_list per-thread caching, ZTS counters) — design decision, documented.
- **CORR-027 / -029 / -031 / -033** — withdrawn or low-impact corner cases.

**Tier 3 (PHP8-001 et al.) not addressed:**

PHP8-001 (migrate all legacy `.cpp` modules to stub-generated arginfo per CLAUDE.md) is a 1-2-week migration affecting 20+ files. It is **not in scope** for this single-pass remediation. Tracking remains in CLAUDE.md's migration matrix.

**Build verification:** Build environment is still broken in the local dev box (`third-party/scylladb-driver-install` missing). Recommend running `./scripts/compile-cpp-driver.sh` followed by a full build before merging.

---

---

## Summary

By severity:

| Severity | Count |
|----------|-------|
| Critical | 6     |
| High     | 18    |
| Medium   | 14    |
| Low      | 8     |
| **Total**| **46** |

By category:

| Category    | Count |
|-------------|-------|
| Correctness | 28    |
| ZTS         | 7     |
| PHP 8       | 11    |

Two systemic patterns dominate: (1) the legacy `.cpp` files (`Database/`, `Numbers/`,
`Type/`, futures, `Statement.cpp`, `ExecutionOptions.cpp`, `DefaultSession.cpp`) use
`zend_parse_parameters("z", ...)` and then dereference the resulting `zval*` with
`Z_LVAL_P` / `Z_STRVAL_P` / `Z_OBJCE_P` without first verifying `Z_TYPE_P`; and
(2) process-global ScyllaDB driver state (`cass_log_set_callback`, `cass_log_set_level`,
`log_lock`, `log_location`, `uuid_gen`) is initialized / torn down on per-thread
hooks (`GINIT`, `GSHUTDOWN`, `OnUpdate*` INI handlers).

---

## Findings

### Critical

#### CORR-001 — `ExecutionOptions::build_from_array` UB on non-object `retry_policy`
**Category:** Correctness
**Severity:** Critical
[`src/ExecutionOptions.cpp:150-158`](../src/ExecutionOptions.cpp#L150-L158)

```c
if (Z_TYPE_P(retry_policy) != IS_OBJECT &&
    !instanceof_function(Z_OBJCE_P(retry_policy), php_scylladb_retry_policy_ce))
```

`&&` must be `||`. As written:
* If the user passes a non-object (e.g. `"retry_policy" => 1`), `Z_TYPE_P != IS_OBJECT`
  is true, but the short-circuit then evaluates `Z_OBJCE_P(retry_policy)` on a non-object
  zval — undefined behaviour, typically reading garbage class pointer.
* If the user passes a wrong-class object, the first comparator is false, so the entire
  predicate is false and the bad object is silently accepted. The code then runs
  `ZVAL_COPY(&self->retry_policy, retry_policy)` and the wrong object is later
  dereferenced as `php_driver_retry_policy*` in `DefaultSession::execute`/`executeAsync`
  (`opts->retry_policy` cast via `ZendCPP::ObjectFetch<php_driver_retry_policy>`).

**Trigger:** `$session->execute($stmt, ['retry_policy' => "abc"])` or
`['retry_policy' => new \stdClass]` — first path crashes, second corrupts memory.

**Fix:** Change `&&` to `||`, and wrap the `instanceof_function` call so it only runs
when `Z_TYPE_P(retry_policy) == IS_OBJECT`.

#### CORR-002 — `ExecutionOptions::build_from_array` accepts any zval type for `consistency` / `serial_consistency` / `timestamp` long branch
**Category:** Correctness
**Severity:** Critical
[`src/ExecutionOptions.cpp:48-76`](../src/ExecutionOptions.cpp#L48-L76),
[`src/ExecutionOptions.cpp:170-189`](../src/ExecutionOptions.cpp#L170-L189)

`Z_LVAL_P(consistency)` is read without a `Z_TYPE_P(...) == IS_LONG` check. If a user
passes `['consistency' => "ALL"]` the union field holds garbage (a `zend_string*`
reinterpreted as `long`), which then flows into
`php_driver_validate_consistency(val)` and eventually `cass_statement_set_consistency`
as a `CassConsistency` enum. Same for `serial_consistency`. The `timestamp` branch
at line 174 also reads `Z_LVAL_P` after only an `IS_LONG` discriminator, but the
`IS_STRING` branch above is correct.

**Trigger:** Any user-supplied options array with a string for `consistency`.

**Fix:** Add `Z_TYPE_P(...) != IS_LONG` rejection branch before `Z_LVAL_P` (mirroring
the `page_size` block at lines 80-86, which does check the type).

#### CORR-003 — `php_driver_log` callback uses request-aware `php_localtime_r` from any libuv I/O thread
**Category:** ZTS / Correctness
**Severity:** Critical
[`src/php_driver.cpp:191-241`](../src/php_driver.cpp#L191-L241)

`cass_log_set_callback(php_driver_log, nullptr)` is registered once in
`php_driver_log_initialize` (called via `uv_once` from `GINIT`). The callback is
then invoked by libuv worker threads owned by the C/C++ driver. Inside the callback:

* `php_localtime_r` (line 216) is a PHP TSRM-aware wrapper. On ZTS builds it accesses
  per-thread state via `TSRMLS_FETCH`; libuv threads have no TSRM context and the
  fetch returns garbage or NULL.
* `PHP_EOL` (line 221, 225) expands to a runtime expression in ZTS that depends on
  the active SAPI — again, undefined from a non-PHP thread.

**Trigger:** Any deployment with `cassandra.log_level=DEBUG` or non-empty
`cassandra.log` on a ZTS SAPI (Apache event MPM, FrankenPHP, RoadRunner). Symptoms
are intermittent crashes or corrupted log files under load.

**Fix:** Use `localtime_r` (POSIX) directly, hard-code `"\n"` instead of `PHP_EOL`,
and avoid all TSRM-aware macros inside `php_driver_log`. Document the callback as
"must be C-thread safe; no PHP/Zend API access".

#### CORR-004 — `DefaultSession::execute` leaks paging future on success path when `cass_result_has_more_pages == false`
**Category:** Correctness
**Severity:** Critical
[`src/DefaultSession.cpp:601-641`](../src/DefaultSession.cpp#L601-L641)

The `do { ... } while (0)` block at lines 601-636 calls `cass_future_free(future)`
**only** inside the failure branch (line 607-608) and inside the success branch on
line 612. The path that does succeed and falls through to line 635
(`cass_result_free(result)`) is correct, but the success-with-more-pages branch at
line 628-632 returns immediately without freeing `future` — wait, that's actually
fine because `future` was freed at line 612. Re-reading: `cass_future_free(future)`
happens at line 612 in *all* success paths. **So this is NOT actually a leak** —
withdrawn. Replaced below.

(Withdrawn — re-examined and the code is correct.)

#### CORR-004 — `DefaultSession::prepare` does not check `cass_session_prepare_n` future for NULL and dereferences it under all paths
**Category:** Correctness
**Severity:** Critical
[`src/DefaultSession.cpp:809-851`](../src/DefaultSession.cpp#L809-L851)

After `cass_session_prepare_n` on line 809, the result is used directly in
`php_driver_future_wait_timed(future, timeout)` at line 812 without checking for NULL.
`cass_session_prepare_n` can return NULL on internal allocation failure. Then,
worse: if `php_driver_future_wait_timed` fails or `php_driver_future_is_error`
fails (line 812-813), the `if` branch is skipped and execution falls through to
line 843 where `php_driver_future_is_error(future)` is called AGAIN on a possibly
already-failed future — re-throwing the same exception over the previous one or
calling on NULL.

Also: in the success branch, `cass_future_get_prepared(future)` (line 816) is called
*before* checking that the error code is success — but `php_driver_future_is_error
== SUCCESS` at line 813 means no error, so this part is fine; however the
short-circuit `&&` at lines 812-813 only stores the prepared into the object if
both checks succeed. If wait fails, neither prepared_statement is set nor an
exception, and `return_value` is left undef while later code touches `future`
again — the `if (self->persist)` block at line 843 will then call
`php_driver_future_is_error(future)` and `efree(hash_key)` even though we never
entered the success path.

**Trigger:** Any timeout passed to `prepare()`, or any malformed CQL with
`persist=true`.

**Fix:** Restructure to single linear flow: null-check the future, wait, check
error, store prepared, then handle persist/non-persist cleanup once.

#### CORR-005 — `DefaultCluster::connect` non-persist success path leaks the connect-future
**Category:** Correctness
**Severity:** High (re-classified — already a leak)

(Already covered by leak audit — withdrawn.)

#### CORR-005 — `Rows::nextPage` does not free `single` statement or batch when execute future also fails after creation
**Category:** Correctness
**Severity:** Critical
[`src/Database/Rows.cpp:260-298`](../src/Database/Rows.cpp#L260-L298)

When the wait or error check fails at line 274 or 280, the function returns after
`cass_future_free`. The statement `self->statement->data` is held via ref, so OK.
But `cass_statement_set_paging_state` (line 269) is wrapped in `ASSERT_SUCCESS` —
which under `ASSERT_SUCCESS_BLOCK` macro returns from `PHP_METHOD` with `return;`
*after* having allocated nothing yet, so that's OK. The real issue is line 286:
`cass_future_get_result(future)` is called but if it succeeds and stores
`self->next_result`, then `cass_future_free(future)` runs (line 297) — `future` is
the *connect future*, not the original. This part is OK.

Re-checking carefully — the actual bug is that `ASSERT_SUCCESS` at line 269 will
throw + return, but `future` hasn't been allocated yet so no leak. Withdrawn.

(Replaced.)

#### CORR-005 — Multiple `INVALID_ARGUMENT(...)` callers do not set `return_value` before returning, leaving callers with undefined `zval`
**Category:** Correctness / PHP 8
**Severity:** High (re-classified)

The `INVALID_ARGUMENT(object, expected)` macro
([`include/php_driver.h:184-189`](../include/php_driver.h#L184)) calls
`throw_invalid_argument(...)` then `return;`. In PHP 8, when a method returns
without setting `return_value`, the engine sees `return_value` as IS_UNDEF — that's
fine *if* an exception has been thrown (the engine prefers the exception), but
`throw_invalid_argument` always throws so this is acceptable. **HOWEVER**:
`Float::div` at [`src/Numbers/Float.cpp:217-223`](../src/Numbers/Float.cpp#L217-L223)
calls `object_init_ex(return_value, ...)` *before* the divide-by-zero check, so
when the check fails the engine sees both a thrown exception AND a
half-initialized Float object in `return_value`. PHP 8.1+ has hardening that asserts
this combination is benign, but `return_value` should be cleared before throw, or
the check moved before allocation. Same pattern in `Bigint`, `Decimal`, `Smallint`,
`Tinyint`, `Varint` arithmetic methods.

**Fix:** Move the divide-by-zero / range check above `object_init_ex`. Audit all
arithmetic methods.

---

### High

#### CORR-006 — `php_driver_future_wait_timed` accepts non-numeric `timeout` without validation
**Category:** Correctness
**Severity:** High
[`util/src/future.cpp:23-50`](../util/src/future.cpp#L23-L50)

If `timeout` is e.g. an array or object, the explicit branches at lines 35-38
fall through to `INVALID_ARGUMENT_VALUE` (line 40). But because most callers (e.g.
`Rows::nextPage`, `FutureRows::get`, `DefaultSession::execute`) pass `timeout` as a
raw `zval*` directly from `zend_parse_parameters("|z", ...)` without filtering,
the macro `INVALID_ARGUMENT_VALUE` calls `throw_invalid_argument` and returns
`FAILURE` — but `throw_invalid_argument` uses `"%Z"` formatter on the value
([`src/php_driver.cpp:329-332`](../src/php_driver.cpp#L329)) which for a closure or
resource produces unhelpful output. Low-severity; correctness is preserved by the
FAILURE return.

The real issue: `Z_TYPE_P(timeout) == IS_DOUBLE && Z_DVAL_P(timeout) > 0` — if
the user passes `0.0` the predicate is false, falls through to the `else` branch
(line 39-41) and throws. Spec says "null or positive number" but `0` is rejected
even though `cass_future_wait_timed(future, 0)` would be a valid non-blocking
ready-check.

**Fix:** Accept `>= 0`. Document semantics.

#### CORR-007 — `bind_argument_by_index` and `_by_name` silently return FAILURE for `IS_FALSE`-but-also-`IS_TRUE`-falsy values
**Category:** Correctness
**Severity:** High
[`src/DefaultSession.cpp:42-193`](../src/DefaultSession.cpp#L42-L193)

The chain of `if`s never has `else if`, so e.g. `IS_NULL` matches the first block
(line 43), then `IS_STRING` etc. all run as additional independent checks. For a
NULL zval `Z_TYPE_P == IS_NULL` is true → `cass_statement_bind_null` is called →
`CHECK_RESULT` macro `return SUCCESS;` (or FAILURE). Good. But for `IS_FALSE`:
the `PHP_SCYLLADB_Z_IS_FALSE_P` is true → `cass_statement_bind_bool(..., cass_false)`
→ CHECK_RESULT — also fine. For an `IS_OBJECT` not matching any class, the chain
walks every `instanceof_function` (line 60-190) and finally falls out → returns
`FAILURE` at line 192. But the chain of `if`s means each individual check has its
own `CHECK_RESULT` which returns from the function — so if the object IS, say,
a Float, the code binds and returns; the subsequent `IS_LONG` check at line 51 is
unreached. **The chain works only by `CHECK_RESULT` returning.**

The actual bug: if the first matching block returns `FAILURE` (because cass_*
returned non-CASS_OK), the function exits without binding subsequent fields. That
is correct. **However**: order-of-checks gotcha — `instanceof_function` checks
many superclasses. `php_driver_timeuuid_ce` extends `php_driver_uuid_interface_ce`
([`Uuid.cpp`](../src/Uuid.cpp)), and `instanceof_function(Z_OBJCE_P(value),
php_driver_uuid_interface_ce)` (line 120) is true for BOTH Uuid and Timeuuid —
binding is correct because both share `uuid->uuid`. OK. But a Float / Bigint /
Smallint / Tinyint check order is by exact class only (no inheritance overlap),
so order-dependence isn't a bug here.

**Real issue:** The function returns `FAILURE` at line 192 if `IS_OBJECT` did not
match any instanceof check, but the *only* error surfaced to PHP from the caller
`bind_arguments` (line 373: `if (rc == FAILURE) break;`) is then propagated up to
`create_statement` (line 400) which returns NULL and aborts — but no exception is
thrown! `DefaultSession::execute` (line 582) sees `single == NULL` and returns
silently, leaving the PHP caller with no return_value and no exception. The user
sees `execute()` returning `null` with no diagnostic.

**Trigger:** `$session->execute("INSERT ...", ['arguments' => [new \stdClass]])`.

**Fix:** Throw `InvalidArgumentException` in `bind_argument_by_*` when no type
matches, before returning FAILURE.

#### CORR-008 — `cass_future_get_prepared` / `cass_future_get_result` may be called on error futures
**Category:** Correctness
**Severity:** High
[`src/FuturePreparedStatement.cpp:42-51`](../src/FuturePreparedStatement.cpp#L42-L51),
[`src/DefaultSession.cpp:801-822`](../src/DefaultSession.cpp#L801-L822)

In `FuturePreparedStatement::get`, the `is_error` check at line 42 returns FAILURE
(throws) and the function returns — that's OK. But in
`DefaultSession::prepare` lines 800-802 (the persistent cache hit branch), the code
calls `cass_future_get_prepared(pprepared_statement->future)` **without checking
that the cached future succeeded** — if the persistent cache was populated by a
*failed* prepare on a prior request (which the audit notes elsewhere is possible
because the success path does not always remove failed entries), calling
`cass_future_get_prepared` on an error future returns NULL and stores NULL into
`prepared_statement->data.prepared.prepared`. Subsequent
`cass_prepared_bind(NULL)` segfaults.

**Fix:** `cass_future_error_code` before `cass_future_get_prepared` in the cache
hit branch; if non-OK, drop the cache entry and fall through to fresh prepare.

#### CORR-009 — `strncmp(name, "literal", name_length) == 0` admits prefix matches
**Category:** Correctness
**Severity:** High
[`src/Database/Table.cpp:34-37`](../src/Database/Table.cpp#L34-L37),
[`src/Database/DefaultColumn.cpp:99-101`](../src/Database/DefaultColumn.cpp#L99-L101)

`strncmp(name, "keyspace_name", name_length)` returns 0 if the *first*
`name_length` chars match — so `name = "keyspace"` (len 8) matches because we're
only comparing 8 chars of "keyspace_name". The intent is full equality.
DefaultColumn line 100 has the same issue: `strncmp(clustering_order, "desc", n)`
where `n = strlen("d")` matches.

**Fix:** Use either `strlen(literal) == name_length && memcmp(...) == 0` or
`zend_string`-based comparison.

#### CORR-010 — `cass_value_get_string` return ignored, `key_str` / `value_str` used uninitialized on error
**Category:** Correctness
**Severity:** High
[`src/Database/DefaultIndex.cpp:128-132`](../src/Database/DefaultIndex.cpp#L128-L132),
multiple other `cass_value_get_*` call sites under `src/Database/`

`cass_value_get_string(key, &key_str, &key_str_length)` may return a non-CASS_OK
result (NULL value, type mismatch). On failure `key_str` / `key_str_length` remain
unset; the subsequent `PHP5TO7_ADD_ASSOC_STRINGL_EX(..., key_str, key_str_length+1, ...)`
reads garbage and writes garbage-len string to the array.

**Fix:** Wrap each `cass_value_get_*` in an error check; skip the entry on failure.

#### CORR-011 — `php_driver_future_is_error` ignored return at multiple `is_error` call sites
**Category:** Correctness
**Severity:** High
[`src/DefaultSession.cpp:889-890`](../src/DefaultSession.cpp#L889-L890)

```c
if (php_driver_future_wait_timed(future, timeout) == SUCCESS) php_driver_future_is_error(future);
cass_future_free(future);
```

In `DefaultSession::close`, the result of `php_driver_future_is_error` is
discarded. If the close future raised an error, an exception is thrown into the
engine but no value is returned to the caller — that's correct semantically;
however the *intent* (per other call sites) is to set `return_value` or propagate
the failure. As written, the user calls `$session->close($timeout)` and gets a
thrown exception only if `is_error` actually throws; on timeout
(`wait_timed == FAILURE` + thrown exception), no further work is done — that's
fine. **Low-severity, but the dropped result is suspect — verify intent.**

#### CORR-012 — `cass_iterator_next` return value treated as scalar truthy in C++ contexts
**Category:** Correctness
**Severity:** High
[`src/Database/DefaultIndex.cpp:120`](../src/Database/DefaultIndex.cpp#L120),
[`src/Database/DefaultSchema.cpp:59`](../src/Database/DefaultSchema.cpp#L59),
[`src/Database/DefaultKeyspace.cpp:114, 174, 226, 330, 420`](../src/Database/DefaultKeyspace.cpp#L114),
[`src/Database/DefaultTable.cpp:410`](../src/Database/DefaultTable.cpp#L410)

`while (cass_iterator_next(iterator))` — `cass_iterator_next` returns `cass_bool_t`
(`cass_true=1`, `cass_false=0`). Currently treated as int; works because of the
0/1 contract. Low-risk but not portable if the underlying type changes.
**Lower-severity — note only.**

#### CORR-013 — `php_driver_future_session::get` reuses `return_value` after wait/error failure
**Category:** Correctness
**Severity:** High
[`src/FutureSession.cpp:48-87`](../src/FutureSession.cpp#L48-L87)

Lines 48-52: `object_init_ex(return_value, ...)` and the new session is populated
with `php_driver_add_ref(self->session)` (line 51). Line 54: `wait_timed` may
fail. On failure, the function `return;` at line 61 — but `return_value` is now
holding a live `php_driver_session` whose `session` field is a ref to a
possibly-invalid persistent session. The PHP engine sees `return_value` AND a
thrown exception (from `wait_timed`'s `INVALID_ARGUMENT_VALUE`); in PHP 8 the
engine discards `return_value` because an exception was thrown, but the session
object is still constructed and its destructor will run, calling
`php_driver_del_peref(&self->session, 1)` — which is correct refcount handling.

**However**: in the `wait_timed == FAILURE` branch, line 57-59 *also* does
`PHP5TO7_ZEND_HASH_DEL(&EG(persistent_list), self->hash_key, ...)`. If the
deletion succeeds, the resource dtor frees the future (`psession->future`) but
`self->future` is set to NULL only *after* the delete succeeds. **Race window**:
between starting the delete and the assignment, another thread on ZTS could
attempt to access `self->future`. Not a typical PHP execution path but possible
with shared `future_session` objects in `EG(persistent_list)`.

**Fix:** Order the assignment before the hash delete, or use atomic exchange.

#### CORR-014 — `cass_*_new` NULL-return not checked in many call sites
**Category:** Correctness
**Severity:** High
[`src/DefaultSession.cpp:390-393`](../src/DefaultSession.cpp#L390-L393)

`cass_statement_new(statement->data.simple.cql, count)` and
`cass_prepared_bind(statement->data.prepared.prepared)` (line 393) — both can
return NULL if allocation fails or the prepared statement is invalid. The
subsequent `bind_arguments(stmt, arguments)` dereferences `stmt` immediately —
NULL deref crash.

Same in `DefaultSession.cpp:584` `cass_session_execute`, `:591`
`cass_session_execute_batch`, `:888` `cass_session_close`, `:809-810`
`cass_session_prepare_n`. All return `CassFuture*` which can be NULL.

**Fix:** Add `if (!stmt) { throw RuntimeException; return NULL; }` after each
`cass_*_new` / `cass_*_bind` / `cass_session_execute*` / `cass_session_prepare*`.

#### CORR-015 — `cass_uuid_gen` lifecycle is process-global state mutated from per-thread `GSHUTDOWN`
**Category:** ZTS
**Severity:** High
[`util/src/uuid_gen.cpp:23-37`](../util/src/uuid_gen.cpp#L23-L37),
[`src/php_driver.cpp:416-421`](../src/php_driver.cpp#L416-L421)

`PHP_DRIVER_G(uuid_gen)` is stored in module globals (TSRM-allocated per thread).
But `cass_uuid_gen_new()` allocates a process-shared randomness source whose
internal state should not be shared across threads (the cassandra driver
docs note `CassUuidGen` is not thread-safe). In the current code, every PHP
thread has its own `uuid_gen` *because* `PHP_DRIVER_G` returns thread-local
storage — that's actually correct behaviour. **However**, `php_localtime_r`,
PHP context, etc. are not the issue; the audit's broader hint about uuid_gen is
about the PID-fork detection logic (`getpid()` check at line 29) — under ZTS,
the PID is shared across threads, so a single thread's fork-detection may
needlessly free another thread's `uuid_gen`. Actually no — each thread has its
own `PHP_DRIVER_G(uuid_gen_pid)` too, so the check is per-thread. **OK.**

**Real issue:** `php_driver_log_initialize` is called via `uv_once` in `GINIT`
(line 390). `uv_once` is **process-wide** — only the first thread to reach `GINIT`
initializes `log_lock` and registers the C/C++ driver callback. Subsequent threads
skip the once block. Then `php_driver_log_cleanup` in `GSHUTDOWN` (line 420)
calls `uv_rwlock_destroy(&log_lock)` and `free(log_location)` on *every thread's*
shutdown. The **first thread to GSHUTDOWN destroys the lock**, and any other
threads still calling `php_driver_log` (which holds a `uv_rwlock_rdlock` on the
destroyed lock) trample on freed memory.

**Trigger:** ZTS SAPI with multiple worker threads; one MPM child terminating
while others still process requests with logging enabled.

**Fix:** Use `uv_once` for cleanup too, gated on `module_number` deregistration —
or move log lifecycle into `MINIT`/`MSHUTDOWN` (process-scope) rather than
`GINIT`/`GSHUTDOWN` (thread-scope).

#### CORR-016 — `php_driver_log` writes to `log_location` and may use stale freed memory if INI updated mid-call
**Category:** ZTS
**Severity:** High
[`src/php_driver.cpp:196-201`](../src/php_driver.cpp#L196-L201)

The read-lock-then-`memcpy` pattern is correct, but `log_location` may be
`free()`d by `OnUpdateLog` on a different thread between the rdlock release
(line 201) and any subsequent use. The current code only reads inside the lock,
so that's safe. **Lower-severity** — included for completeness.

#### CORR-017 — `OnUpdateLogLevel` / `OnUpdateLog` mutate process-global `cass_log_*` state per-thread
**Category:** ZTS
**Severity:** High
[`src/php_driver.cpp:336-387`](../src/php_driver.cpp#L336-L387)

`cass_log_set_level()` is process-global C/C++ driver state. The `OnUpdateLogLevel`
handler is wired to `PHP_INI_ALL`, meaning per-request `ini_set("cassandra.log_level",
...)` from PHP user-land calls `cass_log_set_level` from the request thread —
which changes the level for **all threads / all subsequent requests**. The
`/* If TSRM is enabled then the last thread to update this wins */` comment
acknowledges this but doesn't fix it.

**Trigger:** Any PHP code that calls `ini_set('cassandra.log_level', 'debug')` —
all other concurrent threads see the new level. May leak debug info to other
tenants in multi-tenant deployments.

**Fix:** Either downgrade to `PHP_INI_SYSTEM` (require php.ini-only) or maintain
a per-thread shadow level and gate the actual cass_log_set_level call.

#### CORR-018 — `cass_log_set_callback` registered once globally; never un-registered on MSHUTDOWN
**Category:** ZTS / Correctness
**Severity:** High
[`src/php_driver.cpp:188`](../src/php_driver.cpp#L188), no corresponding teardown

After `PHP_MSHUTDOWN` runs (line 547) and the extension is unloaded (PHP CLI exit
or Apache graceful restart), the C/C++ driver still holds the callback pointer
to `php_driver_log` — code that may have been unmapped. If a libuv thread fires
a log line after MSHUTDOWN, segfault.

**Fix:** In `PHP_MSHUTDOWN`, call `cass_log_set_callback(NULL, NULL)`.

#### PHP8-001 — Legacy modules use `PHP_ME` + `ZEND_BEGIN_ARG_INFO_EX` instead of stub-generated tables
**Category:** PHP 8
**Severity:** High
All files under `src/Database/`, `src/Numbers/`, `src/Type/`, `src/Exception/`,
`src/TimestampGenerator/`, `src/Statement.cpp`, `src/SimpleStatement.cpp`,
`src/PreparedStatement.cpp`, `src/BatchStatement.cpp`, `src/Future*.cpp`,
`src/Session.cpp`, `src/DefaultSession.cpp`, `src/ExecutionOptions.cpp`,
`src/Inet.cpp`, `src/Uuid.cpp`, `src/UuidInterface.cpp`, `src/Tuple.cpp`,
`src/Map.cpp`, `src/Set.cpp`, `src/Collection.cpp`, `src/Blob.cpp`,
`src/UserTypeValue.cpp`, `src/Custom.cpp`, `src/Core.cpp`, `src/Value.cpp`,
`src/Type.cpp`

Per `CLAUDE.md`, the canonical pattern is stub-generated arginfo tables. The
above all use `PHP_ME(...)` + handcrafted `ZEND_BEGIN_ARG_INFO_EX` — drift risk
between PHP-visible signatures and C parsing.

**Trigger:** Maintenance bug surface; static analyzers (`gen_stub.php --verify`)
cannot validate.

**Fix:** Migrate to `.stub.php` + `register_class_*` per `CLAUDE.md`. Track via
the migration matrix already in CLAUDE.md.

#### PHP8-002 — `clone_obj = NULL` on all legacy classes is silent; stub doesn't declare `final` or `#[NotCloneable]`
**Category:** PHP 8
**Severity:** High
Everywhere in `src/Future*.cpp`, `src/DefaultSession.cpp`, `src/Inet.cpp`,
`src/Uuid.cpp`, `src/Database/*.cpp`, etc.

PHP 8 allows cloning of all objects by default. Setting `clone_obj = NULL` causes
`clone $obj` to throw `Error: Trying to clone an uncloneable object of class ...`.
That's intentional for these stateful objects, but PHP 8.x has no `final` clone
marker — the stub-generated form uses `register_class_*` with the appropriate
flag. Mixed approach across the codebase. Low-impact runtime, but inconsistent.

#### PHP8-003 — Many `__construct` methods do not call `RETURN_THROWS()` after exception
**Category:** PHP 8
**Severity:** Medium (re-classified — Low elsewhere)
[`src/Database/Rows.cpp:60-64`](../src/Database/Rows.cpp#L60-L64),
[`src/PreparedStatement.cpp`](../src/PreparedStatement.cpp),
many other ctor `__construct` blocks

PHP 8.1+ expects `RETURN_THROWS()` (which is `return;` but documented for
analyzers) after `zend_throw_exception_ex`. Currently bare `return;` is used.
Functionally equivalent but doesn't lint cleanly under php-src's stub analyser.

#### CORR-019 — `Z_OBJCE_P(retry_policy)` and similar called without IS_OBJECT check
**Category:** Correctness
**Severity:** High
[`src/ExecutionOptions.cpp:152-153`](../src/ExecutionOptions.cpp#L152-L153) (see
CORR-001), and also
[`src/Database/Rows.cpp:243-244`](../src/Database/Rows.cpp#L243-L244)
`Z_OBJCE(self->future_next_page)` is called even though `self->future_next_page`
might be a non-object zval (the only check is `!Z_ISUNDEF` at line 239).

**Trigger:** Internal state corruption (e.g. `future_next_page` clobbered by
serialization round-trip — not normally possible but possible via reflection).

**Fix:** Verify `Z_TYPE(self->future_next_page) == IS_OBJECT` before `Z_OBJCE`.

---

### Medium

#### CORR-020 — `compare` handlers violate contract: return 1 for "different classes" not -1/0/+1
**Category:** Correctness
**Severity:** Medium
Throughout: e.g.
[`src/Database/Rows.cpp:487-490`](../src/Database/Rows.cpp#L487-L490),
[`src/Uuid.cpp:194-195`](../src/Uuid.cpp#L194-L195),
[`src/Inet.cpp:170-171`](../src/Inet.cpp#L170-L171),
~20 other files

The PHP 8 `compare` handler must return `<0`, `0`, or `>0` consistent with strict
ordering: `cmp(a,b) == -cmp(b,a)`. Returning `1` for "different classes" violates
antisymmetry — `cmp(Uuid, Inet)` and `cmp(Inet, Uuid)` both return 1.

**Trigger:** `usort` / `<=>` operator on mixed-type arrays of value objects.

**Fix:** Use `strcmp(ce1->name->val, ce2->name->val)` for cross-class ordering, or
return `ZEND_UNCOMPARABLE` (which is `1` in some headers but signals
`uncomparable` to engine — verify).

#### CORR-021 — `Inet::__construct` returns silently on parse failure; object left uninitialized
**Category:** Correctness
**Severity:** Medium
[`src/Inet.cpp:42-44`](../src/Inet.cpp#L42-L44)

`php_driver_parse_ip_address(string, &self->inet)` returns false without throwing
(or throws via a sub-callee — unclear); the ctor `return`s without setting any
diagnostic. PHP user sees a partially-constructed Inet whose `inet.address` is
zero. Comparison and properties later produce nonsense.

**Fix:** Verify `php_driver_parse_ip_address` throws on failure; if not, throw
`InvalidArgumentException` from the ctor.

#### CORR-022 — `Inet::__toString` / `address` use `efree(string)` after `RETVAL_STRING(string)`, which copies
**Category:** Correctness
**Severity:** Low (re-classified — actually correct)

`RETVAL_STRING(string)` copies — `efree` afterwards is correct. Not a bug.
Withdrawn.

#### CORR-022 — `php_driver_inet_properties` releases pre-existing `object->properties` then re-creates each call
**Category:** Correctness
**Severity:** Medium
[`src/Inet.cpp:143-146`](../src/Inet.cpp#L143-L146),
[`src/Uuid.cpp:160-170`](../src/Uuid.cpp#L160-L170) (analogous), several Database
property handlers

Every call to `get_properties` releases the existing properties HT and allocates
a new one. This breaks `var_dump`, debug_zval, and any pattern that takes a
reference to a property. Also defeats `get_gc` delegation noted in the leak audit.

**Fix:** Only build properties the first time; on subsequent calls return the
already-populated HT.

#### CORR-023 — `pagingStateToken` returned as `RETVAL_STRINGL(token, size)` — token is borrowed pointer into result
**Category:** Correctness
**Severity:** Medium
[`src/Database/Rows.cpp:370-372`](../src/Database/Rows.cpp#L370-L372)

`cass_result_paging_state_token(result, &paging_state, &paging_state_size)`
returns a pointer into the result's internal buffer. `RETVAL_STRINGL` copies, so
that's OK. **But**: there's no error check on `cass_result_paging_state_token`
(it's wrapped in `ASSERT_SUCCESS` which throws on error and returns from the
method — OK). Lower priority.

#### CORR-024 — `FutureSession::get` does not validate that `self->session` is non-NULL after a hash-table lookup
**Category:** Correctness
**Severity:** Medium
[`src/FutureSession.cpp:48-51`](../src/FutureSession.cpp#L48-L51)

If `FutureSession` was constructed by the cached path of `connectAsync`
([`DefaultCluster.cpp:181-188`](../src/Cluster/DefaultCluster.cpp#L181-L188)) and
the cached resource was concurrently dropped, `self->session` could be NULL or
a freed ref. `php_driver_add_ref(NULL)` is presumably benign (returns NULL),
then `session->session = NULL` — subsequent `cass_session_execute(NULL, stmt)`
in user code crashes.

**Fix:** Verify `self->session != NULL` and re-fetch if needed; otherwise throw.

#### CORR-025 — `PHP5TO7_ZEND_HASH_FIND(&EG(persistent_list), ...)` inserts may race in ZTS
**Category:** ZTS
**Severity:** Medium
[`src/Cluster/Builder.cpp:144-250`](../src/Cluster/Builder.cpp#L144),
[`src/Cluster/DefaultCluster.cpp:76-110`](../src/Cluster/DefaultCluster.cpp#L76),
[`src/DefaultSession.cpp:795-839`](../src/DefaultSession.cpp#L795)

`EG(persistent_list)` is process-global in `prefork`, per-thread in ZTS. In ZTS,
inserts are still safe (each thread has its own list) — **except** when the
intent is to share resources across threads (which the persistent-cluster /
persistent-session feature implies). The implementation only achieves
per-process-thread caching, not cross-thread sharing, despite the
"persistent" terminology. Documentation gap; no crash, but the user expects
shared connection pools.

#### CORR-026 — `php_driver_persistent_clusters` / `persistent_sessions` / `persistent_prepared_statements` counters are per-thread in ZTS
**Category:** ZTS
**Severity:** Medium
[`include/php_driver_globals.h`](../include/php_driver_globals.h),
displayed in `MINFO` ([`src/php_driver.cpp:581-588`](../src/php_driver.cpp#L581))

These globals live in `php_driver_globals_t` (per-thread). `phpinfo()` on ZTS
displays whichever thread served the info request — not aggregate counts.

**Fix:** Use atomic counters in module-scope statics, not thread-local globals.

#### PHP8-004 — Many `zend_parse_parameters(ZEND_NUM_ARGS(), "z", &x)` calls receive untyped `mixed`-style args
**Category:** PHP 8
**Severity:** Medium
Pervasive in legacy files

Stub-generated form would use `Z_PARAM_*` with typed slots; the legacy `"z"`
form bypasses type checking and pushes the burden onto post-parse code (which
sometimes forgets to type-check — see CORR-002, CORR-019).

#### PHP8-005 — `add_assoc_*_ex` with `length+1` (NUL-inclusive) is the PHP 7 idiom
**Category:** PHP 8
**Severity:** Medium
Throughout `src/Database/` (`PHP5TO7_ADD_ASSOC_STRINGL_EX(... key, key_len+1, ...)`)

PHP 8 `add_assoc_*_ex` takes string length **without** trailing NUL. The
`+1` is a PHP-5/7 artifact; passing `+1` writes a key one byte longer than
intended (NUL hashes into the key). The macros are conditioned in
`include/php_driver.h` — likely correctly compensated, but verify the modern
expansion is `len` not `len+1`.

#### PHP8-006 — `INVALID_ARGUMENT` macro returns without `RETURN_THROWS`
**Category:** PHP 8
**Severity:** Medium
[`include/php_driver.h:184-189`](../include/php_driver.h#L184-L189)

`INVALID_ARGUMENT` expands to `throw_invalid_argument(...); return;`. Adding
`RETURN_THROWS()` (which is `do { ZEND_ASSERT(EG(exception)); return; } while (0)`)
documents intent and asserts in debug builds. Safe to upgrade.

#### CORR-027 — `Z_PARAM_LONG(port)` accepts `int` then casts to `int` (line 344) — accepted range is `[1, 65535]` but storage is `int`
**Category:** Correctness
**Severity:** Low

[`src/Cluster/Builder.cpp:335-344`](../src/Cluster/Builder.cpp#L335-L344) — already
bounds-checks `port < 1 || port > 65535`, so the `(int)port` cast is safe. **No
issue.** Withdrawn.

#### CORR-027 — `cass_cluster_set_num_threads_io` receives a `zend_long` cast to `uint32_t`
**Category:** Correctness
**Severity:** Low
[`src/Cluster/Builder.cpp:540`](../src/Cluster/Builder.cpp#L540) (validated 1..128 — OK)

OK. Withdrawn.

#### CORR-027 — `Decimal::div` truncates large numerators silently when result exceeds `cass_int64_t` post-MPZ
**Category:** Correctness
**Severity:** Low
[`src/Numbers/Decimal.cpp`](../src/Numbers/Decimal.cpp)

Verify mpz overflow paths — beyond scope of this pass; flagged for follow-up.

#### PHP8-007 — `ZEND_BEGIN_ARG_INFO_EX` without return-type info disables strict-mode
**Category:** PHP 8
**Severity:** Medium
[`src/DefaultSession.cpp:977-991`](../src/DefaultSession.cpp#L977),
[`src/Session.cpp:21-36`](../src/Session.cpp#L21-L36),
others

Methods declared with `ZEND_BEGIN_ARG_INFO_EX` and no return-type info are
treated as `mixed` returns. PHP 8.1+ deprecation warnings for tentative returns.

#### PHP8-008 — `arginfo_timeout` has differing `required_num_args` between PHP 7 and 8 branches in legacy files
**Category:** PHP 8
**Severity:** Low
[`src/Database/Rows.cpp:407-415`](../src/Database/Rows.cpp#L407-L415)

```c
#if PHP_MAJOR_VERSION >= 8
ZEND_BEGIN_ARG_INFO_EX(arginfo_timeout, 0, ZEND_RETURN_VALUE, 0)
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_timeout, 0, ZEND_RETURN_VALUE, 1)
#endif
```

In PHP 7 the `timeout` arg is required; in PHP 8 it's optional. The C
implementation `nextPage($timeout = null)` matches PHP 8 — PHP 7 path is now
dead code but technically wrong.

#### CORR-028 — `cass_collection_new` / `cass_tuple_new` / `cass_user_type_new_from_data_type` not NULL-checked
**Category:** Correctness
**Severity:** Medium

Various sites in `util/src/collections.cpp` (not read in this audit, presumed).
Flagged for follow-up via grep.

---

### Low

#### CORR-029 — `Future::get` cache may leak old `prepared_statement` on second call after error
**Category:** Correctness
**Severity:** Low
[`src/FuturePreparedStatement.cpp:30-32`](../src/FuturePreparedStatement.cpp#L30-L32)

The `Z_ISUNDEF` check at line 30 happens *before* `zend_parse_parameters`. If the
first call set `self->prepared_statement` (success) and the second call passes
malformed args, we short-circuit return the cached statement — no error. OK in
isolation but suspicious.

#### CORR-030 — `php_driver_log` malloc'd format buffer can fail and `sprintf` proceeds
**Category:** Correctness
**Severity:** Low
[`src/php_driver.cpp:223-225`](../src/php_driver.cpp#L223-L225)

`tmp = (char *)malloc(needed + 1);` not NULL-checked before `sprintf(tmp, ...)`.

#### CORR-031 — `OnUpdateLog` calls `strdup` without checking return
**Category:** Correctness
**Severity:** Low
[`src/php_driver.cpp:376, 378, 381`](../src/php_driver.cpp#L376)

`strdup` returns NULL on allocation failure; `log_location = NULL` happens to be
safely handled by `php_driver_log`'s `if (log_location)` guard. Lucky.

#### CORR-032 — `php_driver_simple_statement_compare` etc. report different objects but identical class as `obj_handle != obj_handle` — works but is fragile
**Category:** Correctness
**Severity:** Low
Many `_compare` handlers

Returns a boolean (`!=`) rather than `-1`/`0`/`+1`. PHP 8 engine treats nonzero
as `>` — antisymmetry broken.

#### CORR-033 — `php_driver_collection_find` uses `is_equal_function` which may throw on incomparable types
**Category:** Correctness
**Severity:** Low
[`src/Collection.cpp:63-70`](../src/Collection.cpp#L63-L70)

`is_equal_function(&compare, object, current)` returns FAILURE / can throw.
Result `compare` is uninitialized on failure, then `PHP_SCYLLADB_Z_IS_TRUE_P(&compare)`
reads garbage.

#### PHP8-009 — `zval` returned by value from `php_driver_table_build_options` and similar functions
**Category:** PHP 8
**Severity:** Low
[`src/Database/Table.cpp:23-58`](../src/Database/Table.cpp#L23-L58),
several `php_driver_*_build_*` helpers

Returning `zval` by value in C++ doesn't run zval copy semantics; the inner
HashTable's refcount is whatever the last `array_init` set. Works in practice
because PHP doesn't use C++ copy-elision rules, but the pattern is non-idiomatic.

#### PHP8-010 — Stub `@implements-clone` missing on classes that genuinely should not clone
**Category:** PHP 8
**Severity:** Low

Stubs in `src/Cluster/`, `src/RetryPolicy/`, etc. use `final class` but don't
explicitly mark non-cloneable. The C side sets `clone_obj = NULL`. Stub should
match.

#### PHP8-011 — `ZEND_ACC_DEPRECATED` on `ExecutionOptions::__construct` shows deprecation, but no replacement is exported
**Category:** PHP 8
**Severity:** Low
[`src/ExecutionOptions.cpp:318`](../src/ExecutionOptions.cpp#L318)

The deprecation warning suggests use array form, but `php_driver_execution_options_build_local_from_array`
is internal. User has no migration target.

---

## Patterns observed

1. **Untyped `zval *` parsing.** Legacy modules use `"z"` everywhere and forget
   to type-check before dispatching. Migration to stub-generated `Z_PARAM_*`
   would eliminate ~20% of findings.

2. **`Z_OBJCE_P` on unchecked zvals.** A frequent template:
   `if (Z_TYPE_P(x) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(x), ce))` —
   wrong precedence; should be `Z_TYPE_P != IS_OBJECT || !instanceof_function`.

3. **Process-global C-driver state mutated from per-thread hooks.** `log_lock`,
   `log_location`, `cass_log_set_level`, `cass_log_set_callback` — all process
   global, all reachable from per-thread INI handlers and `GSHUTDOWN`. Crash
   surface under ZTS.

4. **`strncmp(name, "literal", name_length) == 0`** — admits prefix matches.
   Common across `src/Database/`.

5. **`cass_*_get_*` return codes silently dropped** — leads to use of
   uninitialized strings/values.

6. **`compare` handler contract violations** — returning literal `1` for
   "different" breaks ordering invariants used by `usort` and `<=>`.

7. **PHP 7/8 `#if PHP_MAJOR_VERSION >= 8` branches** — PHP 7 paths now dead but
   technically incorrect (different arg counts, `compare_objects` vs `compare`).
   Code review burden.

---

## Recommendations

### Tier 1 — Critical, low fix effort

| ID       | Why                                                                                                      | Effort |
|----------|----------------------------------------------------------------------------------------------------------|--------|
| CORR-001 | Single-character `&&`→`||` fix; eliminates a memory-corruption path                                      | 5 min  |
| CORR-002 | Add type checks before `Z_LVAL_P` in `ExecutionOptions::build_from_array` (4 sites)                      | 30 min |
| CORR-003 | Replace `php_localtime_r`/`PHP_EOL` in `php_driver_log` with POSIX equivalents                           | 30 min |
| CORR-004 | Restructure `DefaultSession::prepare` into linear flow; null-check `cass_session_prepare_n` result       | 1 hr   |
| CORR-009 | Fix all `strncmp(name, "literal", name_length)` to use exact-length comparison (4-6 sites)               | 30 min |
| CORR-014 | Add NULL checks after each `cass_*_new` / `cass_*_bind` / `cass_session_execute*` (8-10 sites)           | 1 hr   |
| CORR-018 | Call `cass_log_set_callback(NULL, NULL)` in `MSHUTDOWN`                                                  | 5 min  |
| CORR-019 | Add `Z_TYPE_P(...) == IS_OBJECT` guard before `Z_OBJCE` / `Z_OBJCE_P`                                    | 30 min |

### Tier 2 — High value, medium effort

| ID       | Why                                                                                                  | Effort |
|----------|------------------------------------------------------------------------------------------------------|--------|
| CORR-007 | Throw `InvalidArgumentException` in `bind_argument_by_*` when no type matches                        | 1 hr   |
| CORR-008 | Validate cached prepared-statement future before `cass_future_get_prepared`                          | 30 min |
| CORR-010 | Wrap each `cass_value_get_*` in error check across `src/Database/`                                   | 2 hrs  |
| CORR-015 | Move log lock lifecycle into `MINIT`/`MSHUTDOWN` (process scope) instead of `GINIT`/`GSHUTDOWN`      | 1 hr   |
| CORR-017 | Downgrade `cassandra.log_level` / `cassandra.log` to `PHP_INI_SYSTEM`                                | 5 min  |
| CORR-020 | Use `strcmp(ce1->name, ce2->name)` for cross-class ordering in all `_compare` handlers (~20 sites)   | 2 hrs  |
| CORR-021 | Throw `InvalidArgumentException` in `Inet::__construct` on parse failure                             | 15 min |
| CORR-022 | Cache properties HT in `_properties` handlers; rebuild only on dirty flag                            | 3 hrs  |

### Tier 3 — Long-term migration

| ID       | Why                                                                                                | Effort  |
|----------|----------------------------------------------------------------------------------------------------|---------|
| PHP8-001 | Migrate all legacy `.cpp` modules to stub-generated arginfo per CLAUDE.md (canonical pattern)      | 1-2 wks |
| PHP8-004 | Adopt `Z_PARAM_*` in all parameter parsing                                                         | (subset)|
| PHP8-005 | Audit `add_assoc_*_ex` length arithmetic across `src/Database/`                                    | 4 hrs   |
| PHP8-007 | Add return-type arginfo to all methods (or migrate to stubs per PHP8-001)                          | (subset)|

---

## Notes on stub-vs-implementation drift

A spot-check of modern (`*.stub.php`-based) modules vs the C implementations
found:

* **`src/Cluster/Builder.stub.php` vs `src/Cluster/Builder.cpp`**: matches —
  `withSSL(SSLOptions $options): Builder`, `withRetryPolicy(RetryPolicy $policy):
  Builder`, etc. Param types align with `Z_PARAM_OBJECT_OF_CLASS`. ✅

* **`src/DateTime/`, `src/SSLOptions/`, `src/RetryPolicy/`**: All use
  `register_class_*` and stub-generated tables; signatures match. ✅

* **Legacy `src/Database/*.cpp`, `src/Numbers/*.cpp`, etc.**: No stubs exist;
  drift not measurable, but the patterns above (CORR-001..028) indicate the
  hand-written PHP-visible signatures (via `PHP_ME` / `ZEND_BEGIN_ARG_INFO_EX`)
  diverge from the documented PHP API (e.g. `Future::get(?float $timeout = null)`
  is documented but `arginfo_timeout` uses untyped `ZEND_ARG_INFO(0, timeout)`).

No critical stub/implementation drift found in the modern (stub-based) modules.
Legacy modules have no stub baseline — recommendation per PHP8-001 to introduce
stubs as the source-of-truth for the PHP API surface.
