/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Cassandra\Async\Poll — a driver-owned event loop over Io\Poll\Context.
 *
 * Compiled only in a build with HAVE_PHP_POLL_API (see PollHandle.c). It drives
 * ext/standard's Context rather than its own php_poll_ctx, so the same loop can
 * also watch the caller's sockets and pipes through context().
 *
 * The loop owns, per watched future: the pinned Future object and an optional
 * completion callback. tick() removes a watcher as soon as its future resolves —
 * mandatory, not tidiness: a driver notification descriptor stays readable after
 * it fires, so a watcher left in place would report an event every tick.
 *
 * Nothing here runs on a driver IO thread. Completion still travels over the
 * notifier descriptor; this is all PHP-thread work.
 */

#include "php_scylladb.h"
#include "php_scylladb_types.h"

#include <php_poll.h>

#include <Zend/zend_enum.h>

#include "Async/PollHandle.h"

#include "Async/Poll_arginfo.h"

extern zend_object_handlers php_scylladb_async_poll_handlers;

/* One watched future. Zend-allocated: unlike the reactor's records, nothing here
 * is ever touched off the PHP thread. */
typedef struct
{
  zend_object*          future;
  zend_fcall_info_cache fcc; /* uninitialised when watch() got no callback */
} php_scylladb_poll_watch;

/* ─── Io\Poll class lookups ───────────────────────────────────────────────────
 * Resolved once per process from CG(class_table) — ext/standard registers them
 * long before a shared extension starts up, and caching avoids a lookup per
 * call. Never null once MINIT of ext/standard ran, but checked anyway so a
 * hypothetical PHP without the classes throws instead of crashing. */

static zend_class_entry*
poll_class(const char* lc_name, size_t len)
{
  zend_class_entry* ce = zend_hash_str_find_ptr(CG(class_table), lc_name, len);
  if (ce == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "The polling API is unavailable in this PHP build");
  }
  return ce;
}

static zend_class_entry*
poll_context_ce(void)
{
  static zend_class_entry* ce = nullptr;
  if (ce == nullptr) {
    ce = poll_class(ZEND_STRL("io\\poll\\context"));
  }
  return ce;
}

static zend_class_entry*
poll_event_ce(void)
{
  static zend_class_entry* ce = nullptr;
  if (ce == nullptr) {
    ce = poll_class(ZEND_STRL("io\\poll\\event"));
  }
  return ce;
}

/* Call $obj->$name(...). Returns FAILURE with the exception left pending. */
static int
poll_call(zval* obj, const char* lc_name, size_t len, zval* retval, uint32_t argc, zval* argv)
{
  zend_function* fn = zend_hash_str_find_ptr(&Z_OBJCE_P(obj)->function_table, lc_name, len);
  if (fn == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "%s::%s() is missing — unexpected polling API shape",
                            ZSTR_VAL(Z_OBJCE_P(obj)->name), lc_name);
    return FAILURE;
  }

  zend_call_known_instance_method(fn, Z_OBJ_P(obj), retval, argc, argv);
  return EG(exception) ? FAILURE : SUCCESS;
}

/* ─── object lifecycle ────────────────────────────────────────────────────── */

/* Also runs for an entry tick() already emptied on its way to dispatch, so both
 * fields are optional here. */
static void
poll_watch_free(zval* zv)
{
  php_scylladb_poll_watch* watch = Z_PTR_P(zv);

  if (watch->future != nullptr) {
    OBJ_RELEASE(watch->future);
  }
  if (ZEND_FCC_INITIALIZED(watch->fcc)) {
    zend_fcc_dtor(&watch->fcc);
  }
  efree(watch);
}

zend_object*
php_scylladb_async_poll_new(zend_class_entry* ce)
{
  php_scylladb_async_poll* self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_async_poll, ce, &php_scylladb_async_poll_handlers);

  ZVAL_UNDEF(&self->context);
  self->watched = nullptr; /* the constructor allocates it */

  return &self->zendObject;
}

void
php_scylladb_async_poll_free(zend_object* object)
{
  php_scylladb_async_poll* self = php_scylladb_async_poll_object_fetch(object);

  if (!Z_ISUNDEF(self->context)) {
    zval_ptr_dtor(&self->context);
    ZVAL_UNDEF(&self->context);
  }
  if (self->watched != nullptr) {
    zend_hash_destroy(self->watched);
    FREE_HASHTABLE(self->watched);
    self->watched = nullptr;
  }

  zend_object_std_dtor(object);
}

/* A callback may capture the loop, so every reference has to be reachable or a
 * cycle leaks. */
HashTable*
php_scylladb_async_poll_gc(zend_object* object, zval** table, int* n)
{
  php_scylladb_async_poll* self = php_scylladb_async_poll_object_fetch(object);
  zend_get_gc_buffer*      buf  = zend_get_gc_buffer_create();

  if (!Z_ISUNDEF(self->context)) {
    zend_get_gc_buffer_add_zval(buf, &self->context);
  }

  if (self->watched != nullptr) {
    php_scylladb_poll_watch* watch = nullptr;
    ZEND_HASH_FOREACH_PTR(self->watched, watch)
    {
      zend_get_gc_buffer_add_obj(buf, watch->future);
      if (ZEND_FCC_INITIALIZED(watch->fcc)) {
        zend_get_gc_buffer_add_fcc(buf, &watch->fcc);
      }
    }
    ZEND_HASH_FOREACH_END();
  }

  zend_get_gc_buffer_use(buf, table, n);
  return zend_std_get_properties(object);
}

/* ─── Cassandra\Async\Poll ────────────────────────────────────────────────── */

ZEND_METHOD(Cassandra_Async_Poll, __construct)
{
  zval* backend = nullptr;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_OBJECT_OR_NULL(backend)
  ZEND_PARSE_PARAMETERS_END();

  zend_class_entry* ctx_ce = poll_context_ce();
  if (ctx_ce == nullptr) {
    RETURN_THROWS();
  }

  php_scylladb_async_poll* self = PHP_SCYLLADB_GET_ASYNC_POLL(ZEND_THIS);

  if (object_init_ex(&self->context, ctx_ce) != SUCCESS) {
    RETURN_THROWS();
  }

  /* Backend::Auto is the parameter default, so passing nothing is the same as
     passing Auto — let the context apply its own default. */
  zend_call_known_instance_method(ctx_ce->constructor, Z_OBJ(self->context), nullptr,
                                 backend == nullptr ? 0 : 1, backend);
  if (EG(exception)) {
    RETURN_THROWS();
  }

  ALLOC_HASHTABLE(self->watched);
  zend_hash_init(self->watched, 8, nullptr, poll_watch_free, 0);
}

ZEND_METHOD(Cassandra_Async_Poll, isSupported)
{
  ZEND_PARSE_PARAMETERS_NONE();

  RETURN_BOOL(zend_hash_str_exists(CG(class_table), ZEND_STRL("io\\poll\\context")));
}

ZEND_METHOD(Cassandra_Async_Poll, watch)
{
  zval*                 future_zv = nullptr;
  zend_fcall_info       fci       = empty_fcall_info;
  zend_fcall_info_cache fcc       = empty_fcall_info_cache;

  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_OBJECT_OF_CLASS(future_zv, php_scylladb_future_ce)
    Z_PARAM_OPTIONAL
    Z_PARAM_FUNC_OR_NULL(fci, fcc)
  ZEND_PARSE_PARAMETERS_END();

  php_scylladb_async_poll* self     = PHP_SCYLLADB_GET_ASYNC_POLL(ZEND_THIS);
  zend_class_entry*        event_ce = poll_event_ce();
  if (event_ce == nullptr) {
    RETURN_THROWS();
  }

  zval handle;
  if (php_scylladb_poll_handle_for_future(future_zv, &handle) == FAILURE) {
    RETURN_THROWS();
  }

  zend_object* read = zend_enum_get_case_cstr(event_ce, "Read");
  if (read == nullptr) {
    zval_ptr_dtor(&handle);
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Io\\Poll\\Event::Read is missing — unexpected polling API shape");
    RETURN_THROWS();
  }

  zval args[2];
  ZVAL_COPY_VALUE(&args[0], &handle); /* ownership moves into args */
  array_init_size(&args[1], 1);
  GC_ADDREF(read);
  add_next_index_object(&args[1], read);

  int rc = poll_call(&self->context, ZEND_STRL("add"), return_value, 2, args);

  zval_ptr_dtor(&args[0]);
  zval_ptr_dtor(&args[1]);

  if (rc == FAILURE) {
    RETURN_THROWS();
  }

  php_scylladb_poll_watch* watch = emalloc(sizeof(*watch));
  watch->future                  = Z_OBJ_P(future_zv);
  GC_ADDREF(watch->future); /* pin until it resolves or the loop dies */
  watch->fcc = empty_fcall_info_cache;
  if (ZEND_FCC_INITIALIZED(fcc)) {
    zend_fcc_dup(&watch->fcc, &fcc);
  }

  /* Keyed by the watcher's object handle: Context hands back a distinct Watcher
     per add(), and wait() returns those same objects. */
  zend_hash_index_update_ptr(self->watched, Z_OBJ_HANDLE_P(return_value), watch);
}

ZEND_METHOD(Cassandra_Async_Poll, pending)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_async_poll* self = PHP_SCYLLADB_GET_ASYNC_POLL(ZEND_THIS);
  RETURN_LONG(self->watched == nullptr ? 0 : (zend_long)zend_hash_num_elements(self->watched));
}

/*
 * Context::wait()'s first parameter took (?int $seconds, int $micros, ?int $max)
 * up to 8.6.0alpha3 and a ?Time\Duration after it. Both spellings are still in
 * the wild while 8.6 is in alpha, so look at the declared type once and build
 * the argument to match. For an internal function arg_info[0] is the first real
 * argument (zend_register_functions offsets past the return info).
 */
static int
poll_wait(zval* context, zval* timeout, zval* retval)
{
  zend_function* fn = zend_hash_str_find_ptr(&Z_OBJCE_P(context)->function_table, ZEND_STRL("wait"));
  if (fn == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Io\\Poll\\Context::wait() is missing — unexpected polling API shape");
    return FAILURE;
  }

  bool wants_object = fn->common.num_args > 0 && ZEND_TYPE_HAS_NAME(fn->common.arg_info[0].type);

  /* null means "wait indefinitely" in either spelling. */
  if (!wants_object || Z_TYPE_P(timeout) == IS_NULL) {
    return poll_call(context, ZEND_STRL("wait"), retval, 1, timeout);
  }

  /* Time\Duration is readonly with a private constructor, so go through its
     static factory rather than object_init_ex + __construct — that would leave
     the readonly properties unset. */
  zend_string*      name   = ZEND_TYPE_NAME(fn->common.arg_info[0].type);
  zend_class_entry* dur_ce = zend_lookup_class(name);
  zend_function*    from_seconds =
      dur_ce == nullptr
             ? nullptr
             : zend_hash_str_find_ptr(&dur_ce->function_table, ZEND_STRL("fromseconds"));

  if (from_seconds == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Io\\Poll\\Context::wait() wants a %s, which has no fromSeconds() "
                            "this build can call; pass null, or call context()->wait() yourself",
                            ZSTR_VAL(name));
    return FAILURE;
  }

  zval duration;
  zend_call_known_function(from_seconds, nullptr, dur_ce, &duration, 1, timeout, nullptr);
  if (EG(exception)) {
    return FAILURE;
  }

  int rc = poll_call(context, ZEND_STRL("wait"), retval, 1, &duration);
  zval_ptr_dtor(&duration);
  return rc;
}

/* Take one ready watcher out of the loop: detach its payload, drop the table
 * entry, and remove the watcher from the context. A driver descriptor stays
 * readable after it fires, so leaving the watcher in place would make the loop
 * spin. Returns FAILURE (thrown) when the context refuses. */
static int
poll_take(php_scylladb_async_poll* self,
          zval*                    watcher_zv,
          php_scylladb_poll_watch* watch,
          zend_object**            future_out,
          zend_fcall_info_cache*   fcc_out)
{
  /* Take ownership out of the table before dispatching: a callback may call
     watch()/tick() again, and must not see a half-consumed entry. */
  *future_out   = watch->future;
  *fcc_out      = watch->fcc;
  watch->future = nullptr;
  watch->fcc    = empty_fcall_info_cache;
  zend_hash_index_del(self->watched, Z_OBJ_HANDLE_P(watcher_zv));

  if (poll_call(watcher_zv, ZEND_STRL("remove"), nullptr, 0, nullptr) == FAILURE) {
    OBJ_RELEASE(*future_out);
    if (ZEND_FCC_INITIALIZED(*fcc_out)) {
      zend_fcc_dtor(fcc_out);
    }
    return FAILURE;
  }
  return SUCCESS;
}

/* Wait once, then dispatch. `out` collects the callback-less futures; pass null
 * to discard them (run()).
 *
 * Callbacks run before anything is collected, in two passes over the same ready
 * set. A callback that throws therefore aborts the tick while every
 * callback-less future is still watched, so the next tick picks it up — a
 * throwing poll never returns its array, and a one-pass loop would drop
 * whatever it had already put there. */
static int
poll_tick(php_scylladb_async_poll* self, zval* timeout, zval* out)
{
  zval watchers;
  if (poll_wait(&self->context, timeout, &watchers) == FAILURE) {
    return FAILURE;
  }

  if (Z_TYPE(watchers) != IS_ARRAY) {
    zval_ptr_dtor(&watchers);
    return SUCCESS;
  }

  zval* watcher_zv = nullptr;

  /* Pass 1 — the futures a callback is waiting on. */
  ZEND_HASH_FOREACH_VAL(Z_ARRVAL(watchers), watcher_zv)
  {
    if (Z_TYPE_P(watcher_zv) != IS_OBJECT) {
      continue;
    }

    php_scylladb_poll_watch* watch =
        zend_hash_index_find_ptr(self->watched, Z_OBJ_HANDLE_P(watcher_zv));
    /* null: someone else's watcher on our context, or one a re-entrant tick()
       already took. Either way, leave it alone. */
    if (watch == nullptr || !ZEND_FCC_INITIALIZED(watch->fcc)) {
      continue;
    }

    zend_object*          future = nullptr;
    zend_fcall_info_cache fcc    = empty_fcall_info_cache;
    if (poll_take(self, watcher_zv, watch, &future, &fcc) == FAILURE) {
      zval_ptr_dtor(&watchers);
      return FAILURE;
    }

    zval args[1];
    zval unused;
    ZVAL_OBJ(&args[0], future);

    zend_call_known_fcc(&fcc, &unused, 1, args, nullptr);
    zval_ptr_dtor(&unused);
    zend_fcc_dtor(&fcc);
    OBJ_RELEASE(future);

    if (EG(exception)) {
      zval_ptr_dtor(&watchers);
      return FAILURE; /* the rest stays watched for the next tick */
    }
  }
  ZEND_HASH_FOREACH_END();

  /* Pass 2 — the futures the caller reads itself. No user code runs here. */
  ZEND_HASH_FOREACH_VAL(Z_ARRVAL(watchers), watcher_zv)
  {
    if (Z_TYPE_P(watcher_zv) != IS_OBJECT) {
      continue;
    }

    php_scylladb_poll_watch* watch =
        zend_hash_index_find_ptr(self->watched, Z_OBJ_HANDLE_P(watcher_zv));
    if (watch == nullptr) {
      continue;
    }

    zend_object*          future = nullptr;
    zend_fcall_info_cache fcc    = empty_fcall_info_cache;
    if (poll_take(self, watcher_zv, watch, &future, &fcc) == FAILURE) {
      zval_ptr_dtor(&watchers);
      return FAILURE;
    }

    if (out != nullptr) {
      zval zv;
      ZVAL_OBJ(&zv, future); /* the pin moves into the array */
      add_next_index_zval(out, &zv);
    } else {
      OBJ_RELEASE(future);
    }
  }
  ZEND_HASH_FOREACH_END();

  zval_ptr_dtor(&watchers);
  return SUCCESS;
}

ZEND_METHOD(Cassandra_Async_Poll, tick)
{
  zval* timeout = nullptr;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  php_scylladb_async_poll* self = PHP_SCYLLADB_GET_ASYNC_POLL(ZEND_THIS);

  array_init(return_value);
  if (self->watched == nullptr || zend_hash_num_elements(self->watched) == 0) {
    return;
  }

  zval null_timeout;
  if (timeout == nullptr) {
    ZVAL_NULL(&null_timeout);
    timeout = &null_timeout;
  }

  poll_tick(self, timeout, return_value);
}

ZEND_METHOD(Cassandra_Async_Poll, run)
{
  zval* timeout = nullptr;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  php_scylladb_async_poll* self = PHP_SCYLLADB_GET_ASYNC_POLL(ZEND_THIS);
  if (self->watched == nullptr) {
    return;
  }

  zval null_timeout;
  if (timeout == nullptr) {
    ZVAL_NULL(&null_timeout);
    timeout = &null_timeout;
  }

  while (zend_hash_num_elements(self->watched) > 0) {
    if (poll_tick(self, timeout, nullptr) == FAILURE) {
      return;
    }
  }
}

ZEND_METHOD(Cassandra_Async_Poll, context)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_async_poll* self = PHP_SCYLLADB_GET_ASYNC_POLL(ZEND_THIS);
  if (Z_ISUNDEF(self->context)) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "This Poll was never constructed");
    RETURN_THROWS();
  }

  RETURN_COPY(&self->context);
}
