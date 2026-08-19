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
#include "FutureNotifier.h"

#include "FutureValue_arginfo.h"

extern zend_object_handlers php_scylladb_future_value_handlers;

ZEND_METHOD(Cassandra_FutureValue, __construct)
{
}

ZEND_METHOD(Cassandra_FutureValue, get)
{
  zval *timeout = nullptr;
  php_scylladb_future_value *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_FUTURE_VALUE(getThis());

  if (!Z_ISUNDEF(self->value)) {
    RETURN_ZVAL(&self->value, 1, 0);
  }
}

ZEND_METHOD(Cassandra_FutureValue, getResource)
{
  ZEND_PARSE_PARAMETERS_NONE();

  /* FutureValue is always already resolved — hand back a readable stream. */
  auto self = PHP_SCYLLADB_GET_FUTURE_VALUE(getThis());

  if (!php_scylladb_future_claim_notifier(Z_OBJ_P(getThis()), self->reactor_reg)) {
    return;
  }

  php_scylladb_future_get_ready_resource(&self->notifier, &self->notify_stream, return_value);
}

ZEND_METHOD(Cassandra_FutureValue, isReady)
{
  ZEND_PARSE_PARAMETERS_NONE();

  RETURN_TRUE;
}

HashTable *php_scylladb_future_value_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

int php_scylladb_future_value_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_future_value_free(zend_object *object)
{
  auto self = php_scylladb_future_value_object_fetch(object);

  zval_ptr_dtor(&self->value);

  if (!Z_ISUNDEF(self->notify_stream)) {
    zval_ptr_dtor(&self->notify_stream);
    ZVAL_UNDEF(&self->notify_stream);
  }
  php_scylladb_notifier_unref(self->notifier);
  self->notifier = nullptr;

  zend_object_std_dtor(&self->zendObject);
}

zend_object *php_scylladb_future_value_new(zend_class_entry *ce)
{
  php_scylladb_future_value *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_future_value, ce, &php_scylladb_future_value_handlers);

  self->notifier    = nullptr;
  self->reactor_reg = nullptr;
  ZVAL_UNDEF(&self->value);
  ZVAL_UNDEF(&self->notify_stream);
  return &self->zendObject;
}
