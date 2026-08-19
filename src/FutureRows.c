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

#include "php_scylladb.h"
#include "php_scylladb_types.h"
#include "FutureUtil.h"
#include "FutureNotifier.h"
#include "Database/ResultDecoder.h"

#include "FutureRows_arginfo.h"

extern zend_object_handlers php_scylladb_future_rows_handlers;

int php_scylladb_future_rows_get_result(php_scylladb_future_rows *future_rows, zval *timeout)
{
  if (!future_rows->result) {
    const CassResult *result = nullptr;

    if (php_scylladb_future_wait_coro(future_rows->future, &future_rows->notifier,
                                      future_rows->reactor_reg, timeout) == FAILURE) {
      return FAILURE;
    }

    if (php_scylladb_future_is_error(future_rows->future) == FAILURE) {
      return FAILURE;
    }

    result = cass_future_get_result(future_rows->future);
    if (!result) {
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                              "Future doesn't contain a result.");
      return FAILURE;
    }

    future_rows->result = result;
  }

  return SUCCESS;
}

ZEND_METHOD(Cassandra_FutureRows, __construct)
{
}

ZEND_METHOD(Cassandra_FutureRows, get)
{
  zval *timeout = nullptr;
  php_scylladb_rows *rows = nullptr;

  auto self = PHP_SCYLLADB_GET_FUTURE_ROWS(getThis());

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  if (php_scylladb_future_rows_get_result(self, timeout) == FAILURE) {
    return;
  }

  if (Z_ISUNDEF(self->rows)) {
    if (php_scylladb_get_result(self->result, &self->rows) == FAILURE) {
      zval_ptr_dtor(&self->rows);
      return;
    }
  }

  object_init_ex(return_value, php_scylladb_rows_ce);
  rows = PHP_SCYLLADB_GET_ROWS(return_value);

  ZVAL_COPY(&rows->rows, &self->rows);

  if (cass_result_has_more_pages(self->result)) {
    /* Share statement (refcounted resource) and transfer result
       ownership to the new Rows. session is a zval — refcount via copy. */
    ZVAL_COPY(&rows->session, &self->session);
    GC_ADDREF(self->statement);
    rows->statement = self->statement;
    rows->result    = self->result;
    self->result    = nullptr;   /* transferred to Rows */
  }
}

ZEND_METHOD(Cassandra_FutureRows, getResource)
{
  ZEND_PARSE_PARAMETERS_NONE();

  auto self = PHP_SCYLLADB_GET_FUTURE_ROWS(getThis());

  if (!php_scylladb_future_claim_notifier(Z_OBJ_P(getThis()), self->reactor_reg)) {
    return;
  }

  php_scylladb_future_get_resource(self->future, &self->notifier, &self->notify_stream, return_value);
}

ZEND_METHOD(Cassandra_FutureRows, isReady)
{
  ZEND_PARSE_PARAMETERS_NONE();

  auto self = PHP_SCYLLADB_GET_FUTURE_ROWS(getThis());
  RETURN_BOOL(php_scylladb_future_is_ready(self->future));
}

HashTable *php_scylladb_future_rows_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

int php_scylladb_future_rows_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_future_rows_free(zend_object *object)
{
  auto self = php_scylladb_future_rows_object_fetch(object);

  zval_ptr_dtor(&self->rows);

  if (self->statement) {
    zend_list_delete(self->statement);
    self->statement = nullptr;
  }
  if (self->result) {
    cass_result_free((CassResult *)self->result);
    self->result = nullptr;
  }
  if (!Z_ISUNDEF(self->session)) {
    zval_ptr_dtor(&self->session);
    ZVAL_UNDEF(&self->session);
  }

  if (self->future) {
    cass_future_free(self->future);
  }

  /* Drop the notification stream (closes the read fd) and our notifier ref;
     a late driver callback only touches the refcounted notifier, never this
     object. */
  if (!Z_ISUNDEF(self->notify_stream)) {
    zval_ptr_dtor(&self->notify_stream);
    ZVAL_UNDEF(&self->notify_stream);
  }
  php_scylladb_notifier_unref(self->notifier);
  self->notifier = nullptr;

  zend_object_std_dtor(&self->zendObject);
}

zend_object *php_scylladb_future_rows_new(zend_class_entry *ce)
{
  php_scylladb_future_rows *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_future_rows, ce, &php_scylladb_future_rows_handlers);

  self->future      = nullptr;
  self->statement   = nullptr;
  self->result      = nullptr;
  self->notifier    = nullptr;
  self->reactor_reg = nullptr;
  ZVAL_UNDEF(&self->session);
  ZVAL_UNDEF(&self->rows);
  ZVAL_UNDEF(&self->notify_stream);
  return &self->zendObject;
}
