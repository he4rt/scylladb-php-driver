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

#include <php.h>

/*
 * Shared "reactor": one eventfd/pipe + a completion queue shared across many
 * in-flight futures, so an event loop watches O(1) fds regardless of how many
 * queries are outstanding (see docs/async.md). Opt-in high-concurrency mode; the
 * per-future Future::getResource() path is unchanged.
 *
 * The reactor lives in module globals (per-thread under ZTS), created lazily.
 * Its eventfd + mutex persist for the process/thread lifetime (created once,
 * freed at GSHUTDOWN), while its request-scoped registration state — pinned
 * FutureRows and the php_stream resource — is reset every RSHUTDOWN. This header
 * exposes only what the module lifecycle needs; the PHP-facing
 * Cassandra\Async\Reactor methods and all queue internals live in Reactor.c.
 */
typedef struct php_scylladb_reactor_ php_scylladb_reactor;

/* Create a reactor (its shared fd + mutex). Returns nullptr on failure. */
php_scylladb_reactor* php_scylladb_reactor_create(void);

/* RSHUTDOWN: force any outstanding futures to complete (so no driver callback
 * fires later), release the pinned futures, and drop the cached stream — but
 * KEEP the eventfd + mutex for reuse next request. Null-safe. */
void php_scylladb_reactor_reset(php_scylladb_reactor* reactor);

/* GSHUTDOWN (process/thread end): reset, then free the eventfd + mutex and the
 * reactor itself (drops the module-globals reference). Null-safe. */
void php_scylladb_reactor_destroy(php_scylladb_reactor* reactor);
