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

/*
 * Native Swoole / OpenSwoole coroutine bridge.
 *
 * Swoole's coroutine wait API is C++-only (swoole::coroutine::System::wait_event
 * and swoole::Coroutine::get_current_cid live in namespaces, and swoole_api.h's
 * extern-"C" surface does not cover them). This header declares a tiny set of
 * C-callable wrappers implemented in the C++ shim src/Async/SwooleBridge.cc.
 *
 * The whole bridge is compiled ONLY when the extension is built with
 * -DPHP_SCYLLADB_ENABLE_SWOOLE=ON (or _OPENSWOOLE=ON), which also defines
 * HAVE_SWOOLE_COROUTINE. Callers must guard use with `#ifdef HAVE_SWOOLE_COROUTINE`.
 *
 * The swoole C++ symbols are referenced only inside the .cc and resolve lazily
 * from the loaded (open)swoole extension at runtime, so callers must first
 * confirm the swoole/openswoole module is loaded before invoking these.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Current coroutine id, or -1 when not running inside a coroutine.
 * Wraps swoole::Coroutine::get_current_cid(). */
long php_scylladb_swoole_current_cid(void);

/* Suspend ONLY the current coroutine until `fd` is readable (SW_EVENT_READ).
 * `timeout` is in seconds; a negative value waits indefinitely. Returns a
 * non-negative value when the fd became readable, or a negative value on
 * timeout/error/cancellation. Wraps swoole::coroutine::System::wait_event(). */
int php_scylladb_swoole_wait_readable(int fd, double timeout);

#ifdef __cplusplus
}
#endif
