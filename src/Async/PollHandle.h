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
 * Cassandra\Async\PollHandle (see Async/PollHandle.c). Present only in a build
 * against PHP >= 8.6 with main/php_poll.h — the whole translation unit and this
 * declaration are compiled out otherwise.
 */

#ifdef HAVE_PHP_POLL_API

#include <php.h>

extern zend_class_entry* php_scylladb_async_poll_handle_ce;

/* Materialise `future_zv`'s notification descriptor as a PollHandle in
 * `return_value`. Shared with Async/Poll.c, which registers it with a context on
 * the caller's behalf. Returns SUCCESS or FAILURE (thrown). */
[[nodiscard]] int php_scylladb_poll_handle_for_future(zval* future_zv, zval* return_value);

#endif
