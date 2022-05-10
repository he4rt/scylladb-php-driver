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

#include <Futures/FutureValue.h>
#include <cassandra_driver.h>

#include "FutureValue_arginfo.h"

zend_class_entry* php_driver_future_value_ce = NULL;

ZEND_METHOD(Cassandra_FutureValue, get)
{
  zval* timeout                 = NULL;
  php_driver_future_value* self = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &timeout) == FAILURE)
    return;

  self = PHP_DRIVER_FUTURE_VALUE_THIS();

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->value)) {
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->value), 1, 0);
  }
}

static zend_object_handlers php_driver_future_value_handlers;

static HashTable*
php_driver_future_value_properties(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object
#else
  zval* object
#endif
)
{
  HashTable* props = zend_std_get_properties(object);

  return props;
}

static int
php_driver_future_value_compare(zval* obj1, zval* obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_future_value_free(zend_object* object)
{
  php_driver_future_value* self =
    PHP5TO7_ZEND_OBJECT_GET(future_value, object);

  PHP5TO7_ZVAL_MAYBE_DESTROY(self->value);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_future_value_new(zend_class_entry* ce)
{
  php_driver_future_value* self = make(php_driver_future_value);

  PHP5TO7_ZVAL_UNDEF(self->value);

  PHP5TO7_ZEND_OBJECT_INIT(future_value, self, ce);
}

void
php_driver_define_FutureValue(zend_class_entry* future_interface)
{
  php_driver_future_value_ce = register_class_Cassandra_FutureValue(future_interface);

  php_driver_future_value_ce->create_object = php_driver_future_value_new;

  memcpy(&php_driver_future_value_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_future_value_handlers.get_properties = php_driver_future_value_properties;
  php_driver_future_value_handlers.compare        = php_driver_future_value_compare;
  php_driver_future_value_handlers.clone_obj      = NULL;
}
