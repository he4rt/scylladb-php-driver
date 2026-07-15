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
  zend_object*         future_obj; /* pinned FutureRows (GC_ADDREF'd); nulled on dispatch */
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
  return reactor;
}

/* ─── registration records ──────────────────────────────────────────────────── */

/* Create a reg for `future_obj`, pin the object, and link it into the
 * registered-list. Main thread only. */
static php_scylladb_reg*
reg_create(php_scylladb_reactor* reactor, zend_object* future_obj)
{
  php_scylladb_reg* reg = pemalloc(sizeof(*reg), 1);
  atomic_init(&reg->refcount, 2);
  atomic_init(&reg->enqueued, false);
  reg->reactor = reactor;
  php_scylladb_reactor_addref(reactor);

  GC_ADDREF(future_obj); /* pin until dispatched */
  reg->future_obj = future_obj;

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

/* Detach + consume one spliced ready-list. If `out` is non-null, the pinned
 * FutureRows objects are appended to it (dispatch); otherwise their pins are
 * released (RSHUTDOWN discard). Main thread only. */
static void
reactor_consume(php_scylladb_reactor* reactor, php_scylladb_reg* head, zval* out)
{
  while (head) {
    php_scylladb_reg* reg = head;
    head                  = head->next_ready;

    reg_unregister(reactor, reg);

    if (reg->future_obj) {
      php_scylladb_future_rows* rows = php_scylladb_future_rows_object_fetch(reg->future_obj);
      rows->reactor_reg = nullptr; /* detach so getResource()/free_obj are safe again */

      if (out) {
        zval zv;
        ZVAL_OBJ(&zv, reg->future_obj); /* transfer the pin ref into the array */
        add_next_index_zval(out, &zv);
      } else {
        OBJ_RELEASE(reg->future_obj);
      }
      reg->future_obj = nullptr;
    }

    reg_release(reg); /* drop ready-list + registered refs → frees */
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
      if (reg->future_obj) {
        php_scylladb_future_rows* rows = php_scylladb_future_rows_object_fetch(reg->future_obj);
        if (rows->future) {
          cass_future_wait(rows->future);
        }
      }
    }

    php_scylladb_notifier_drain(reactor->fd);
    php_scylladb_reg* head = reactor_splice(reactor);
    if (head) {
      reactor_consume(reactor, head, nullptr);
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
  zval* future_zv = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(future_zv, php_scylladb_future_ce)
  ZEND_PARSE_PARAMETERS_END();

  if (!instanceof_function(Z_OBJCE_P(future_zv), php_scylladb_future_rows_ce)) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "The reactor currently supports only FutureRows (from executeAsync)");
    return;
  }

  php_scylladb_future_rows* rows = PHP_SCYLLADB_GET_FUTURE_ROWS(future_zv);

  if (rows->reactor_reg != nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "This future is already registered with the reactor");
    return;
  }
  if (rows->notifier != nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "This future already exposes a per-future resource (getResource); "
                            "use one async model per future");
    return;
  }
  if (rows->future == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "FutureRows has no underlying future to register");
    return;
  }

  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return;
  }

  php_scylladb_reg* reg = reg_create(reactor, Z_OBJ_P(future_zv));
  rows->reactor_reg     = reg;

  CassError rc = cass_future_set_callback(rows->future, reactor_cb, reg);
  if (rc != CASS_OK) {
    /* Callback will never fire (already-set/error): surface it now so poll()
       returns it and get() reports the driver error. */
    reg_enqueue(reg);
  } else if (cass_future_ready(rows->future)) {
    /* Belt-and-suspenders if the driver did not synchronously fire the callback
       for an already-resolved future; the CAS in reg_enqueue dedups a real
       synchronous fire. */
    reg_enqueue(reg);
  }
}

ZEND_METHOD(Cassandra_Async_Reactor, poll)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_reactor* reactor = reactor_instance();
  if (reactor == nullptr) {
    return;
  }

  array_init(return_value);
  php_scylladb_notifier_drain(reactor->fd); /* drain BEFORE splicing — no lost wakeup */
  php_scylladb_reg* head = reactor_splice(reactor);
  reactor_consume(reactor, head, return_value);
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
