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

/* This file is added to the build ONLY when -DPHP_SCYLLADB_ENABLE_SWOOLE /
   _OPENSWOOLE is set (see src/CMakeLists.txt), which also puts the (open)swoole
   source headers on the include path. It is therefore never compiled by the
   default build. Standalone tooling that scans all sources (clang-tidy) skips it
   explicitly — it cannot resolve <swoole.h> without the swoole source tree. */

/* Select the runtime from the include path (keyed on the source tree passed via
   PHP_SCYLLADB_SWOOLE_SRC), not a compile define. OpenSwoole renamed its headers
   to openswoole_*.h, its C++ namespace to `openswoole`, and its event flag to
   OSW_EVENT_READ; upstream Swoole keeps swoole_*.h / `swoole` / SW_EVENT_READ.
   Both expose the same Coroutine::get_current_cid() and
   coroutine::System::wait_event() surface, so a namespace alias covers both. */
#if defined(__has_include) && __has_include(<openswoole.h>)
#  include <openswoole.h>
#  include <openswoole_coroutine.h>
#  include <openswoole_coroutine_system.h>
namespace cass_swoole = openswoole;
#  define CASS_SWOOLE_EVENT_READ OSW_EVENT_READ
#else
#  include <swoole.h>
#  include <swoole_coroutine.h>
#  include <swoole_coroutine_system.h>
namespace cass_swoole = swoole;
#  define CASS_SWOOLE_EVENT_READ SW_EVENT_READ
#endif

#include "SwooleBridge.h"

long
php_scylladb_swoole_current_cid(void)
{
  return cass_swoole::Coroutine::get_current_cid();
}

int
php_scylladb_swoole_wait_readable(int fd, double timeout)
{
  /* Suspends only the calling coroutine; the scheduler keeps running every
     other coroutine until `fd` is readable (or the timeout elapses). */
  return cass_swoole::coroutine::System::wait_event(fd, CASS_SWOOLE_EVENT_READ, timeout);
}
