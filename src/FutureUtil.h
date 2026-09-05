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

/* `timeout` may be nullptr (= wait forever). `future` is nullptr when the driver
 * failed to allocate it, and both functions report that as a PHP exception, so
 * the parameter must not be marked nonnull. */
[[nodiscard]] zend_result php_scylladb_future_wait_timed(CassFuture* future, zval* timeout);
[[nodiscard]] zend_result php_scylladb_future_is_error(CassFuture* future);
