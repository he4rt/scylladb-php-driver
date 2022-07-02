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

#include "php_driver.h"
#include "php_driver_types.h"

#include <CassandraDriver.h>
#include <Futures/FutureRows.h>
#include <zend_exceptions.h>

#include "util/FutureInterface.h"
#include "util/ref.h"
#include "util/result.h"

#include "FutureRows_arginfo.h"

zend_class_entry* php_driver_future_rows_ce = NULL;

static void
free_result(void* result)
{
  cass_result_free((CassResult*) result);
}

int
php_driver_future_rows_get_result(php_driver_future_rows* future_rows, zval* timeout)
{
  if (!future_rows->result) {
    const CassResult* result = NULL;

    if (php_driver_future_wait_timed(future_rows->future, timeout) == FAILURE) {
      return FAILURE;
    }

    if (php_driver_future_is_error(future_rows->future) == FAILURE) {
      return FAILURE;
    }

    result = cass_future_get_result(future_rows->future);
    if (!result) {
      zend_throw_exception_ex(spl_ce_RuntimeException, 0,
                              "Future doesn't contain a result.");
      return FAILURE;
    }

    future_rows->result = php_driver_new_ref((void*) result, free_result);
  }

  return SUCCESS;
}

ZEND_METHOD(Cassandra_FutureRows, get)
{
  zval* timeout         = NULL;
  php_driver_rows* rows = NULL;

  php_driver_future_rows* self = PHP_DRIVER_FUTURE_ROWS_THIS();

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &timeout) == FAILURE) {
    return;
  }

  if (php_driver_future_rows_get_result(self, timeout) == FAILURE) {
    return;
  }

  if (PHP5TO7_ZVAL_IS_UNDEF(self->rows)) {
    if (php_driver_get_result((const CassResult*) self->result->data,
                              &self->rows)
        == FAILURE) {
      ZVAL_DESTROY(self->rows);
      return;
    }
  }

  object_init_ex(return_value, php_driver_rows_ce);
  rows = PHP_DRIVER_GET_ROWS(return_value);

  PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(rows->rows),
                    PHP5TO7_ZVAL_MAYBE_P(self->rows));

  if (cass_result_has_more_pages((const CassResult*) self->result->data)) {
    rows->session   = php_driver_add_ref(self->session);
    rows->statement = php_driver_add_ref(self->statement);
    rows->result    = php_driver_add_ref(self->result);
  }
}

static zend_object_handlers php_driver_future_rows_handlers;

static HashTable*
php_driver_future_rows_properties(zend_object* object)
{
  HashTable* props = zend_std_get_properties(object);

  return props;
}

static int
php_driver_future_rows_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_future_rows_free(zend_object* object)
{
  php_driver_future_rows* self = PHP_DRIVER_FUTURE_ROWS_OBJECT(object);

  ZVAL_DESTROY(self->rows);

  php_driver_del_ref(&self->statement);
  php_driver_del_peref(&self->session, 1);
  php_driver_del_ref(&self->result);

  if (self->future) {
    cass_future_free(self->future);
  }

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_future_rows_new(zend_class_entry* ce)
{
  php_driver_future_rows* self =
    PHP5TO7_ZEND_OBJECT_ECALLOC(future_rows, ce);

  self->future    = NULL;
  self->statement = NULL;
  self->result    = NULL;
  self->session   = NULL;
  ZVAL_UNDEF(&self->rows);

  PHP5TO7_ZEND_OBJECT_INIT(future_rows, self, ce);
}

void
php_driver_define_FutureRows(zend_class_entry* future_interface)
{
  php_driver_future_rows_ce                = register_class_Cassandra_FutureRows(future_interface);
  php_driver_future_rows_ce->create_object = php_driver_future_rows_new;

  memcpy(&php_driver_future_rows_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_future_rows_handlers.get_properties = php_driver_future_rows_properties;
  php_driver_future_rows_handlers.compare        = php_driver_future_rows_compare;
  php_driver_future_rows_handlers.clone_obj      = NULL;
}
