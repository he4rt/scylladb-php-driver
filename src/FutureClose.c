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

#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include "FutureUtil.h"

#include "FutureClose_arginfo.h"

extern zend_object_handlers php_scylladb_future_close_handlers;

ZEND_METHOD(Cassandra_FutureClose, get)
{
  zval *timeout = NULL;
  php_scylladb_future_close *self = NULL;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_FUTURE_CLOSE(getThis());

  if (php_scylladb_future_wait_timed(self->future, timeout) == FAILURE)
    return;

  if (php_scylladb_future_is_error(self->future) == FAILURE)
    return;
}

HashTable *php_scylladb_future_close_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

int php_scylladb_future_close_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_future_close_free(zend_object *object)
{
  php_scylladb_future_close *self = php_scylladb_future_close_object_fetch(object);

  if (self->future)
    cass_future_free(self->future);

  zend_object_std_dtor(&self->zendObject);
}

zend_object *php_scylladb_future_close_new(zend_class_entry *ce)
{
  php_scylladb_future_close *self = (php_scylladb_future_close *)ecalloc(1, sizeof(php_scylladb_future_close) + zend_object_properties_size(ce));

  self->future = NULL;

  zend_object_std_init(&self->zendObject, ce);
  self->zendObject.handlers = &php_scylladb_future_close_handlers;
  return &self->zendObject;
}
