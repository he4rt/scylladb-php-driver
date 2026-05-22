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
#include "FutureUtil.h"
#include "util/result.h"
BEGIN_EXTERN_C()
#include "FutureRows_arginfo.h"

zend_class_entry *php_driver_future_rows_ce = NULL;

int php_driver_future_rows_get_result(php_driver_future_rows *future_rows, zval *timeout)
{
  if (!future_rows->result) {
    const CassResult *result = NULL;

    if (php_driver_future_wait_timed(future_rows->future, timeout) == FAILURE) {
      return FAILURE;
    }

    if (php_driver_future_is_error(future_rows->future) == FAILURE) {
      return FAILURE;
    }

    result = cass_future_get_result(future_rows->future);
    if (!result) {
      zend_throw_exception_ex(php_driver_runtime_exception_ce, 0,
                              "Future doesn't contain a result.");
      return FAILURE;
    }

    future_rows->result = result;
  }

  return SUCCESS;
}

ZEND_METHOD(Cassandra_FutureRows, get)
{
  zval *timeout = NULL;
  php_driver_rows *rows = NULL;

  php_driver_future_rows *self = PHP_DRIVER_GET_FUTURE_ROWS(getThis());

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  if (php_driver_future_rows_get_result(self, timeout) == FAILURE) {
    return;
  }

  if (Z_ISUNDEF(self->rows)) {
    if (php_driver_get_result(self->result, &self->rows) == FAILURE) {
      zval_ptr_dtor(&self->rows);
      return;
    }
  }

  object_init_ex(return_value, php_driver_rows_ce);
  rows = PHP_DRIVER_GET_ROWS(return_value);

  ZVAL_COPY(&rows->rows, &self->rows);

  if (cass_result_has_more_pages(self->result)) {
    /* Share statement (refcounted resource) and transfer result
       ownership to the new Rows. session is a zval — refcount via copy. */
    ZVAL_COPY(&rows->session, &self->session);
    GC_ADDREF(self->statement);
    rows->statement = self->statement;
    rows->result    = self->result;
    self->result    = NULL;   /* transferred to Rows */
  }
}

static zend_object_handlers php_driver_future_rows_handlers;

static HashTable *php_driver_future_rows_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

static int php_driver_future_rows_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

static void php_driver_future_rows_free(zend_object *object)
{
  php_driver_future_rows *self = php_driver_future_rows_object_fetch(object);

  zval_ptr_dtor(&self->rows);

  if (self->statement) {
    zend_list_delete(self->statement);
    self->statement = NULL;
  }
  if (self->result) {
    cass_result_free((CassResult *)self->result);
    self->result = NULL;
  }
  if (!Z_ISUNDEF(self->session)) {
    zval_ptr_dtor(&self->session);
    ZVAL_UNDEF(&self->session);
  }

  if (self->future) {
    cass_future_free(self->future);
  }

  zend_object_std_dtor(&self->zendObject);
}

static zend_object *php_driver_future_rows_new(zend_class_entry *ce)
{
  php_driver_future_rows *self = (php_driver_future_rows *)ecalloc(1, sizeof(php_driver_future_rows) + zend_object_properties_size(ce));

  self->future    = NULL;
  self->statement = NULL;
  self->result    = NULL;
  ZVAL_UNDEF(&self->session);
  ZVAL_UNDEF(&self->rows);

  zend_object_std_init(&self->zendObject, ce);
  php_driver_future_rows_handlers.offset = XtOffsetOf(php_driver_future_rows, zendObject);
  php_driver_future_rows_handlers.free_obj = php_driver_future_rows_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_future_rows_handlers;
  return &self->zendObject;
}

END_EXTERN_C()

void php_driver_define_FutureRows()
{
  php_driver_future_rows_ce = register_class_Cassandra_FutureRows(php_driver_future_ce);
  php_driver_future_rows_ce->create_object = php_driver_future_rows_new;

  memcpy(&php_driver_future_rows_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_future_rows_handlers.get_properties = php_driver_future_rows_properties;
  php_driver_future_rows_handlers.compare = php_driver_future_rows_compare;
  php_driver_future_rows_handlers.clone_obj = NULL;
}
