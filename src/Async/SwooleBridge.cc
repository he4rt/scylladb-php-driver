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
 * Native (Open)Swoole coroutine bridge — the ONLY C++ translation unit in this
 * extension, and it exists solely because Swoole's coroutine wait API is C++.
 * It is compiled only when -DPHP_SCYLLADB_ENABLE_SWOOLE / _OPENSWOOLE is set,
 * so the pure-C default build never sees it.
 *
 * OpenSwoole is a fork of Swoole that keeps the same `swoole::` C++ namespace
 * for its coroutine core, so a single shim serves both; the build flags only
 * select which source tree's headers are on the include path.
 */

#include <swoole.h>
#include <swoole_coroutine.h>
#include <swoole_coroutine_system.h>

#include "SwooleBridge.h"

long
php_scylladb_swoole_current_cid(void)
{
  return swoole::Coroutine::get_current_cid();
}

int
php_scylladb_swoole_wait_readable(int fd, double timeout)
{
  /* Suspends only the calling coroutine; the scheduler keeps running every
     other coroutine until `fd` is readable (or the timeout elapses). */
  return swoole::coroutine::System::wait_event(fd, SW_EVENT_READ, timeout);
}
