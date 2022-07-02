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

#include <Exception/Exceptions.h>
#include <cassandra.h>
#include <php.h>

#include "php_driver.h"

int
php_driver_future_wait_timed(CassFuture* future, zval* timeout)
{
  if (cass_future_ready(future))
    return SUCCESS;

  if (timeout == NULL || Z_TYPE_P(timeout) == IS_NULL || Z_TYPE_P(timeout) == IS_UNDEF) {
    cass_future_wait(future);
  } else {
    cass_duration_t timeout_us;

    if ((Z_TYPE_P(timeout) == IS_LONG && Z_LVAL_P(timeout) > 0)) {
      timeout_us = Z_LVAL_P(timeout) * 1000000;
    } else if ((Z_TYPE_P(timeout) == IS_DOUBLE && Z_DVAL_P(timeout) > 0)) {
      timeout_us = ceil(Z_DVAL_P(timeout) * 1000000);
    } else {
      INVALID_ARGUMENT_VALUE(timeout, "an positive number of seconds or null", FAILURE);
    }

    if (!cass_future_wait_timed(future, timeout_us)) {
      zend_throw_exception_ex(phpDriverTimeoutExceptionCe, 0,
                              "Future hasn't resolved within %lu microseconds", timeout_us);
      return FAILURE;
    }
  }

  return SUCCESS;
}

int
php_driver_future_is_error(CassFuture* future)
{
  int rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    const char* message;
    size_t message_len;
    cass_future_error_message(future, &message, &message_len);
    zend_throw_exception_ex(exception_class(rc), rc,
                            "%.*s", (int) message_len, message);
    return FAILURE;
  }
  return SUCCESS;
}
