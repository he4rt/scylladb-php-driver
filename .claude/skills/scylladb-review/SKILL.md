---
name: scylladb-review
description: Review ScyllaDB/Cassandra C driver API usage in this extension — error handling, session/cluster lifecycle, type mapping, and async patterns. Use when touching any code that calls cass_* functions.
allowed-tools: Read Grep Glob
---

Review the code at `$ARGUMENTS` (or recently changed files if no argument) for correct use of the ScyllaDB/Cassandra C driver API.

For each finding: **file:line — issue — fix**.

## 1. Error Handling

- Every `cass_*` call that returns `CassError` must be checked — never silently dropped
- Use `ASSERT_SUCCESS(rc)` for calls where failure is a programming error (wrong type, invalid arg)
- Use explicit `if (rc != CASS_OK)` + `php_error_docref(NULL, E_WARNING, "%s", cass_error_desc(rc))` for recoverable errors (network, timeout)
- `CASS_ERROR_LIB_NOT_IMPLEMENTED` must be handled specially — some features are optional in older driver versions (see `Builder.cpp withHostnameResolution` as reference)
- Never use `cass_error_desc()` result after the `CassFuture` or object it came from is freed

## 2. Future Lifecycle

- Every `CassFuture *` must be freed with `cass_future_free()` — check all exit paths including early returns
- `cass_future_error_code()` checked before accessing result
- `cass_future_get_result()` only called when `cass_future_error_code() == CASS_OK`
- `CassResult *` from `cass_future_get_result()` freed with `cass_result_free()` — never leaked

## 3. Cluster / Session Lifecycle

- `cass_cluster_new()` always paired with `cass_cluster_free()` (or stored in persistent list)
- `cass_session_new()` always paired with `cass_session_free()`
- Persistent clusters stored in `EG(persistent_list)` with `ZVAL_NEW_PERSISTENT_RES` — not plain pointers
- Persistent cluster hash key uniquely encodes all configuration fields — adding a new `cass_cluster_set_*` call requires updating the hash key format in `Builder::build()`
- Session connect future (`cass_session_connect`) always freed even on error

## 4. String Ownership

- `cass_string_t` / `const char *` returned by driver functions are **not** owned by caller — do not free them
- Copy to `zend_string` with `zend_string_init(ptr, len, 0)` if the value outlives the driver object
- `ZSTR_VAL()` / `ZSTR_LEN()` used when passing `zend_string *` to `cass_*` functions expecting `const char *`

## 5. Type Mapping (CassValue / CassCollection)

- `cass_value_get_*` called only after `cass_value_type()` confirms the correct type
- `CASS_VALUE_TYPE_UNKNOWN` and `CASS_VALUE_TYPE_CUSTOM` handled — don't assume a known type
- `CassCollection *` freed with `cass_collection_free()` after `cass_statement_bind_collection()` (binding does not transfer ownership)
- `CassTuple *` / `CassUserType *` freed with their respective `_free()` functions

## 6. Statement Binding

- Index-based binding (`cass_statement_bind_*_by_idx`) preferred over name-based for performance
- `cass_statement_free()` called after `cass_session_execute()` — the future does not own the statement
- Prepared statement `CassPrepared *` freed with `cass_prepared_free()` when no longer needed

## 7. Iteration

- `CassIterator *` always freed with `cass_iterator_free()` on all paths
- `cass_iterator_next()` checked before calling `cass_iterator_get_*()` — never read past end
- Row iteration: use `cass_result_first_row()` for single-row results rather than creating an iterator

## 8. Thread Safety

- All `cass_*` functions are thread-safe for distinct objects — document any shared state explicitly
- `CassCluster` is NOT reused across sessions simultaneously — one cluster per session or use persistent list with separate session per request
- `PHP_DRIVER_G(persistent_clusters)` counter incremented/decremented atomically with the persistent list

## 9. ScyllaDB-specific (vs pure Cassandra)

- ScyllaDB shard-awareness: do not set `cass_cluster_set_num_threads_io` to a value lower than the ScyllaDB shard count on the target node — default of 1 is intentionally conservative
- `cass_cluster_set_use_schema(false)` disables schema metadata — appropriate for high-throughput write paths; flag it if enabled on a write-heavy builder
- Token-aware routing enabled by default — verify it is not disabled without reason

Report all findings. If a section is clean, say "✓ clean". End with: PASS, NEEDS FIXES (n issues), or CRITICAL (leak/crash risk).
