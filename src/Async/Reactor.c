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

#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>

#include "php_scylladb.h"
#include "php_scylladb_globals.h"
#include "php_scylladb_types.h"

#include "FutureNotifier.h"
#include "Async/Reactor.h"

#include "Async/Reactor_arginfo.h"

/*
 * One registration per future added to the reactor. Persistent-malloc'd and
 * atomically refcounted (like the notifier) because the driver completion
 * callback runs on an IO thread and only ever touches this record + the
 * reactor's fd/mutex — never any zval/zend state.
 *
 * Refcount owners at creation (=2): the "registered" list (main thread) and the
 * pending completion (transferred to the ready-list when the callback enqueues).
 */
typedef struct php_scylladb_reg_
{
  _Atomic uint32_t     refcount;
  _Atomic bool         enqueued;   /* CAS-guards the single ready-list push */
  php_scylladb_reactor* reactor;
  zend_object*         future_obj; /* pinned Future (GC_ADDREF'd); nulled on dispatch */
  /* Resolved once by add(). `reg_slot` points into the pinned object's own
     struct, which stays alive for this reg's whole life, so neither pointer
     needs re-deriving from the class on dispatch. `future` is null for a
     FutureValue, which resolves on construction. */
  CassFuture*          future;
  php_scylladb_reg**   reg_slot;
  /* Optional completion callback. Resolved once by add() and held through
     zend_fcc_dup, so a dispatch costs no callable re-resolution. Main thread
     only — the IO-thread callback never reads it. Left uninitialised when
     add() got none. */
  zend_fcall_info_cache fcc;
  struct php_scylladb_reg_* next_ready; /* MPSC ready-list (cross-thread, mutex) */
  struct php_scylladb_reg_* prev_reg;   /* registered-list (main thread) */
  struct php_scylladb_reg_* next_reg;
} php_scylladb_reg;

struct php_scylladb_reactor_
{
  _Atomic uint32_t      refcount;    /* globals ref + one per live reg */
  php_scylladb_notifier* fd;         /* the single shared eventfd/pipe */
  pthread_mutex_t       mutex;       /* guards the ready-list only */
  php_scylladb_reg*     ready_head;  /* MPSC ready-list */
  php_scylladb_reg*     ready_tail;
  php_scylladb_reg*     registered;  /* all outstanding regs (main thread) */
  uint32_t              pending;
  zval                  stream;      /* cached resource returned by resource() */
  zval                  poll_handle; /* cached Io\Poll handle (PHP >= 8.6) */
};

/* ─── reactor refcount (main thread only; reg_unref may free it) ─────────────── */

static void
php_scylladb_reactor_addref(php_scylladb_reactor* reactor)
{
  atomic_fetch_add_explicit(&reactor->refcount, 1, memory_order_relaxed);
}

static void
php_scylladb_reactor_unref(php_scylladb_reactor* reactor)
{
  if (atomic_fetch_sub_explicit(&reactor->refcount, 1, memory_order_acq_rel) == 1) {
    if (!Z_ISUNDEF(reactor->stream)) {
      zval_ptr_dtor(&reactor->stream);
    }
    if (!Z_ISUNDEF(reactor->poll_handle)) {
      zval_ptr_dtor(&reactor->poll_handle);
    }
    pthread_mutex_destroy(&reactor->mutex);
    php_scylladb_notifier_unref(reactor->fd);
    pefree(reactor, 1);
  }
}

php_scylladb_reactor*
php_scylladb_reactor_create(void)
{
  php_scylladb_notifier* fd = php_scylladb_notifier_create();
  if (fd == nullptr) {
    return nullptr;
  }

  php_scylladb_reactor* reactor = pemalloc(sizeof(*reactor), 1);
  atomic_init(&reactor->refcount, 1);
  reactor->fd = fd;
  pthread_mutex_init(&reactor->mutex, nullptr);
  reactor->ready_head = nullptr;
  reactor->ready_tail = nullptr;
  reactor->registered = nullptr;
  reactor->pending    = 0;
  ZVAL_UNDEF(&reactor->stream);
  ZVAL_UNDEF(&reactor->poll_handle);
  return reactor;
}

/* ─── registration records ──────────────────────────────────────────────────── */

/* Create a reg for `future_obj`, pin the object, and link it into the
 * registered-list. Main thread only. */
static php_scylladb_reg*
reg_create(php_scylladb_reactor*            reactor,
           zend_object*                     future_obj,
           const php_scylladb_future_slots* slots,
           const zend_fcall_info_cache*     callback)
{
  php_scylladb_reg* reg = pemalloc(sizeof(*reg), 1);
  atomic_init(&reg->refcount, 2);
  atomic_init(&reg->enqueued, false);
  reg->reactor = reactor;
  php_scylladb_reactor_addref(reactor);

  GC_ADDREF(future_obj); /* pin until dispatched */
  reg->future_obj = future_obj;
  reg->future     = slots->future;
  reg->reg_slot   = slots->reactor_reg;
  *reg->reg_slot  = reg;

  reg->fcc = empty_fcall_info_cache;
  if (callback != nullptr && ZEND_FCC_INITIALIZED(*callback)) {
    zend_fcc_dup(&reg->fcc, callback);
  }

  reg->next_ready = nullptr;
  reg->prev_reg   = nullptr;
  reg->next_reg   = reactor->registered;
  if (reactor->registered) {
    reactor->registered->prev_reg = reg;
  }
  reactor->registered = reg;
  reactor->pending++;
  return reg;
}

/* Release both of a reg's references (ready-list + registered) at once and free
 * it if that was the last reference. Consuming a reg always drops exactly these
 * two, and doing it in a single atomic decrement (rather than two calls) keeps
 * the free path unambiguous for static analysis. Main thread only. */
static void
reg_release(php_scylladb_reg* reg)
{
  if (atomic_fetch_sub_explicit(&reg->refcount, 2, memory_order_acq_rel) == 2) {
    if (reg->future_obj) { /* safety net; normally cleared at dispatch */
      OBJ_RELEASE(reg->future_obj);
    }
    if (ZEND_FCC_INITIALIZED(reg->fcc)) {
      zend_fcc_dtor(&reg->fcc);
    }
    php_scylladb_reactor_unref(reg->reactor);
    pefree(reg, 1);
  }
}

/* Unlink from the registered-list. Main thread only. */
static void
reg_unregister(php_scylladb_reactor* reactor, php_scylladb_reg* reg)
{
  if (reg->prev_reg) {
    reg->prev_reg->next_reg = reg->next_reg;
  } else {
    reactor->registered = reg->next_reg;
  }
  if (reg->next_reg) {
    reg->next_reg->prev_reg = reg->prev_reg;
  }
  reg->prev_reg = nullptr;
  reg->next_reg = nullptr;
  reactor->pending--;
}

/* Push a reg to the ready-list and wake the fd. Exactly one push per reg is
 * performed (CAS guard), so the driver callback and the register-time recheck
 * can both call this safely. Runs on either thread. */
static void
reg_enqueue(php_scylladb_reg* reg)
{
  bool expected = false;
  if (!atomic_compare_exchange_strong_explicit(
          &reg->enqueued, &expected, true, memory_order_acq_rel, memory_order_relaxed)) {
    return; /* already enqueued */
  }

  php_scylladb_reactor* reactor = reg->reactor;
  pthread_mutex_lock(&reactor->mutex);
  reg->next_ready = nullptr;
  if (reactor->ready_tail) {
    reactor->ready_tail->next_ready = reg;
  } else {
    reactor->ready_head = reg;
  }
  reactor->ready_tail = reg;

  /* Signal the fd BEFORE unlocking. This is the callback's last touch of the
     reactor, and the consumer (reactor_splice) must take this same mutex before
     it can splice — and thus free — the reg/reactor. Signalling under the lock
     therefore orders any free strictly after this callback is done, closing a
     cross-thread use-after-free window at GSHUTDOWN. The write() is non-blocking
     (O_NONBLOCK fd), so holding the lock across it cannot deadlock. */
  php_scylladb_notifier_signal(reactor->fd);
  pthread_mutex_unlock(&reactor->mutex);
}

/* Driver completion callback — IO thread. Touches only the reg + reactor. */
static void
reactor_cb(CassFuture* future, void* data)
{
  (void)future;
  reg_enqueue((php_scylladb_reg*)data);
}

/* Detach + consume one spliced ready-list. Main thread only.
 *
 * With `out`: a future that add() got a callback for has that callback invoked
 * with the future as its only argument; a future registered without one is
 * appended to `out` instead. Without `out` (RSHUTDOWN discard) no callback runs
 * and the pins are simply released.
 *
 * Stops after `max` regs (0 = no limit), or as soon as a callback throws, and
 * returns whatever is left. */
static php_scylladb_reg*
reactor_consume(php_scylladb_reactor* reactor, php_scylladb_reg* head, zval* out, uint32_t max)
{
  for (uint32_t taken = 0; head && (max == 0 || taken < max); taken++) {
    php_scylladb_reg* reg = head;
    head                  = head->next_ready;

    reg_unregister(reactor, reg);

    if (reg->future_obj) {
      *reg->reg_slot = nullptr; /* detach so getResource()/free_obj are safe again */

      if (out && ZEND_FCC_INITIALIZED(reg->fcc)) {
        zval args[1];
        zval retval;
        ZVAL_OBJ(&args[0], reg->future_obj); /* borrowed — the pin drops below */

        zend_call_known_fcc(&reg->fcc, &retval, 1, args, nullptr);
        zval_ptr_dtor(&retval);
        OBJ_RELEASE(reg->future_obj);
      } else if (out) {
        zval zv;
        ZVAL_OBJ(&zv, reg->future_obj); /* transfer the pin ref into the array */
        add_next_index_zval(out, &zv);
      } else {
        OBJ_RELEASE(reg->future_obj);
      }
      reg->future_obj = nullptr;
    }

    reg_release(reg); /* drop ready-list + registered refs → frees */

    /* A throwing callback stops the drain. The rest is requeued by the caller,
       so the loop picks it up on the next readable event. */
    if (EG(exception)) {
      break;
    }
  }

  return head;
}

/* Put an unconsumed remainder back at the FRONT of the ready-list (those regs
 * completed before anything that arrived meanwhile) and re-signal the fd, so a
 * level-triggered watcher wakes again on the next tick. Main thread only. */
static void
reactor_requeue(php_scylladb_reactor* reactor, php_scylladb_reg* head)
{
  php_scylladb_reg* tail = head;
  while (tail->next_ready) {
    tail = tail->next_ready;
  }

  pthread_mutex_lock(&reactor->mutex);
  tail->next_ready = reactor->ready_head;
  reactor->ready_head = head;
  if (reactor->ready_tail == nullptr) {
    reactor->ready_tail = tail;
  }
  php_scylladb_notifier_signal(reactor->fd);
  pthread_mutex_unlock(&reactor->mutex);
}

/* Re-register the futures reactor_consume() already collected into `out`.
 *
 * A callback that throws makes poll() return nothing, so anything already in the
 * array would be dropped: it was unregistered here but never handed to userland.
 * Registering it again — no driver callback needed, it has already resolved —
 * puts it back at the FRONT of the ready-list, so the next poll() delivers it
 * before the rest of the batch. Empties `out`. Main thread only. */
static void
reactor_reclaim(php_scylladb_reactor* reactor, zval* out)
{
  php_scylladb_reg* head = nullptr;
  php_scylladb_reg* tail = nullptr;
  zval*             entry;

  ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(out), entry)
  {
    /* The non-throwing form: a callback's exception is pending here and must
       survive. It cannot fail anyway — the array only holds futures this
       reactor put there. */
    php_scylladb_future_slots slots;
    if (!php_scylladb_future_slots_try(entry, &slots)) {
      continue;
    }

    php_scylladb_reg* reg = reg_create(reactor, Z_OBJ_P(entry), &slots, nullptr);
    /* Never handed to a driver thread, so claim the single push straight away
       and link it by hand instead of going through reg_enqueue (which appends
       to the tail). */
    atomic_store_explicit(&reg->enqueued, true, memory_order_relaxed);
    reg->next_ready = nullptr;
    if (tail != nullptr) {
      tail->next_ready = reg;
    } else {
      head = reg;
    }
    tail = reg;
  }
  ZEND_HASH_FOREACH_END();

  /* reg_create pinned each object, so dropping the array's references here is
     balanced. */
  zend_hash_clean(Z_ARRVAL_P(out));

  if (head != nullptr) {
    reactor_requeue(reactor, head);
  }
}

/* Splice the whole ready-list under the lock. */
static php_scylladb_reg*
reactor_splice(php_scylladb_reactor* reactor)
{
  pthread_mutex_lock(&reactor->mutex);
  php_scylladb_reg* head = reactor->ready_head;
  reactor->ready_head    = nullptr;
  reactor->ready_tail    = nullptr;
  pthread_mutex_unlock(&reactor->mutex);
  return head;
}

/* Force every outstanding future to complete so its callback fires now (never
   after teardown), then consume + discard. Loop because a callback may still be
   in flight on an IO thread immediately after cass_future_wait returns. */
static void
reactor_drain(php_scylladb_reactor* reactor)
{
  while (reactor->registered != nullptr) {
    for (php_scylladb_reg* reg = reactor->registered; reg; reg = reg->next_reg) {
      if (reg->future != nullptr) {
        cass_future_wait(reg->future);
      }
    }

    php_scylladb_notifier_drain(reactor->fd);
    php_scylladb_reg* head = reactor_splice(reactor);
    if (head) {
      reactor_consume(reactor, head, nullptr, 0);
    } else if (reactor->registered != nullptr) {
      sched_yield(); /* callback in flight; yield and retry */
    }
  }
}

void
php_scylladb_reactor_reset(php_scylladb_reactor* reactor)
{
  if (reactor == nullptr) {
    return;
  }

  /* Per-request (RSHUTDOWN): drop all request-scoped state — the pinned
     FutureRows and the php_stream resource — but KEEP the eventfd + mutex alive
     for the process/thread so the next request reuses them (no per-request fd
     churn). The next resource() call re-publishes a fresh stream over the same
     fd. */
  reactor_drain(reactor);
  php_scylladb_notifier_drain(reactor->fd); /* clear any residual counter */

  if (!Z_ISUNDEF(reactor->stream)) {
    zval_ptr_dtor(&reactor->stream);
    ZVAL_UNDEF(&reactor->stream);
  }
  if (!Z_ISUNDEF(reactor->poll_handle)) {
    zval_ptr_dtor(&reactor->poll_handle);
    ZVAL_UNDEF(&reactor->poll_handle);
  }
}

void
php_scylladb_reactor_destroy(php_scylladb_reactor* reactor)
{
  if (reactor == nullptr) {
    return;
  }

  /* Process/thread teardown (GSHUTDOWN): reset then drop the module-globals
     reference, which frees the eventfd + mutex once the last reg is gone. */
  reactor_drain(reactor);
  php_scylladb_reactor_unref(reactor);
}

/* ─── module-globals accessor ───────────────────────────────────────────────── */

static php_scylladb_reactor*
reactor_instance(void)
{
  if (PHP_SCYLLADB_G(reactor) == nullptr) {
    php_scylladb_reactor* reactor = php_scylladb_reactor_create();
    if (reactor == nullptr) {
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                              "Failed to create the async reactor");
      return nullptr;
    }
    PHP_SCYLLADB_G(reactor) = reactor;
  }
  return PHP_SCYLLADB_G(reactor);
}

#ifdef HAVE_PHP_POLL_API
bool
php_scylladb_reactor_poll_slots(php_scylladb_notifier** notifier_out, zval** handle_slot_out)
{
  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return false;
  }

  *notifier_out    = reactor->fd;
  *handle_slot_out = &reactor->poll_handle;
  return true;
}
#endif

/* ─── PHP-facing Cassandra\Async\Reactor (static methods) ────────────────────── */

ZEND_METHOD(Cassandra_Async_Reactor, resource)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return;
  }
  php_scylladb_notifier_publish(reactor->fd, &reactor->stream, return_value);
}

ZEND_METHOD(Cassandra_Async_Reactor, add)
{
  zval*                 future_zv = nullptr;
  zend_fcall_info       fci       = empty_fcall_info;
  zend_fcall_info_cache fcc       = empty_fcall_info_cache;

  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_OBJECT_OF_CLASS(future_zv, php_scylladb_future_ce)
    Z_PARAM_OPTIONAL
    Z_PARAM_FUNC_OR_NULL(fci, fcc)
  ZEND_PARSE_PARAMETERS_END();

  php_scylladb_future_slots slots;
  if (!php_scylladb_future_slots_get(future_zv, &slots) ||
      !php_scylladb_future_claim_reactor(Z_OBJ_P(future_zv), *slots.reactor_reg, *slots.notifier)) {
    return;
  }

  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return;
  }

  php_scylladb_reg* reg = reg_create(reactor, Z_OBJ_P(future_zv), &slots, &fcc);

  if (slots.future == nullptr) {
    /* Resolved on construction (FutureValue): there is nothing to wait for, so
       hand it straight to the ready-list. */
    reg_enqueue(reg);
    return;
  }

  CassError rc = cass_future_set_callback(slots.future, reactor_cb, reg);

  if (rc != CASS_OK && !cass_future_ready(slots.future)) {
    /* No completion will ever be delivered for a still-pending future. Enqueueing
       it would make poll() hand back a future whose get() blocks, so roll the
       registration back instead. */
    *reg->reg_slot = nullptr;
    reg_unregister(reactor, reg);
    reg_release(reg);
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to register the async completion callback: %s",
                            cass_error_desc(rc));
    return;
  }

  if (rc != CASS_OK || cass_future_ready(slots.future)) {
    /* Belt-and-suspenders if the driver did not synchronously fire the callback
       for an already-resolved future; the CAS in reg_enqueue dedups a real
       synchronous fire. */
    reg_enqueue(reg);
  }
}

ZEND_METHOD(Cassandra_Async_Reactor, poll)
{
  zend_long max      = 0;
  bool      max_null = true;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG_OR_NULL(max, max_null)
  ZEND_PARSE_PARAMETERS_END();

  if (!max_null && max < 1) {
    zend_argument_value_error(1, "must be greater than 0, or null for every completion");
    return;
  }

  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return;
  }

  array_init(return_value);
  php_scylladb_notifier_drain(reactor->fd); /* drain BEFORE splicing — no lost wakeup */
  php_scylladb_reg* head = reactor_splice(reactor);
  head = reactor_consume(reactor, head, return_value, max_null ? 0 : (uint32_t)max);

  if (head != nullptr) {
    reactor_requeue(reactor, head);
  }

  /* A throwing callback discards return_value, so put back whatever this batch
     had already collected. Runs after the remainder so the reclaimed futures end
     up ahead of it, in completion order. */
  if (EG(exception) && zend_hash_num_elements(Z_ARRVAL_P(return_value)) > 0) {
    reactor_reclaim(reactor, return_value);
  }
}

ZEND_METHOD(Cassandra_Async_Reactor, pending)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return;
  }
  RETURN_LONG((zend_long)reactor->pending);
}
