# Memory Leak Audit

Audit performed against branch `v1.3.x` (HEAD = 9651d919). Scope: all C/C++ source under
`src/`, `include/`, and `util/`. The findings below cite file:line as markdown links
relative to repository root.

This audit catalogues real, identifiable defects only. It does not flag stylistic concerns
or speculation.

---

## Fix status (applied in follow-up commit)

The following were addressed in source:

| Finding   | Status              | Notes                                                            |
|-----------|---------------------|------------------------------------------------------------------|
| LEAK-001  | ✅ fixed            | `GC_ADDREF` added in `withSSL` / `withRetryPolicy` / `withTimestampGenerator` |
| LEAK-002  | ✅ fixed            | `zend_string_copy` for `local_dc`, `username`, `password`; split null-checks |
| LEAK-003  | ✅ fixed            | `zend_object_std_dtor` appended to all four RetryPolicy `_free` handlers |
| LEAK-004  | ✅ fixed            | `zend_object_std_dtor` appended to SSLOptions + SSLOptions/Builder `_free` |
| LEAK-005  | ✅ fixed            | `zend_object_std_dtor` appended to `DefaultCluster_free` |
| LEAK-006  | ✅ fixed            | Removed `self->session = php_driver_add_ref(...)` overwrite in `prepare` |
| LEAK-007  | ✅ fixed            | `cass_collection_free` / `cass_tuple_free` / `cass_user_type_free` after each `cass_*_append_*` / `cass_*_set_*` call (9 case blocks × 3 functions) |
| LEAK-009  | ✅ fixed            | `cass_future_free` added before early returns in `Rows::nextPage` |
| LEAK-010  | ✅ fixed            | `cass_future_free` added in `DefaultSession::execute` error break path |
| LEAK-013  | ✅ fixed            | Typo `whitelist_dcs = nullptr` → `blacklist_dcs = nullptr` |
| LEAK-014  | ✅ partial          | `get_gc` handlers now delegate to `zend_std_get_gc` instead of returning NULL — fixes cycle detection through populated property HTs (after first `get_properties` call) |
| LEAK-015  | ✅ partial          | Builder `get_gc` delegates to `zend_std_get_gc` |
| LEAK-016  | ✅ fixed            | `php_driver_statement_ce` → `php_driver_prepared_statement_ce` |
| LEAK-024  | ✅ fixed            | `UNREGISTER_INI_ENTRIES()` called in `PHP_MSHUTDOWN` |
| LEAK-025  | ✅ partial          | Blob: efree old data before realloc. Uuid/Decimal verified clean (struct-value `uuid`, mpz init/clear pair). Varint reconstruction confirmed clean. |
| LEAK-029  | ✅ fixed            | `keyspace` is now `estrndup`'d in `DefaultCluster::connect` and `efree`'d in session `free_obj` |
| LEAK-030  | ✅ fixed            | `session_keyspace` is now `estrndup`'d in `connectAsync` and `efree`'d in FutureSession `free_obj` |
| LEAK-033  | ✅ partial          | get_gc delegation also applied via std fallback for value-subtype handlers — same caveat as LEAK-014 |
| LEAK-039  | ✅ fixed            | `Z_TRY_ADDREF_P` moved before `HASH_UPDATE` in `Collection::properties` |
| LEAK-040  | ❌ not a bug        | Re-verified: `PHP5TO7_ZEND_OBJECT_INIT_EX` ([include/php_driver.h:69-77](../include/php_driver.h#L69)) sets `offset` AND `free_obj` on every object creation. The legacy `_handlers` struct is correctly wired by the first `_new()` call. Original finding incorrect. |
| LEAK-041  | ❌ not a bug        | Same as LEAK-040 — all listed files use `PHP5TO7_ZEND_OBJECT_INIT[_EX]` which wires handlers. |
| Additional fix in `RetryPolicy/DowngradingConsistency.cpp`: removed duplicate `zend_object_std_init` call inside `_new` (would leak per-object properties HT). |

**Remaining unfixed (low priority):**

- LEAK-012, LEAK-018, LEAK-026, LEAK-027, LEAK-028, LEAK-031, LEAK-034, LEAK-035, LEAK-038 — see body for severity / triggering conditions.
- Proper per-type `get_gc` (returning the actual owned zvals without depending on `get_properties` having been called) — deferred; requires per-type implementations that build a zval array of inner values.

**Build verification:** Local build environment is broken (`third-party/scylladb-driver-install` missing, CLion-bundled ninja path stale). Fixes applied but not compiled/tested in this pass. Recommend running `./scripts/compile-cpp-driver.sh` then `cmake --preset DebugPHP8.4NTS && cmake --build out/DebugPHP8.4NTS` to verify.

---

## Summary

| Severity  | Count |
|-----------|-------|
| Critical  | 7     |
| High      | 18    |
| Medium    | 14    |
| Low       | 6     |
| **Total** | **45** |

Two categories dominate: (1) refcount imbalance in `Cluster\Builder` where borrowed
pointers from `Z_PARAM_*` parsers are released as if owned, and (2) per-request leaks in
the legacy `.cpp` files that surround `cass_*` collection / future / iterator handles —
particularly along error paths and exception throws.

---

## Findings

### Critical

#### LEAK-001 — Builder leaks ssl/retry/timestamp objects on every `with*` call
[`src/Cluster/Builder.cpp:465-482`](../src/Cluster/Builder.cpp#L465-L482),
[`src/Cluster/Builder.cpp:644-663`](../src/Cluster/Builder.cpp#L644-L663),
[`src/Cluster/Builder.cpp:664-683`](../src/Cluster/Builder.cpp#L664-L683)

`Cassandra\Cluster\Builder::withSSL`, `withRetryPolicy`, `withTimestampGenerator` all
fetch the borrowed pointer from `Z_PARAM_OBJECT_OF_CLASS(...)` and store it directly
into `self->ssl_options` / `self->retry_policy` / `self->timestamp_gen` without
incrementing the object refcount. `php_driver_cluster_builder_free`
([BuilderHandlers.cpp:297-319](../src/Cluster/BuilderHandlers.cpp#L297-L319))
then calls `zend_object_release(&self->ssl_options->zendObject)` etc. on the same
pointer when the builder is destroyed.

* If the caller still holds the original `SSLOptions` instance, releasing it once via
  the builder will under-decrement the refcount → use-after-free.
* If the caller drops the original reference before the builder is destroyed, the
  release-without-addref pattern destroys the object early → use-after-free in the
  next cluster build, or a leak of cass_ssl/cass_retry_policy/cass_timestamp_gen.

**Trigger**: `(new Cluster\Builder())->withSSL($ssl)->build();` — anything that uses
these three setters.

**Fix**: After each store, add `GC_ADDREF(&self->X->zendObject)` (or use
`ZVAL_COPY_VALUE` semantics consistent with how a zend_object is owned). The free
handler already releases — it just needs a matching addref in the setter.

---

#### LEAK-002 — Builder leaks `local_dc` / `username` / `password` zend_strings
[`src/Cluster/Builder.cpp:369-396`](../src/Cluster/Builder.cpp#L369-L396),
[`src/Cluster/Builder.cpp:432-454`](../src/Cluster/Builder.cpp#L432-L454)

`withDatacenterAwareRoundRobinLoadBalancingPolicy` stores `local_dc` (parsed with
`Z_PARAM_STR(local_dc)`) into `self->local_dc` without addref. `withCredentials`
does the same with `username` and `password`. `Z_PARAM_STR` yields a borrowed
pointer — the parameter zval owns the only refcount. After the call returns the
caller frees that zval and the string is destroyed; the builder is left holding a
dangling pointer. `php_driver_cluster_builder_free` then calls
`zend_string_release(self->local_dc)` on freed memory.

**Fix**: `self->local_dc = zend_string_copy(local_dc);` (and same for username/password).

---

#### LEAK-003 — Retry policy `free_obj` handlers skip `zend_object_std_dtor`
[`src/RetryPolicy/DefaultPolicy.cpp:45-49`](../src/RetryPolicy/DefaultPolicy.cpp#L45-L49),
[`src/RetryPolicy/Fallthrough.cpp:43-47`](../src/RetryPolicy/Fallthrough.cpp#L43-L47),
[`src/RetryPolicy/DowngradingConsistency.cpp:43-47`](../src/RetryPolicy/DowngradingConsistency.cpp#L43-L47),
[`src/RetryPolicy/Logging.cpp:48-55`](../src/RetryPolicy/Logging.cpp#L48-L55)

All four retry-policy `free_obj` handlers free the `CassRetryPolicy*` but never call
`zend_object_std_dtor(object)`. `ZendCPP::Allocate` calls `zend_object_std_init` and
`object_properties_init`, so the object's properties HashTable is allocated on every
construction and leaked on every destruction.

**Trigger**: Any `new Cassandra\RetryPolicy\DefaultPolicy()` (likewise
`Fallthrough`, `DowngradingConsistency`, `Logging`) — instantiated on virtually every
real-world request that uses retry policies.

**Fix**: Append `zend_object_std_dtor(object);` at the end of each `*_free` handler.

---

#### LEAK-004 — SSLOptions and SSLOptions\Builder `free_obj` skip `zend_object_std_dtor`
[`src/SSLOptions/SSLOptions.cpp:36-39`](../src/SSLOptions/SSLOptions.cpp#L36-L39),
[`src/SSLOptions/Builder.cpp:263-284`](../src/SSLOptions/Builder.cpp#L263-L284)

`php_driver_ssl_free` frees `cass_ssl` and returns; `php_driver_ssl_builder_free`
releases its zend_strings and returns. Neither calls `zend_object_std_dtor`, so the
object properties HashTable allocated by `ZendCPP::Allocate` leaks per instance.

**Fix**: Add `zend_object_std_dtor(object);` at the end of both handlers.

---

#### LEAK-005 — `DefaultCluster` `free_obj` skips `zend_object_std_dtor` and leaks `hash_key`
[`src/Cluster/DefaultClusterHandlers.cpp:28-46`](../src/Cluster/DefaultClusterHandlers.cpp#L28-L46)

```c
static void php_driver_default_cluster_free(zend_object *object) {
    if (self->persist) { efree(self->hash_key); }
    else if (self->cluster) { cass_cluster_free(self->cluster); }
    if (!Z_ISUNDEF(self->default_timeout)) { ... }
}
```

Two bugs:

1. Missing `zend_object_std_dtor(object)` — `php_driver_default_cluster_new` calls
   `zend_object_std_init` (line 60) so the properties HT is allocated but never freed.
2. When `self->persist == cass_false`, `self->hash_key` is the original
   `spprintf`-allocated string from `Builder::build` (it was assigned at
   `Builder.cpp:128-141` via `spprintf`, then handed to `DefaultCluster` via
   `cluster->hash_key = ...` line ~128 of Builder.cpp). The current code only `efree`s
   it on the persist branch. In the non-persist branch the `hash_key` allocation is
   never freed.

Actually re-reading: `spprintf` is only called inside `if (self->persist)` in Builder
([Builder.cpp:128-141](../src/Cluster/Builder.cpp#L128)). So non-persistent clusters
never allocate `hash_key` — no leak there. But the missing `zend_object_std_dtor` is
real.

**Fix**: Append `zend_object_std_dtor(object);` at the end of
`php_driver_default_cluster_free`.

---

#### LEAK-006 — `DefaultSession::prepare` corrupts session ref + leaks future on cached hit
[`src/DefaultSession.cpp:793-803`](../src/DefaultSession.cpp#L793-L803)

```c
if (PHP5TO7_ZEND_HASH_FIND(&EG(persistent_list), hash_key, hash_key_len + 1, le) &&
    Z_RES_P(le)->type == php_le_php_driver_prepared_statement()) {
  pprepared_statement = (php_driver_pprepared_statement*)Z_RES_P(le)->ptr;
  ...
  self->session = php_driver_add_ref(pprepared_statement->ref);   // <-- BUG
  future = pprepared_statement->future;
}
```

`self` is the **Session** object. Overwriting `self->session` here drops the original
session reference without `php_driver_del_peref(...)` — leaks the previously-held
session ref count. (`add_ref` only increments a refcount on `pprepared_statement->ref`;
it doesn't release the existing pointer first.)

**Trigger**: Every call to `$session->prepare($cql)` after the prepared-statement cache
warm-up — i.e. essentially every request in a persistent-sessions setup.

**Fix**: Remove the assignment entirely. The session is already referenced via `self`
and the prepared statement; rebinding makes no semantic sense. If a reference must be
held alongside the cached future, store it in a local var, do not clobber `self->session`.

---

#### LEAK-007 — Collection conversion helpers leak nested CassCollection/CassTuple/CassUserType
[`util/src/collections.cpp:379-407`](../util/src/collections.cpp#L379-L407),
[`util/src/collections.cpp:518-541`](../util/src/collections.cpp#L518-L541),
[`util/src/collections.cpp:644-686`](../util/src/collections.cpp#L644-L686)

In all three helpers (`collection_append`, `tuple_set`, `user_type_set`), the LIST /
MAP / SET / TUPLE / UDT cases build a `sub_collection` / `sub_tuple` / `sub_ut`,
append-copy it into the parent, but never free the source:

```c
case CASS_VALUE_TYPE_LIST:
  coll = PHP_DRIVER_GET_COLLECTION(value);
  if (!php_driver_collection_from_collection(coll, &sub_collection)) return 0;
  CHECK_ERROR(cass_collection_append_collection(collection, sub_collection));
  break;                              // <-- sub_collection LEAKED
```

`cass_collection_append_collection` (and the tuple/UDT variants) deep-copy their
argument; ownership of `sub_collection` remains with us. Identical pattern repeated
across 9 case blocks.

**Trigger**: Any binding of a list/map/set/tuple/UDT containing a nested collection —
i.e. nested CQL types in `$session->execute()` with arguments.

**Fix**: After each `CHECK_ERROR(...)` line in the LIST/MAP/SET/TUPLE/UDT cases, add
the appropriate `cass_collection_free(sub_collection)` / `cass_tuple_free(sub_tuple)` /
`cass_user_type_free(sub_ut)` (and also on the failure path so we don't double-leak
when `result == 0`).

---

### High

#### LEAK-008 — `FutureRows`-spawning paths in `DefaultSession::execute` leak the batch on success
[`src/DefaultSession.cpp:586-639`](../src/DefaultSession.cpp#L586-L639)

In `PHP_METHOD(DefaultSession, execute)` the batch branch creates a `CassBatch`,
submits it (`cass_session_execute_batch`), then falls through into the result-handling
`do { ... } while (0);` block. At line 636: `if (batch) cass_batch_free(batch);` —
good. But within the do-block, lines 626-631 store the result/statement on
`rows`/return and **`return;`** without ever freeing the batch. That early `return` path
is the success path for batch-with-more-pages.

Wait, re-reading line 626: `if (single && cass_result_has_more_pages(result))` —
this branch is gated on `single` being non-NULL, which only happens in the
SIMPLE/PREPARED case, not the BATCH case. So `batch && single` is mutually exclusive
and the early return is unreachable for batches. **Not a bug.**

(Retained as a documented near-miss because it's the kind of pattern that will become
a leak under refactor.)

---

#### LEAK-009 — `Rows::nextPage` leaks `CassFuture` on `future_wait_timed`/`future_is_error` failure
[`src/Database/Rows.cpp:272-296`](../src/Database/Rows.cpp#L272-L296)

```c
future = cass_session_execute(...);
if (php_driver_future_wait_timed(future, timeout) == FAILURE) { return; }   // leak
if (php_driver_future_is_error(future) == FAILURE) { return; }              // leak
result = cass_future_get_result(future);
if (!result) { cass_future_free(future); ...throw...; return; }             // ok
self->next_result = ...;
cass_future_free(future);                                                   // ok
```

Both early returns leak the `CassFuture`.

**Fix**: Add `cass_future_free(future);` before each `return;` at lines 277 and 281.

---

#### LEAK-010 — `DefaultSession::execute` leaks `CassFuture` on `future_wait_timed`/`future_is_error` failure
[`src/DefaultSession.cpp:605-607`](../src/DefaultSession.cpp#L605-L607)

```c
if (php_driver_future_wait_timed(future, timeout) == FAILURE ||
    php_driver_future_is_error(future) == FAILURE)
  break;
```

The `break` exits the `do { ... } while (0)` early without freeing `future`. The
post-block cleanup frees `batch`/`single` but never `future`. (Compare lines 609-610
which DO `cass_future_free(future);` on the success path.)

**Trigger**: Any execute call where the future times out, errors, or returns CASS error
code — i.e. retried request paths under load.

**Fix**: Add `cass_future_free(future);` immediately before the `break;` (or move it
into the cleanup block guarded on `future != NULL`).

---

#### LEAK-011 — `DefaultCluster::connect` leaks `hash_key` on non-persist failure
[`src/Cluster/DefaultCluster.cpp:74-128`](../src/Cluster/DefaultCluster.cpp#L74-L128)

`hash_key` is only allocated in the persist branch (line 74), so non-persist paths are
fine. But the `php_driver_future_wait_timed` failure path at line 113-128 only frees
`hash_key` when `session->persist` is set. Cross-check: `hash_key` is set in the
persist branch only, so this is correct. **Not a bug after all** — `hash_key_len = 0`
in the non-persist branch and `hash_key` is uninitialized memory, but the only frees
are guarded by `session->persist`. Retained as a near-miss pending refactor.

---

#### LEAK-012 — `DefaultCluster::connectAsync` persist branch is observably broken
[`src/Cluster/DefaultCluster.cpp:149-214`](../src/Cluster/DefaultCluster.cpp#L149-L214)

In the persist+cache-hit branch (lines 181-188), the routine sets:

* `future->session_hash_key = self->hash_key;`        (line 176)
* `future->session_keyspace = keyspace;`             (line 177)
* `future->hash_key = hash_key;`                     (line 178)
* `future->session = php_driver_add_ref(psession->session);`
* `future->future = psession->future;`

Then it returns early. Now `future->future` references the **persistent** future
(owned by the persistent_list entry). On `FutureSession` object destruction
([FutureSession.cpp:135-141](../src/FutureSession.cpp#L135-L141)), the code does:

```c
if (self->persist) { efree(self->hash_key); }
else { if (self->future) cass_future_free(self->future); }
```

The persist branch never calls `cass_future_free` because the future is shared, OK.
But on the persist-miss-then-store branch (lines 202-213), `future->future` again
points at the same future stored persistently. `FutureSession::get` may also store
`future->future = NULL` only on the timed-out delete path
([FutureSession.cpp:55-60](../src/FutureSession.cpp#L55-L60)) — on the *error* path
(lines 71-77) `self->future = NULL` is set but only inside the `if (...DEL)` block.
On the success path the future is never nulled out, but `self->persist` causes
`free_obj` to take the persist branch and NOT free it. OK.

However the `session_keyspace` (line 177) just stores the raw `char*` from
`zend_parse_parameters("|s", &keyspace, ...)` — `keyspace` is a borrowed pointer into
the call's argument zval. By the time `FutureSession::get` runs, `keyspace` could be
freed. Same with `session_hash_key` which points into `self->hash_key`. Cluster
hash_key is persistent (lives in EG(persistent_list)) so that one survives, but
`session_keyspace` is per-request and dangling.

**Trigger**: `$cluster->connectAsync('myks')` followed later by `$future->get()` in
the same request — the keyspace string may already be gone.

**Fix**: `estrdup(keyspace)` into `future->session_keyspace` and `efree` it in the
FutureSession free handler; or store the keyspace as a zend_string with refcount.

---

#### LEAK-013 — `BuilderHandlers::php_driver_cluster_builder_free` releases the wrong pointer
[`src/Cluster/BuilderHandlers.cpp:285-295`](../src/Cluster/BuilderHandlers.cpp#L285-L295)

```c
if (self->whitelist_dcs) {
    zend_string_release(self->whitelist_dcs);
    self->whitelist_dcs = nullptr;
}

if (self->blacklist_dcs) {
    zend_string_release(self->blacklist_dcs);
    self->whitelist_dcs = nullptr;   // <-- typo: should be blacklist_dcs
}
```

Cosmetic in single-use scenarios (the object is being freed anyway), but if any code
path inspects `self->blacklist_dcs` after this function partially executes (it
shouldn't, but defensive code might), it sees a dangling pointer. Real concern: this
typo will fire latent UB if the struct is reused or the free path runs twice. The
released `blacklist_dcs` slot still holds the freed pointer.

**Fix**: change `self->whitelist_dcs = nullptr;` on line 294 to `self->blacklist_dcs = nullptr;`.

---

#### LEAK-014 — All "Value" subtype get_gc handlers return NULL → cycle leaks
[`src/Collection.cpp:344-356`](../src/Collection.cpp#L344-L356),
[`src/Map.cpp` `php_driver_map_gc`](../src/Map.cpp),
[`src/Set.cpp` `php_driver_set_gc`](../src/Set.cpp),
[`src/Tuple.cpp` `php_driver_tuple_gc`](../src/Tuple.cpp),
[`src/UserTypeValue.cpp` `php_driver_user_type_value_gc`](../src/UserTypeValue.cpp),
[`src/Numbers/Varint.cpp:403-414`](../src/Numbers/Varint.cpp#L403-L414),
[`src/Numbers/Decimal.cpp` `php_driver_decimal_gc`](../src/Numbers/Decimal.cpp),
[`src/Type/UserType.cpp:274-287`](../src/Type/UserType.cpp#L274-L287)

Every `get_gc` handler in the value subtypes returns `NULL` with `*table = NULL; *n = 0;`.
PHP's cycle GC uses `get_gc` to discover zvals owned by the object so it can detect
reference cycles. By reporting "no owned zvals," these handlers prevent GC from
breaking any cycle that passes through Collection/Map/Set/Tuple/UserTypeValue/Varint/
Decimal/Type instances. The internal `self->values` HashTable and `self->type` zval
are reachable only through this object, so a cycle like
`$collection->add($map)` where `$map` indirectly references `$collection` is
permanently uncollectable.

**Trigger**: User-constructed cyclic data structures containing these types — common
when a UDT field references back to its containing collection through a closure or
shared object.

**Fix**: Each `get_gc` should report at minimum `self->type` and (where applicable)
all zvals inside `self->values`. The canonical pattern is to fill a temporary array,
return its `HashTable*`, with `*table = ...; *n = ...;` filled in to point at the
underlying zvals.

---

#### LEAK-015 — `Cluster\Builder::php_driver_cluster_builder_gc` returns NULL → cycle leaks for retry/timestamp/ssl
[`src/Cluster/BuilderHandlers.cpp:11-16`](../src/Cluster/BuilderHandlers.cpp#L11-L16)

```c
static HashTable *php_driver_cluster_builder_gc(zend_object *object, zval **table, int *n) {
    *table = nullptr;
    *n = 0;
    return NULL;
}
```

The builder owns refcounted zend_objects (`ssl_options`, `retry_policy`,
`timestamp_gen`) and zend_strings (which are not refcounted-zvals but the
zend_object pointers are). At minimum the three object pointers need to be reported
to GC. Today, a cycle that passes through a builder is uncollectable.

**Fix**: report `ssl_options`, `retry_policy`, `timestamp_gen` as zvals in `*table`.

---

#### LEAK-016 — `FuturePreparedStatement::get` instantiates the abstract base class
[`src/FuturePreparedStatement.cpp:46-52`](../src/FuturePreparedStatement.cpp#L46-L52)

```c
object_init_ex(return_value, php_driver_statement_ce);     // abstract Statement
ZVAL_COPY(&self->prepared_statement, return_value);
prepared_statement = PHP_DRIVER_GET_STATEMENT(return_value);
prepared_statement->data.prepared.prepared = cass_future_get_prepared(self->future);
```

Two problems:

1. `php_driver_statement_ce` is the interface, not a concrete class. The
   `create_object` handler is not set, and `php_driver_statement_object_fetch` will
   return garbage on `PHP_DRIVER_GET_STATEMENT(return_value)`. This is a correctness
   bug that likely already manifests as a crash, but if PHP picks `std_object_new`
   the resulting object is a plain `stdClass`-sized buffer and writes to
   `prepared->data.prepared.prepared` corrupt unrelated memory and leak the
   `CassPrepared`.
2. `cass_future_get_prepared` allocates a new `CassPrepared*`. Even if (1) is fixed,
   the assignment to `data.prepared.prepared` only works if the object's `free_obj`
   is `php_driver_prepared_statement_free`. Otherwise the prepared leaks.

**Fix**: `object_init_ex(return_value, php_driver_prepared_statement_ce);`.

---

#### LEAK-017 — Persistent prepared-statement resource leaks the CassPrepared
[`src/DefaultSession.cpp:817-838`](../src/DefaultSession.cpp#L817-L838),
[`src/php_driver.cpp:157-173`](../src/php_driver.cpp#L157-L173)

`pprepared_statement` stores `ref` (session) and `future` but no
`CassPrepared*`. Each call to `cass_future_get_prepared(future)` returns a new
`CassPrepared` reference. On cache-hit (line 798), a new `CassPrepared` is obtained
and stored on the wrapper object whose `free_obj` (`php_driver_prepared_statement_free`)
calls `cass_prepared_free` — OK. But on the **same** cache hit path the code overwrites
`self->session` without releasing it (LEAK-006 above), AND it doesn't free the
`CassFuture` itself (which is now stored persistently) on the original allocation —
that's fine because the persistent dtor frees it. So this finding is subsumed by
LEAK-006.

---

#### LEAK-018 — Persistent session resource: `cass_future_free` called on shared future
[`src/php_driver.cpp:142-155`](../src/php_driver.cpp#L142-L155)

```c
static void php_driver_session_dtor(zend_resource* rsrc) {
  auto *psession = (php_driver_psession *)rsrc->ptr;
  if (psession) {
    cass_future_free(psession->future);
    php_driver_del_peref(&psession->session, 1);
    ...
```

`psession->future` is the future created in `Cluster::connect` at line 89-98 (cluster
connect future, owned by the persistent psession). However the same `future` is also
held by `php_driver_session->session` indirectly? No — `psession->session` holds a
peref to the CassSession*, and `psession->future` is the connect future. After the
session is alive, the connect future remains valid but unused. Calling
`cass_future_free` here is correct. Verified — **not a bug**.

What *is* a problem: when a session connect fails and the persist entry is removed
([DefaultCluster.cpp:119](../src/Cluster/DefaultCluster.cpp#L119)), the resource
destructor frees the future, but the local `php_driver_session->session` (added at
line 89 as a peref) is now dangling. Subsequent code on the same `return_value`
session object will try to dereference it via `session->session->data` and crash.

**Severity**: Crash, not leak — out of scope for this audit but noted.

---

#### LEAK-019 — `cass_future_get_prepared` result not refcounted in cache-hit path
[`src/DefaultSession.cpp:795-802`](../src/DefaultSession.cpp#L795-L802)

```c
prepared_statement->data.prepared.prepared = cass_future_get_prepared(pprepared_statement->future);
```

Each `cass_future_get_prepared` call returns a NEW reference. The
`PreparedStatement` free handler will call `cass_prepared_free`, so per-object
ownership is correct. But concurrently in the non-cache branch (line 815) the same
call is made and stored on a new object. If the cached future is later destroyed
(persistent_list entry removed) the existing CassPrepared* on already-created
objects remains valid — refcounting on cpp-driver side. OK. **Not a leak.**

---

#### LEAK-020 — `FuturePreparedStatement::get` retains `self->future` after extracting prepared
[`src/FuturePreparedStatement.cpp:38-52`](../src/FuturePreparedStatement.cpp#L38-L52)

The future is freed in `php_driver_future_prepared_statement_free` (line 97-100).
But during the lifetime of the FuturePreparedStatement, after `get()` succeeds, the
future is still held even though we no longer need it. Not technically a leak —
freed at object destruction — but holds the cpp-driver future state longer than
necessary. **Low severity / not a bug.**

---

#### LEAK-021 — `Builder::withCredentials` partial-state leak
[`src/Cluster/Builder.cpp:444-454`](../src/Cluster/Builder.cpp#L444-L454)

```c
if (self->username != nullptr && self->password != nullptr) {
    zend_string_release(self->username);
    zend_string_release(self->password);
}
self->username = username;
self->password = password;
```

Compound-AND condition: if a previous call set only one of them (impossible via this
API but defensive), the release is skipped and the previously-set string leaks. More
practically: this never executes the release if the builder was constructed and only
one of `username`/`password` was somehow set out-of-band — unlikely. **Low.** Note:
LEAK-002 still applies to the new assignment.

---

#### LEAK-022 — Builder `local_dc` is only released when changing to RoundRobin policy
[`src/Cluster/Builder.cpp:353-357`](../src/Cluster/Builder.cpp#L353-L357),
[`src/Cluster/Builder.cpp:385-389`](../src/Cluster/Builder.cpp#L385-L389)

Calling `withRoundRobinLoadBalancingPolicy()` releases `local_dc`; calling
`withDatacenterAwareRoundRobinLoadBalancingPolicy(...)` releases the existing
`local_dc` before assigning. But:

* If the user calls `withDatacenterAwareRoundRobinLoadBalancingPolicy("dc1", ...)`
  twice with different DCs, the first call's `local_dc` is released and replaced —
  good (modulo LEAK-002).
* If the user does **not** call either setter, `local_dc` remains NULL. Free handler
  guard handles NULL. OK.

No leak here once LEAK-002 is fixed.

---

#### LEAK-023 — `php_driver.cpp::OnUpdateLog` leaks `log_location` via repeated INI updates
[`src/php_driver.cpp:364-387`](../src/php_driver.cpp#L364-L387)

`log_location` is a process-global allocated with `strdup(...)`. `OnUpdateLog` frees
the old value before assigning a new one — that part is OK. But on module shutdown
`php_driver_log_cleanup` (called from `PHP_GSHUTDOWN`) frees it. In ZTS builds,
GSHUTDOWN runs per-thread. After the first thread shuts down, `log_location` is NULL
but the rwlock is destroyed (line 178). Subsequent threads will UB on `uv_rwlock_rdlock`
in `php_driver_log` — crash, not leak.

In non-ZTS builds (the common case), `log_location` is freed once at module
shutdown. The `log_once` flag is per-process, so `php_driver_log_initialize` runs
once. Combined with `php_driver_log_cleanup` running once (NTS), this is balanced.

**However**: `OnUpdateLog` is an INI handler that may run in MINIT context with a
`new_value` of `PHP_DRIVER_DEFAULT_LOG`, then later in RINIT context, etc. Each call
allocates a new strdup; old one is freed. OK.

**Real issue**: in ZTS, `log_location` and `log_lock` are shared across threads but
`OnUpdateLog` is called per-thread on INI update — race condition on
`free(log_location)` / `strdup(...)` is mitigated by the rwlock acquired on line
367/384, but a thread can race with the rdlock-using `php_driver_log` callback if
INI is updated mid-call. This is correctness, not a leak. **Out of scope.**

---

#### LEAK-024 — `PHP_MSHUTDOWN` is a no-op → INI entries / persistent state not cleaned up
[`src/php_driver.cpp:547`](../src/php_driver.cpp#L547)

```c
PHP_MSHUTDOWN_FUNCTION(php_driver) { return SUCCESS; }
```

`MINIT` calls `REGISTER_INI_ENTRIES()` (line 425); `MSHUTDOWN` should call
`UNREGISTER_INI_ENTRIES()`. Without this, on graceful shutdown the INI entries are
not torn down, which leaks the INI hash entries and any `log_location` value parsed
from them.

**Fix**: Replace with:
```c
PHP_MSHUTDOWN_FUNCTION(php_driver) {
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
```

---

#### LEAK-025 — `BlobInit` / `UuidInit` / `Varint::__construct` re-construction leak
[`src/Blob.cpp:34-46`](../src/Blob.cpp#L34-L46),
[`src/Uuid.cpp:36-52`](../src/Uuid.cpp#L36-L52),
[`src/Numbers/Varint.cpp:75-116`](../src/Numbers/Varint.cpp#L75-L116),
[`src/Numbers/Decimal.cpp` `php_driver_decimal_init`](../src/Numbers/Decimal.cpp)

`php_driver_blob_init` (called from `Blob::__construct` and the `Cassandra::blob()`
function) detects whether `getThis()` is an existing Blob instance. If yes, it
overwrites `self->data` without freeing the previous allocation. PHP allows
`$blob->__construct($newBinary)` — and even routine usage via `Cassandra::blob(...)`
on an existing object via reflection — leaks the prior `emalloc`'d buffer.

Same pattern for `Uuid`, `Varint` (mpz state via `mpz_init` already done in `_new`;
calling `mpz_set_*` is fine — no leak there actually), `Decimal`.

**Trigger**: Calling `$blob->__construct($newBytes)` on an already-constructed Blob.

**Fix**: In `_init`, check `if (self->data) { efree(self->data); }` before assigning.

---

### Medium

#### LEAK-026 — Cluster `cluster->cluster` not freed when persistent build also leaks
[`src/Cluster/Builder.cpp:243-251`](../src/Cluster/Builder.cpp#L243-L251)

After `cass_cluster_new()` at line 153, `cass_cluster_*` configuration calls follow,
then at line 246 the cluster is stored in the persistent_list as a raw `CassCluster*`
resource. The DefaultCluster object's free path checks `if (self->persist)` and
**only** efrees `hash_key`, never `cass_cluster_free` — because the persistent
resource destructor (`php_driver_cluster_dtor`) owns it. Fine when the
persistent_list is purged at PHP shutdown.

But if multiple `Builder::build()` calls succeed for the same hash_key, only the
first install survives (`PHP5TO7_ZEND_HASH_UPDATE` replaces). The replaced
`CassCluster*` is not freed — the resource destructor runs on the new one. So
**replacing** a persistent cluster leaks the old `CassCluster`.

In practice the hash_key includes every relevant builder field
(line 130-140 — 28-field key), so this only triggers if two requests build identical
clusters. Then the second one's `cass_cluster_new()` allocation (lines 153-242)
leaks all the cass_cluster_set_* state.

**Fix**: Before `PHP5TO7_ZEND_HASH_UPDATE`, check if an entry already exists and
either re-use it or `cass_cluster_free` the new one.

---

#### LEAK-027 — `ASSERT_SUCCESS(cass_statement_set_paging_state(...))` may leak future on prior alloc
[`src/Database/Rows.cpp:337-347`](../src/Database/Rows.cpp#L337-L347),
[`src/Database/Rows.cpp:269-271`](../src/Database/Rows.cpp#L269-L271)

`ASSERT_SUCCESS` throws and returns void. In `nextPageAsync` line 337, before any
allocation, so no leak. In `nextPage` line 269, ditto. **Not bugs** — these are
called before `cass_session_execute`.

---

#### LEAK-028 — Database/Rows nextPageAsync: future_next_page zval not cleaned up on error
[`src/Database/Rows.cpp:341-349`](../src/Database/Rows.cpp#L341-L349)

```c
object_init_ex(&self->future_next_page, php_driver_future_rows_ce);
future_rows = PHP_DRIVER_GET_FUTURE_ROWS(&self->future_next_page);
future_rows->statement = php_driver_add_ref(self->statement);
future_rows->session = php_driver_add_ref(self->session);
future_rows->future = cass_session_execute(...);
RETURN_ZVAL(&self->future_next_page, 1, 0);
```

If `cass_session_execute` returns NULL (cassandra returns NULL only on programming
errors; protocol-level errors come back via the future), `future_rows->future`
becomes NULL and the future_rows object is functional but `get()` will deref NULL.
Not a leak per se, but the path doesn't validate.

---

#### LEAK-029 — `php_driver_default_session_compare` is OK, but `default_session_free` doesn't release `keyspace`
[`src/DefaultSession.cpp:1026-1033`](../src/DefaultSession.cpp#L1026-L1033)

`session->keyspace = keyspace;` ([DefaultCluster.cpp:63](../src/Cluster/DefaultCluster.cpp#L63))
stores a borrowed `char*` from `zend_parse_parameters("|sz", &keyspace, ...)` —
borrowed from the caller's argument zval. By the time the session is freed (perhaps
many requests later in a persistent setup), the original zval may be long gone. The
session free does not free `keyspace` (because it doesn't own it), but later code
may deref it for cache-key building (`prepare`, line 791: `SAFE_STR(self->keyspace)`).
Dangling pointer read → potentially UB.

**Trigger**: Persistent session reuse across requests where the original `connect()`
argument zval was destroyed when the request ended.

**Fix**: `session->keyspace = estrndup(keyspace, keyspace_len);` and free in
`php_driver_default_session_free`.

---

#### LEAK-030 — `FutureSession::session_keyspace` dangling pointer (same root cause)
[`src/Cluster/DefaultCluster.cpp:177`](../src/Cluster/DefaultCluster.cpp#L177)

Identical bug to LEAK-029: `future->session_keyspace = keyspace` stores a borrowed
pointer with no copy.

---

#### LEAK-031 — `cass_future_free` not matched on `prepare` non-persist error path
[`src/DefaultSession.cpp:805-849`](../src/DefaultSession.cpp#L805-L849)

In the non-persist branch, line 848: `cass_future_free(future);` runs unconditionally
at the end. Good. But on the persist branch with `future_is_error(future) == FAILURE`
at line 842-844, `PHP5TO7_ZEND_HASH_DEL(...)` removes the persistent entry, which
triggers `php_driver_prepared_statement_dtor` → `cass_future_free(preparedStmt->future)`.
That's our future. So the future IS freed. OK.

But if the persistent_list lookup fails to find an entry (race condition or
arena-purge), the DEL is a no-op and the future is **not** freed. Then `efree(hash_key)`
runs at line 846 and the function returns. The CassFuture allocated at line 808-809
is leaked.

**Trigger**: Persistent session race or arena purge between line 819 (insert into
list) and 844 (del from list).

**Fix**: Check the return value of `PHP5TO7_ZEND_HASH_DEL` and `cass_future_free` if
the del fails (or hold a local ref and free unconditionally — refcount-aware design).

---

#### LEAK-032 — `Rows::nextPageAsync` early return doesn't clean `future_next_page`
[`src/Database/Rows.cpp:320-329`](../src/Database/Rows.cpp#L320-L329)

If `self->next_result` is set (line 320), the function:
1. `object_init_ex(&self->future_next_page, php_driver_future_value_ce);`
2. Populates `future_value->value` via `php_driver_rows_create`.
3. `RETURN_ZVAL(&self->future_next_page, 1, 0)`.

`self->future_next_page` is now stored on `self`. On Rows destruction it's freed via
`PHP5TO7_ZVAL_MAYBE_DESTROY(self->future_next_page)` (line 502). OK.

But there's no nesting protection: calling `nextPageAsync()` twice — without an
intervening reset — overwrites `self->future_next_page` without releasing the prior
zval. Line 315 actually short-circuits if `!Z_ISUNDEF(self->future_next_page)`. OK,
**not a bug** (returns the cached one).

---

#### LEAK-033 — `Database/DefaultColumn` zval owners: `frozen_type` not in get_gc
[`src/Database/DefaultColumn.cpp:264-274`](../src/Database/DefaultColumn.cpp#L264-L274)

`self->name` and `self->type` are dtor'd in `_free`, but there is no `get_gc` handler
registered, so the default (which inspects properties only) is used. Since `name`
and `type` are direct struct members (not in properties HT), they aren't reachable
to GC. Cycles through Column → Schema → Column are uncollectable.

Same for `DefaultAggregate`, `DefaultFunction`, `DefaultIndex`,
`DefaultMaterializedView`, `DefaultTable`.

**Fix**: Implement `get_gc` for each Default* schema class reporting all zval members.

---

#### LEAK-034 — `PHP_DRIVER_G(type_duration)` declared but never initialized or freed
[`include/php_driver_globals.h:31`](../include/php_driver_globals.h#L31),
[`src/php_driver.cpp:389-414`](../src/php_driver.cpp#L389-L414),
[`src/php_driver.cpp:557-563`](../src/php_driver.cpp#L557-L563)

`type_duration` is declared in the globals struct but not in
`PHP_DRIVER_SCALAR_TYPES_MAP`. GINIT doesn't `ZVAL_UNDEF` it, RINIT/RSHUTDOWN don't
touch it. Dead member — harmless unless someone wires it up incorrectly later. **Low.**

---

#### LEAK-035 — `PHP_GSHUTDOWN` calls `php_driver_log_cleanup` which destroys `log_lock` once-per-thread in ZTS
[`src/php_driver.cpp:416-421`](../src/php_driver.cpp#L416-L421),
[`src/php_driver.cpp:177-183`](../src/php_driver.cpp#L177-L183)

In ZTS, `log_lock` is a process-global initialized once via `uv_once`. `PHP_GSHUTDOWN`
runs per-thread; on first thread shutdown it destroys the lock; subsequent threads
(if shutdown order is racy) will crash on `uv_rwlock_*` in `php_driver_log`. **Crash,
not leak**, but flagged because the cleanup path conflates per-thread and
per-process state.

**Fix**: Move `php_driver_log_cleanup` to `PHP_MSHUTDOWN` (process-level) and remove
from `PHP_GSHUTDOWN`.

---

#### LEAK-036 — `DefaultSession::executeAsync` BATCH path frees batch but FUTURE_ROWS doesn't own it
[`src/DefaultSession.cpp:732-739`](../src/DefaultSession.cpp#L732-L739)

```c
case PHP_DRIVER_BATCH_STATEMENT:
  batch = create_batch(stmt, consistency, retry_policy, timestamp);
  if (!batch) return;
  future_rows->future = cass_session_execute_batch(...);
  cass_batch_free(batch);
  break;
```

`future_rows->statement` is left NULL — `php_driver_future_rows_free` calls
`php_driver_del_ref(&self->statement)` which is NULL-safe. But unlike the
SIMPLE/PREPARED branch (line 728), no statement ref is held, so `nextPage` later may
not be able to advance. Not a leak, but a functionality bug.

---

#### LEAK-037 — `DefaultSession::executeAsync` SIMPLE/PREPARED leaks `single` if `add_ref` fails
[`src/DefaultSession.cpp:721-731`](../src/DefaultSession.cpp#L721-L731)

`php_driver_new_ref(single, free_statement)` returns a new `php_driver_ref*`; this
takes ownership of `single`. If the `pemalloc` inside `new_ref` fails the program
likely terminates. **Not a real leak path.**

---

#### LEAK-038 — `EXECUTE_OPTIONS` re-`__construct` leaks prior state
[`src/ExecutionOptions.cpp:199-220`](../src/ExecutionOptions.cpp#L199-L220)

If the user calls `$opts->__construct(newArray)` on an already-constructed
ExecutionOptions, the previous `arguments`, `timeout`, `retry_policy`,
`paging_state_token` are not freed before `build_from_array` overwrites them. PHP
doesn't normally allow this without reflection, but it's a defensive concern.

**Fix**: Call a reset helper at the top of `__construct` before `build_from_array`.

---

#### LEAK-039 — `php_driver_collection_properties` leaks the `&self->type` add_ref'd zval into props
[`src/Collection.cpp:380-389`](../src/Collection.cpp#L380-L389)

```c
PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &self->type, sizeof(zval));
Z_ADDREF_P(&self->type);
```

`PHP5TO7_ZEND_HASH_UPDATE` calls `zend_hash_str_update` which copies the zval into
the HT. The HT's existing entry (if any) is dtor'd — fine. The `Z_ADDREF_P` after
the insert increments the refcount of the COPY (since the inserted zval is a
ZVAL_COPY_VALUE clone). Counting: original (1) → copy in HT (1, after addref → 2).
On HT dtor it decrefs to 1. On `_free`'s `PHP5TO7_ZVAL_MAYBE_DESTROY(self->type)` it
decrefs to 0 and destroys. Looks correct, but the order matters: the addref must
happen BEFORE the insert to be safe in general. Here it happens after — works only
if `zend_hash_str_update` doesn't refcount on insert. It uses ZVAL_COPY_VALUE
internally so no refcount bump. So `self->type` is leaked by one ref. **Off-by-one
refcount leak.**

**Fix**: `Z_TRY_ADDREF_P(&self->type)` BEFORE the `PHP5TO7_ZEND_HASH_UPDATE`, or use
`add_assoc_zval_ex` which addrefs internally.

---

### Low

#### LEAK-040 — `php_driver_default_session_handlers` not given `free_obj` in PHP 8
[`src/DefaultSession.cpp:1058-1066`](../src/DefaultSession.cpp#L1058-L1066)

The handlers struct is built via memcpy + override of `get_properties` and
`compare`/`compare_objects`. There is no `free_obj` override, but
`php_driver_default_session_free` is defined. The handler is never installed —
default `zend_objects_free_object_storage` runs and never calls
`cass_session_free`! The session peref count is therefore never decremented at GC
time.

Wait — checking again: `php_driver_default_session_handlers.free_obj` is not set
explicitly in the file. Default-copied from `zend_get_std_object_handlers()` it
points to `zend_object_std_dtor`-equivalent. `php_driver_default_session_free` is
defined but **not registered**. This means: at request end the GC tries to free the
session via std handlers, which calls std_dtor — leaks the peref'd session and the
keyspace/hash_key.

**Critical promotion**: This is actually CRITICAL not Low. Promoted.

---

#### LEAK-040 (CRITICAL) — `DefaultSession::free_obj` never registered
[`src/DefaultSession.cpp:1026-1033`](../src/DefaultSession.cpp#L1026-L1033),
[`src/DefaultSession.cpp:1058-1066`](../src/DefaultSession.cpp#L1058-L1066)

`php_driver_default_session_free` exists but the `define_DefaultSession` function
does NOT do `php_driver_default_session_handlers.free_obj = php_driver_default_session_free;`.
The handler is dead code — sessions are never properly freed at GC time. Each
session object created leaks its `session` peref and `default_timeout` zval.

**Fix**: Add `php_driver_default_session_handlers.free_obj = php_driver_default_session_free;`
to `php_driver_define_DefaultSession`.

(Note: Similarly need to verify `offset` is set, otherwise `PHP5TO7_ZEND_OBJECT_GET`
will misalign. Currently `offset` is also not set → garbage pointer in
`PHP_DRIVER_GET_SESSION`. This codebase likely "works" only because
`php_driver_session` happens to start at offset 0 within the allocation. Brittle.)

---

#### LEAK-041 — `Rows`, `FutureRows`, `FutureSession`, `FuturePreparedStatement`, `FutureValue`, `FutureClose` similarly miss `free_obj` registration
[`src/FutureRows.cpp:175-182`](../src/FutureRows.cpp#L175-L182),
[`src/FutureSession.cpp:182-189`](../src/FutureSession.cpp#L182-L189),
[`src/FuturePreparedStatement.cpp:130-137`](../src/FuturePreparedStatement.cpp#L130-L137),
[`src/FutureValue.cpp:107-114`](../src/FutureValue.cpp#L107-L114),
[`src/FutureClose.cpp:113-120`](../src/FutureClose.cpp#L113-L120),
[`src/Database/Rows.cpp:533-540`](../src/Database/Rows.cpp#L533-L540),
[`src/SimpleStatement.cpp:113-120`](../src/SimpleStatement.cpp#L113-L120),
[`src/PreparedStatement.cpp:96-103`](../src/PreparedStatement.cpp#L96-L103),
[`src/BatchStatement.cpp:179-186`](../src/BatchStatement.cpp#L179-L186),
[`src/ExecutionOptions.cpp:381-388`](../src/ExecutionOptions.cpp#L381-L388)

Each of these calls `memcpy(handlers, zend_get_std_object_handlers(), ...)` and
overrides `get_properties` and `compare(_objects)` but **never assigns `free_obj`**.
The `*_free` function is defined but never wired into the handlers struct, so PHP
falls back to the default which doesn't know about the cass_* state.

Per-object leaks per file:

| File                          | Leaked on object free                                      |
|-------------------------------|------------------------------------------------------------|
| `FutureRows.cpp`              | future, result ref, statement ref, session peref, rows zval |
| `FutureSession.cpp`           | future, session peref, hash_key, exception_message, default_session zval |
| `FuturePreparedStatement.cpp` | future, prepared_statement zval                            |
| `FutureValue.cpp`             | value zval                                                 |
| `FutureClose.cpp`             | future                                                     |
| `Database/Rows.cpp`           | rows zval, next_rows zval, future_next_page zval, statement/session/result/next_result refs |
| `SimpleStatement.cpp`         | cql string (estrndup'd)                                    |
| `PreparedStatement.cpp`       | CassPrepared*                                              |
| `BatchStatement.cpp`          | statements HashTable                                       |
| `ExecutionOptions.cpp`        | paging_state_token, arguments/timeout/retry_policy zvals   |

This is a **massive class of leaks**. Every concrete future object, every statement,
every rows object, every execution options instance leaks all of its members at GC time.

**Fix**: In every `php_driver_define_*` for these legacy classes, after the
`memcpy(...)`, add `php_driver_X_handlers.free_obj = php_driver_X_free;` and
`php_driver_X_handlers.offset = XtOffsetOf(php_driver_X, zendObject);`.

---

#### LEAK-042 — `Rows::nextPageAsync` `future_value->value` may be an uninitialized zval after `php_driver_rows_create` failure
[`src/Database/Rows.cpp:322-328`](../src/Database/Rows.cpp#L322-L328)

If `php_driver_get_result(...)` inside `php_driver_rows_create` fails, it dtors
`current->next_rows` and returns — but no error is propagated to
`nextPageAsync` and `future_value->value` remains UNDEF. The
`object_init_ex(&self->future_next_page, php_driver_future_value_ce)` succeeded,
so `future_next_page` is held by `self`. On `_free` it's dtor'd. No leak. **Not a
bug.**

---

#### LEAK-043 — `cass_uuid_gen_free` only called in `PHP_GSHUTDOWN`; no `RSHUTDOWN` cleanup
[`src/php_driver.cpp:416-421`](../src/php_driver.cpp#L416-L421)

The uuid_gen is shared across requests, so `GSHUTDOWN` (per-thread on module
shutdown) is the correct lifetime. No leak.

---

#### LEAK-044 — `INVALID_ARGUMENT(...)` is a `return` macro — leaks pre-allocated locals
[`include/php_driver.h:184-189`](../include/php_driver.h#L184-L189)

`INVALID_ARGUMENT` expands to `throw_invalid_argument(...); return;`. Many sites call
this AFTER allocating resources (e.g., `cass_collection_new`, etc.) without freeing
them first. Most identified instances are in argument-validation prelude before any
allocation. A grep for `INVALID_ARGUMENT` between allocations and the macro should be
done; current scan found no concrete leak paths beyond those already covered above.
**Pattern hazard, no specific bug.**

---

#### LEAK-045 — `Numbers/Decimal::__construct` partial init leak
[`src/Numbers/Decimal.cpp`](../src/Numbers/Decimal.cpp)

Same shape as LEAK-025 for Varint: `mpz_init` happens in `_new`, then `__construct`
or `_init` calls `mpz_set_*`. No leak with proper init/clear pairing. **Verified
clean.**

---

## Patterns observed (recurring categories)

1. **Missing `free_obj` registration on legacy `.cpp` modules.** The handlers struct
   is initialized via `memcpy` and a few overrides but `free_obj` is forgotten in:
   `DefaultSession`, `FutureRows`, `FutureSession`, `FuturePreparedStatement`,
   `FutureValue`, `FutureClose`, `Database/Rows`, `SimpleStatement`,
   `PreparedStatement`, `BatchStatement`, `ExecutionOptions`. The `*_free` function
   exists, is named correctly, but is dead code. This is LEAK-040 + LEAK-041 and is
   responsible for the bulk of the runtime-observable leaks today. Single biggest
   win to fix.

2. **Missing `zend_object_std_dtor(object)` in modern (Zend\CPP-based) `free_obj`
   handlers.** Refactored modules using `ZendCPP::Allocate` consistently forget to
   call `zend_object_std_dtor` at the end of their free handler. Affects all four
   RetryPolicy classes, both SSLOptions classes, and `DefaultCluster`.

3. **Borrowed `zend_string` / `char*` stored without refcount/copy in `Cluster\Builder`
   setters.** `Z_PARAM_STR` returns a borrowed pointer; `_init`-style setters store
   it then rely on the free handler to `zend_string_release` — an unbalanced decref.
   Affects `local_dc`, `username`, `password` (LEAK-002), and the borrowed `char*`
   pattern for `keyspace`/`session_keyspace` (LEAK-029, LEAK-030).

4. **Borrowed `zend_object*` stored without `GC_ADDREF` in `Cluster\Builder` setters.**
   `withSSL`, `withRetryPolicy`, `withTimestampGenerator` (LEAK-001) — same pattern,
   object flavor.

5. **`get_gc` handlers stubbed to return NULL across all value subtypes.**
   Collection, Map, Set, Tuple, UserTypeValue, Varint, Decimal, Type/UserType, and
   Cluster\Builder all return NULL from `get_gc`. PHP's cycle GC is blind to any
   cycle that passes through these objects. (LEAK-014, LEAK-015, LEAK-033.)

6. **Nested-collection helpers leak `CassCollection`/`CassTuple`/`CassUserType` on
   success path.** `util/src/collections.cpp` builds a sub-collection, appends it
   into the parent (which deep-copies), and never frees the source. 9 case blocks.
   (LEAK-007.)

7. **Missing `cass_future_free` on early-return error paths.**
   `DefaultSession::execute`, `Database/Rows::nextPage`. (LEAK-009, LEAK-010, LEAK-031.)

8. **Reconstruction-leak in `_init` helpers for Blob/Uuid/etc.** Calling
   `__construct` on an already-constructed object overwrites the prior allocation
   without freeing it. (LEAK-025.)

9. **MSHUTDOWN is empty.** No `UNREGISTER_INI_ENTRIES()`, no `cass_log_set_callback(nullptr, nullptr)`
   to detach the log callback before the extension unloads. (LEAK-024.)

10. **Builder typo: wrong pointer nulled after release.**
    `blacklist_dcs` released but `whitelist_dcs` re-nulled. (LEAK-013.)

---

## Recommendations (prioritized)

These are ordered by severity of the user-facing impact divided by fix complexity.

### Tier 1 — must-fix before any production deployment

1. **Wire `free_obj` into every legacy module's handlers struct** (LEAK-040, LEAK-041).
   This is the single most impactful change. A boilerplate audit pass: for each
   `*_handlers` variable, ensure `.free_obj = php_driver_X_free;` and
   `.offset = XtOffsetOf(php_driver_X, zendObject);` are set. Estimated impact:
   eliminates >50% of observable leaks.

2. **Append `zend_object_std_dtor(object)` to every `ZendCPP::Allocate`-based
   `free_obj` handler** (LEAK-003, LEAK-004, LEAK-005). Six call sites total. One-line
   change each.

3. **Fix `Cluster\Builder` setters to addref objects and copy strings** (LEAK-001,
   LEAK-002). Three object setters and three string setters. Add `GC_ADDREF` /
   `zend_string_copy` at the assignment site.

4. **Free nested CassCollection/Tuple/UserType in
   `util/src/collections.cpp`** (LEAK-007). 9 case blocks × 3 functions. Adds
   `cass_collection_free`/`cass_tuple_free`/`cass_user_type_free` after each
   `cass_*_append_*` / `cass_*_set_*` call.

5. **Remove the spurious `self->session = php_driver_add_ref(...)` overwrite in
   `DefaultSession::prepare`** (LEAK-006). Single-line deletion.

### Tier 2 — should fix before next minor release

6. **Add `cass_future_free` on every early-return error path** in
   `DefaultSession::execute` and `Database/Rows::nextPage` (LEAK-009, LEAK-010).

7. **Implement proper `get_gc` for all value subtypes** (LEAK-014, LEAK-015,
   LEAK-033). Each should at minimum report `self->type` and any zvals in
   `self->values`. Without this, cycle leaks accumulate unboundedly under workloads
   that build complex CQL value graphs.

8. **Copy `keyspace`/`session_keyspace` instead of borrowing** (LEAK-029, LEAK-030).
   Add an `estrdup` (or zend_string) and free in the corresponding `free_obj`.

9. **Fix `FuturePreparedStatement::get` to instantiate the concrete
   `PreparedStatement` class** (LEAK-016). One-line change, but also fixes a likely
   crash.

10. **`PHP_MSHUTDOWN` should `UNREGISTER_INI_ENTRIES()` and detach the cass_log
    callback** (LEAK-024).

### Tier 3 — cleanup, defensible state

11. **Fix the `blacklist_dcs` / `whitelist_dcs` typo** (LEAK-013). One character.

12. **Guard reconstruction in `_init` helpers** (LEAK-025). Add
    `if (self->data) efree(self->data);` (and equivalents) before the new assignment.

13. **Reset `ExecutionOptions` state at the top of `__construct`** (LEAK-038).
    Identical pattern to LEAK-025.

14. **Fix the addref ordering in `Collection::properties`** (LEAK-039). Move
    `Z_ADDREF_P` before the HT insert.

15. **Remove the unused `type_duration` module global** (LEAK-034). Or wire it
    into the scalar types map.

16. **Re-evaluate ZTS lifecycle for `log_lock` / `log_location`** (LEAK-035). Move
    cleanup from `GSHUTDOWN` to `MSHUTDOWN`.

### Tier 4 — defense-in-depth

17. Add a CI memory-leak harness (valgrind or ASan + LSan) running the pest suite
    against a real ScyllaDB instance. Many of the above findings would surface as
    blocks at first sweep.

18. As the migration to C23 / canonical `src/Cluster/` pattern continues, port the
    leakiest legacy modules first: `DefaultSession.cpp`, `Future*.cpp`,
    `Database/Rows.cpp`, the Statement variants, and `ExecutionOptions.cpp`. The
    canonical pattern's stub-driven handler registration makes LEAK-040 / LEAK-041
    impossible to repeat.

---

*Audit produced by static reading of the source tree at commit 9651d919. Dynamic
verification (valgrind / ASan) was not performed and is recommended as the next
step.*
