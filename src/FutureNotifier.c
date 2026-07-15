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

#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#if defined(__linux__)
#include <sys/eventfd.h>
#endif

#include "php_scylladb.h"
#include "php_scylladb_types.h"

#include "FutureNotifier.h"
#include "FutureUtil.h"

/*
 * Backend is chosen at compile time: a Linux eventfd (one fd, 8-byte counter,
 * no SIGPIPE) or a POSIX pipe elsewhere. For eventfd read_fd == write_fd.
 */
struct php_scylladb_notifier_
{
  _Atomic uint32_t refcount;
  int              read_fd;  /* owned here; a duplicate is handed to the php_stream */
  int              write_fd; /* owned here; == read_fd for eventfd */
};

/* Signal readability on the notifier's fd, retrying on EINTR. eventfd needs an
 * 8-byte counter write; a pipe needs a single byte. A full/broken fd
 * (EAGAIN/EPIPE) is harmless — the reader only needs to observe readability,
 * and eventfd never raises SIGPIPE. */
static void
php_scylladb_notifier_poke(const php_scylladb_notifier* notifier)
{
  if (notifier == nullptr || notifier->write_fd < 0) {
    return;
  }

  ssize_t rc;
#if defined(__linux__)
  uint64_t token = 1; /* eventfd requires an 8-byte counter write */
#else
  uint8_t token = 1;
#endif
  do {
    rc = write(notifier->write_fd, &token, sizeof(token));
  } while (rc < 0 && errno == EINTR);
  (void)rc;
}

php_scylladb_notifier*
php_scylladb_notifier_create(void)
{
  int read_fd  = -1;
  int write_fd = -1;

#if defined(__linux__)
  /* Linux fast path: a single eventfd is one syscall (flags included), one fd,
     an 8-byte counter, and — being a counter, not a pipe — never raises SIGPIPE
     when written after its readers close. */
  int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (efd < 0) {
    return nullptr;
  }
  read_fd  = efd;
  write_fd = efd;
#else
  /* POSIX fallback (macOS/BSD: no eventfd, no pipe2). Non-blocking +
     close-on-exec on both ends via fcntl. */
  int fds[2];
  if (pipe(fds) != 0) {
    return nullptr;
  }
  for (int i = 0; i < 2; i++) {
    int flags = fcntl(fds[i], F_GETFL, 0);
    if (flags < 0 || fcntl(fds[i], F_SETFL, flags | O_NONBLOCK) < 0) {
      goto fail;
    }
    int fdflags = fcntl(fds[i], F_GETFD, 0);
    if (fdflags < 0 || fcntl(fds[i], F_SETFD, fdflags | FD_CLOEXEC) < 0) {
      goto fail;
    }
  }
  read_fd  = fds[0];
  write_fd = fds[1];
#endif

  /* Persistent (malloc) allocation: the final unref may run on the driver IO
     thread, where the Zend allocator is unsafe. */
  php_scylladb_notifier* notifier = pemalloc(sizeof(*notifier), 1);
  atomic_init(&notifier->refcount, 1);
  notifier->read_fd  = read_fd;
  notifier->write_fd = write_fd;
  return notifier;

#if !defined(__linux__)
fail: {
  int saved = errno;
  close(fds[0]);
  close(fds[1]);
  errno = saved;
  return nullptr;
}
#endif
}

void
php_scylladb_notifier_addref(php_scylladb_notifier* notifier)
{
  if (notifier == nullptr) {
    return;
  }
  atomic_fetch_add_explicit(&notifier->refcount, 1, memory_order_relaxed);
}

void
php_scylladb_notifier_unref(php_scylladb_notifier* notifier)
{
  if (notifier == nullptr) {
    return;
  }

  if (atomic_fetch_sub_explicit(&notifier->refcount, 1, memory_order_acq_rel) == 1) {
    /* The php_stream owns a *duplicate* of the read end and closes that
       independently. Keeping our own read end open for the notifier's whole
       life guarantees the pipe always has a reader when the driver callback
       writes, so a late callback (fired after the Future object and its stream
       were freed) can never raise SIGPIPE. For eventfd read_fd == write_fd, so
       close it exactly once. */
    if (notifier->write_fd >= 0) {
      close(notifier->write_fd);
    }
    if (notifier->read_fd >= 0 && notifier->read_fd != notifier->write_fd) {
      close(notifier->read_fd);
    }
    pefree(notifier, 1);
  }
}

void
php_scylladb_notify_cb(CassFuture* future, void* data)
{
  (void)future;
  php_scylladb_notifier* notifier = (php_scylladb_notifier*)data;

  /* Signal readability BEFORE dropping our reference: poke() reads the notifier
     fields while we still hold a ref, since unref may free the struct if the
     Future object was already destroyed. */
  php_scylladb_notifier_poke(notifier);
  php_scylladb_notifier_unref(notifier);
}

void
php_scylladb_notifier_signal(php_scylladb_notifier* notifier)
{
  php_scylladb_notifier_poke(notifier);
}

void
php_scylladb_notifier_drain(php_scylladb_notifier* notifier)
{
  if (notifier == nullptr || notifier->read_fd < 0) {
    return;
  }

  /* Level-triggered: read until empty so the watched stream stops signalling.
     One eventfd read returns+resets the counter; a pipe may hold several bytes.
     The fd is non-blocking, so the drain stops at EAGAIN. */
#if defined(__linux__)
  uint64_t buf;
  ssize_t  rc;
  do {
    rc = read(notifier->read_fd, &buf, sizeof(buf));
  } while (rc == (ssize_t)sizeof(buf) || (rc < 0 && errno == EINTR));
#else
  uint8_t buf[256];
  ssize_t rc;
  do {
    rc = read(notifier->read_fd, buf, sizeof(buf));
  } while (rc > 0 || (rc < 0 && errno == EINTR));
#endif
}

/* Wrap the notifier's read end in an unbuffered php_stream resource and publish
 * it into stream_slot / return_value (cached — returns the same resource on
 * repeat calls). The stream gets its own dup of the read fd. Returns SUCCESS or
 * FAILURE (exception thrown). */
int
php_scylladb_notifier_publish(php_scylladb_notifier* notifier,
                              zval*                  stream_slot,
                              zval*                  return_value)
{
  if (!Z_ISUNDEF_P(stream_slot)) {
    ZVAL_COPY(return_value, stream_slot);
    return SUCCESS;
  }

  /* Hand the php_stream a *duplicate* of the read end so it can close its copy
     on GC while the notifier keeps its own read end open (see unref). Both
     duplicates share the same pipe, so the stream stays readable when the
     driver writes the wakeup byte. */
  int stream_fd = fcntl(notifier->read_fd, F_DUPFD_CLOEXEC, 0);
  if (stream_fd < 0) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to duplicate the async notification fd: %s", strerror(errno));
    return FAILURE;
  }

  php_stream* stream = php_stream_fopen_from_fd(stream_fd, "r", nullptr);
  if (stream == nullptr) {
    close(stream_fd);
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to wrap the async notification fd in a stream");
    return FAILURE;
  }

  /* Keep the fd out of PHP's stdio buffering so stream_select() and the event
     loops watch the raw kernel fd (PHP_STREAM_AS_FD_FOR_SELECT). */
  stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;

  php_stream_to_zval(stream, stream_slot);
  ZVAL_COPY(return_value, stream_slot);
  return SUCCESS;
}

/* Lazily create the notifier for `future` (stored in *notifier_slot, owned by
 * the caller), register the driver completion callback, and self-notify if the
 * future already resolved. No-op if the notifier already exists. Returns
 * SUCCESS or FAILURE (exception thrown). */
static int
php_scylladb_notifier_ensure(CassFuture* future, php_scylladb_notifier** notifier_slot)
{
  if (*notifier_slot != nullptr) {
    return SUCCESS;
  }

  if (future == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Received a nullptr future from the driver (allocation failure)");
    return FAILURE;
  }

  php_scylladb_notifier* notifier = php_scylladb_notifier_create();
  if (notifier == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to create async notification pipe: %s", strerror(errno));
    return FAILURE;
  }

  /* The callback owns a reference until it fires (rule: refcount the notifier,
     never the future). Register BEFORE re-checking readiness. Refcount is now 2
     (creation + callback). */
  php_scylladb_notifier_addref(notifier);
  CassError rc = cass_future_set_callback(future, php_scylladb_notify_cb, notifier);

  /* Register-then-recheck: if the future already resolved (or the callback
     could not be registered), poke the pipe ourselves. Idempotent — an extra
     wakeup is harmless because userland re-checks via get()/isReady(). Poke
     BEFORE dropping the callback ref so the notifier is unambiguously alive. */
  if (rc != CASS_OK || cass_future_ready(future)) {
    php_scylladb_notifier_poke(notifier);
  }

  if (rc != CASS_OK) {
    /* Callback will never fire (error or CASS_ERROR_LIB_CALLBACK_ALREADY_SET) and
       set_callback failing means no IO thread ever held a ref, so drop the
       callback ref directly. Refcount is exactly 2 here → 1: the creation ref
       always survives, so this can never free — decrement inline (not via unref)
       so static analysis doesn't model an impossible free of the pointer we
       return in *notifier_slot below. */
    atomic_fetch_sub_explicit(&notifier->refcount, 1, memory_order_acq_rel);
  }

  *notifier_slot = notifier; /* Future object keeps the creation reference */
  return SUCCESS;
}

int
php_scylladb_future_get_resource(CassFuture*             future,
                                 php_scylladb_notifier** notifier_slot,
                                 zval*                   stream_slot,
                                 zval*                   return_value)
{
  if (!Z_ISUNDEF_P(stream_slot)) {
    ZVAL_COPY(return_value, stream_slot);
    return SUCCESS;
  }

  if (php_scylladb_notifier_ensure(future, notifier_slot) == FAILURE) {
    return FAILURE;
  }

  return php_scylladb_notifier_publish(*notifier_slot, stream_slot, return_value);
}

int
php_scylladb_future_get_ready_resource(php_scylladb_notifier** notifier_slot,
                                       zval*                   stream_slot,
                                       zval*                   return_value)
{
  if (!Z_ISUNDEF_P(stream_slot)) {
    ZVAL_COPY(return_value, stream_slot);
    return SUCCESS;
  }

  php_scylladb_notifier* notifier = php_scylladb_notifier_create();
  if (notifier == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to create async notification pipe: %s", strerror(errno));
    return FAILURE;
  }

  if (php_scylladb_notifier_publish(notifier, stream_slot, return_value) == FAILURE) {
    php_scylladb_notifier_unref(notifier);
    return FAILURE;
  }

  /* Already resolved: make the stream immediately readable. */
  php_scylladb_notifier_poke(notifier);

  *notifier_slot = notifier;
  return SUCCESS;
}

bool
php_scylladb_future_is_ready(CassFuture* future)
{
  return future != nullptr && cass_future_ready(future);
}

#ifdef HAVE_SWOOLE_COROUTINE
#include "Async/SwooleBridge.h"

/* True only when an (open)swoole module is loaded AND we are inside a coroutine.
 * The module check runs first so the swoole C++ symbols in the bridge are only
 * dereferenced when swoole is actually present.
 *
 * The "is (open)swoole loaded" answer cannot change after module startup, so it
 * is resolved once and cached — every subsequent get() then costs only a cheap
 * get_current_cid() rather than two hashtable lookups. (-1 = unknown, 0 = no,
 * 1 = yes; a benign race between threads recomputes the same value.) */
static int php_scylladb_swoole_loaded = -1;

static bool
php_scylladb_swoole_active(void)
{
  if (php_scylladb_swoole_loaded < 0) {
    php_scylladb_swoole_loaded =
        (zend_hash_str_exists(&module_registry, "swoole", sizeof("swoole") - 1) ||
         zend_hash_str_exists(&module_registry, "openswoole", sizeof("openswoole") - 1))
            ? 1
            : 0;
  }

  if (php_scylladb_swoole_loaded == 0) {
    return false;
  }

  return php_scylladb_swoole_current_cid() >= 0;
}

/* Convert the PHP timeout zval (seconds; null/non-positive => wait forever) to
 * the double seconds Swoole's wait_event expects (negative = infinite). */
static double
php_scylladb_timeout_seconds(zval* timeout)
{
  if (timeout == nullptr || Z_TYPE_P(timeout) == IS_NULL || Z_TYPE_P(timeout) == IS_UNDEF) {
    return -1.0;
  }
  if (Z_TYPE_P(timeout) == IS_LONG && Z_LVAL_P(timeout) > 0) {
    return (double)Z_LVAL_P(timeout);
  }
  if (Z_TYPE_P(timeout) == IS_DOUBLE && Z_DVAL_P(timeout) > 0) {
    return Z_DVAL_P(timeout);
  }
  return -1.0;
}
#endif

int
php_scylladb_future_wait_coro(CassFuture*             future,
                              php_scylladb_notifier** notifier_slot,
                              zval*                   timeout)
{
#ifdef HAVE_SWOOLE_COROUTINE
  if (future != nullptr && !cass_future_ready(future) && php_scylladb_swoole_active()) {
    if (php_scylladb_notifier_ensure(future, notifier_slot) == FAILURE) {
      return FAILURE;
    }

    /* Suspend only this coroutine on the notification fd; the scheduler keeps
       running everything else until the driver resolves the future. */
    php_scylladb_swoole_wait_readable((*notifier_slot)->read_fd,
                                      php_scylladb_timeout_seconds(timeout));

    if (!cass_future_ready(future)) {
      zend_throw_exception_ex(php_scylladb_timeout_exception_ce, 0,
                              "Future hasn't resolved (coroutine wait timed out or was cancelled)");
      return FAILURE;
    }
    return SUCCESS;
  }
#endif

  return php_scylladb_future_wait_timed(future, timeout);
}
