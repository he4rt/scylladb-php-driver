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
BEGIN_EXTERN_C()
#include "FutureValue_arginfo.h"

zend_class_entry *php_driver_future_value_ce = NULL;

ZEND_METHOD(Cassandra_FutureValue, get)
{
  zval *timeout = NULL;
  php_driver_future_value *self = NULL;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_FUTURE_VALUE(getThis());

  if (!Z_ISUNDEF(self->value)) {
    RETURN_ZVAL(&self->value, 1, 0);
  }
}

static zend_object_handlers php_driver_future_value_handlers;

static HashTable *php_driver_future_value_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

static int php_driver_future_value_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj2);
}

static void php_driver_future_value_free(zend_object *object)
{
  php_driver_future_value *self = php_driver_future_value_object_fetch(object);

  zval_ptr_dtor(&self->value);

  zend_object_std_dtor(&self->zendObject);
}

static zend_object *php_driver_future_value_new(zend_class_entry *ce)
{
  php_driver_future_value *self = (php_driver_future_value *)ecalloc(
      1, sizeof(php_driver_future_value) + zend_object_properties_size(ce));

  ZVAL_UNDEF(&self->value);

  zend_object_std_init(&self->zendObject, ce);
  self->zendObject.handlers = &php_driver_future_value_handlers;
  return &self->zendObject;
}

END_EXTERN_C()

void php_driver_define_FutureValue()
{
  php_driver_future_value_ce = register_class_Cassandra_FutureValue(php_driver_future_ce);
  php_driver_future_value_ce->create_object = php_driver_future_value_new;

  memcpy(&php_driver_future_value_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_future_value_handlers.offset = XtOffsetOf(php_driver_future_value, zendObject);
  php_driver_future_value_handlers.free_obj = php_driver_future_value_free;
  php_driver_future_value_handlers.get_properties = php_driver_future_value_properties;
  php_driver_future_value_handlers.compare = php_driver_future_value_compare;
  php_driver_future_value_handlers.clone_obj = NULL;
}
