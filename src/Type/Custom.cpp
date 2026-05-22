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
#include "util/types.h"
BEGIN_EXTERN_C()
#include "Custom_arginfo.h"
zend_class_entry *php_driver_type_custom_ce = NULL;

ZEND_METHOD(Cassandra_Type_Custom, __construct) {
  zend_throw_exception_ex(php_driver_logic_exception_ce, 0,
                          "Instantiation of a " PHP_DRIVER_NAMESPACE
                          "\\Type\\Custom type is not supported.");
  return;
}

ZEND_METHOD(Cassandra_Type_Custom, name) {
  php_driver_type *custom;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  custom = PHP_DRIVER_GET_TYPE(getThis());

  RETVAL_STRING(custom->data.custom.class_name);
}

ZEND_METHOD(Cassandra_Type_Custom, __toString) {
  php_driver_type *custom;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  custom = PHP_DRIVER_GET_TYPE(getThis());

  RETVAL_STRING(custom->data.custom.class_name);
}

ZEND_METHOD(Cassandra_Type_Custom, create) {
  zend_throw_exception_ex(php_driver_logic_exception_ce, 0,
                          "Instantiation of a " PHP_DRIVER_NAMESPACE
                          "\\Type\\Custom instance is not supported.");
  return;
}


static zend_object_handlers php_driver_type_custom_handlers;

static HashTable *php_driver_type_custom_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object,
#else
    zendObject *object,
#endif
    zval **table, int *n) {
  *table = NULL;
  *n = 0;
  return NULL;
}

static HashTable *php_driver_type_custom_properties(zend_object *object) {
  zval name;

  php_driver_type *self = php_driver_type_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(1);
  HashTable *props = object->properties;

  ZVAL_STRING(&name, self->data.custom.class_name);

  (void)zend_hash_str_update(props, ZEND_STRL("name"), &name);
  return props;
}

static int php_driver_type_custom_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)
  php_driver_type *type1 = PHP_DRIVER_GET_TYPE(obj1);
  php_driver_type *type2 = PHP_DRIVER_GET_TYPE(obj2);

  return php_driver_type_compare(type1, type2);
}

static void php_driver_type_custom_free(zend_object *object) {
  php_driver_type *self = php_driver_type_object_fetch(object);

  if (self->data_type) cass_data_type_free(self->data_type);
  if (self->data.custom.class_name) {
    efree(self->data.custom.class_name);
    self->data.custom.class_name = NULL;
  }

  zend_object_std_dtor(&self->zendObject);

}

static zend_object *php_driver_type_custom_new(zend_class_entry *ce) {
  php_driver_type *self = (php_driver_type *)ecalloc(1, sizeof(php_driver_type) + zend_object_properties_size(ce));

  self->type = CASS_VALUE_TYPE_CUSTOM;
  self->data_type = cass_data_type_new(self->type);
  self->data.custom.class_name = NULL;

  zend_object_std_init(&self->zendObject, ce);
  php_driver_type_custom_handlers.offset = XtOffsetOf(php_driver_type, zendObject);
  php_driver_type_custom_handlers.free_obj = php_driver_type_custom_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_type_custom_handlers;
  return &self->zendObject;
}

void php_driver_define_TypeCustom() {
  php_driver_type_custom_ce = register_class_Cassandra_Type_Custom(php_driver_type_ce);
  memcpy(&php_driver_type_custom_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_driver_type_custom_handlers.get_properties = php_driver_type_custom_properties;
  php_driver_type_custom_handlers.get_gc = php_driver_type_custom_gc;
  php_driver_type_custom_handlers.compare = php_driver_type_custom_compare;
  php_driver_type_custom_ce->create_object = php_driver_type_custom_new;
}
END_EXTERN_C()