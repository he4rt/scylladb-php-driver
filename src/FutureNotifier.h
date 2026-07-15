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
#pragma once

#include <cassandra.h>
#include <php.h>

/*
 * Event-loop notification for CassFuture completion (the "self-pipe" trick).
 *
 * The cpp-driver resolves a CassFuture on one of its internal libuv IO threads.
 * To let a userland PHP event loop (ReactPHP / AMPHP / Swoole / stream_select)
 * await a query without blocking, we hand the loop a readable php_stream that
 * becomes readable exactly once when the future resolves.
 *
 * A `php_scylladb_notifier` owns a pipe. The driver callback (running on the IO
 * thread) writes one byte to the write end; the read end is wrapped in a
 * php_stream handed to userland.
 *
 * Ownership split (never crossed):
 *   - the php_stream exclusively owns and close()s the READ fd on resource GC;
 *   - the notifier exclusively owns and close()s the WRITE fd on last unref.
 *
 * The notifier is reference counted between (a) the PHP Future object and
 * (b) the registered driver callback. It is allocated with persistent malloc
 * (NOT the Zend allocator) because the last unref may run on the driver IO
 * thread, where emalloc/efree are unsafe. The callback only write()s an fd and
 * atomic-decrements the refcount; it never touches any zend/zval state.
 */
typedef struct php_scylladb_notifier_ php_scylladb_notifier;

/* Create a notifier backed by a non-blocking, close-on-exec pipe.
 * Returns nullptr on failure (errno set). */
php_scylladb_notifier* php_scylladb_notifier_create(void);

/* Atomic refcount. unref frees the notifier (closing the write fd) on the last
 * reference; safe to call from any thread. */
void php_scylladb_notifier_addref(php_scylladb_notifier* notifier);
void php_scylladb_notifier_unref(php_scylladb_notifier* notifier);

/* The driver-facing completion callback (CassFutureCallback). Writes one byte
 * to the notifier's write fd, then drops the callback's reference. */
void php_scylladb_notify_cb(CassFuture* future, void* data);

/* Signal readability on the notifier's fd (used by the shared reactor, which
 * shares one notifier across many futures). Thread-safe; never touches PHP. */
void php_scylladb_notifier_signal(php_scylladb_notifier* notifier);

/* Drain the notifier's fd back to non-readable (level-triggered reset). Call on
 * the PHP thread after consuming completions so the watched stream settles. */
void php_scylladb_notifier_drain(php_scylladb_notifier* notifier);

/* Wrap the notifier's read end in a cached, unbuffered php_stream resource
 * (its own dup of the fd) and return it. Returns SUCCESS or FAILURE (thrown). */
int php_scylladb_notifier_publish(php_scylladb_notifier* notifier,
                                  zval*                  stream_slot,
                                  zval*                  return_value);

/*
 * Lazily materialise a notification stream for `future`.
 *
 * On the first call: creates a notifier (stored in *notifier_slot, owned by the
 * caller / Future object), registers the driver callback, wraps the read end in
 * a php_stream cached in *stream_slot, and copies that resource into
 * return_value. Subsequent calls return the cached resource.
 *
 * Uses the register-then-recheck pattern: after registering the callback it
 * re-checks cass_future_ready() and self-writes the wakeup byte if the future
 * already resolved, so no completion is ever missed regardless of whether the
 * driver invokes the callback synchronously for an already-set future.
 *
 * Returns SUCCESS or FAILURE (with a thrown exception).
 */
int php_scylladb_future_get_resource(CassFuture* future,
                                     php_scylladb_notifier** notifier_slot,
                                     zval* stream_slot,
                                     zval* return_value);

/*
 * Like the above but for an already-resolved future with no CassFuture
 * (e.g. FutureValue): returns an eagerly-readable stream so userland can treat
 * every Future subtype through one "on readable -> get()" code path.
 */
int php_scylladb_future_get_ready_resource(php_scylladb_notifier** notifier_slot,
                                           zval* stream_slot,
                                           zval* return_value);

/* Wraps cass_future_ready(); false when still pending. */
bool php_scylladb_future_is_ready(CassFuture* future);

/*
 * Wait for `future` to resolve, cooperating with a running (Open)Swoole
 * coroutine when the extension was built with -DPHP_SCYLLADB_ENABLE_SWOOLE.
 *
 * When built with native Swoole support AND the caller is inside a coroutine
 * AND the (open)swoole module is loaded, this lazily materialises the notifier
 * (creating *notifier_slot if needed) and suspends ONLY the current coroutine
 * on the notification fd — the scheduler keeps running other coroutines. In
 * every other case (no Swoole build, not in a coroutine, module not loaded) it
 * falls back to the blocking php_scylladb_future_wait_timed(), so behaviour is
 * identical to before for non-Swoole builds.
 *
 * Returns SUCCESS or FAILURE (with a thrown exception).
 */
int php_scylladb_future_wait_coro(CassFuture*             future,
                                  php_scylladb_notifier** notifier_slot,
                                  zval*                   timeout);
