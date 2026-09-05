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
#include "Type/TypeFactory.h"

#include "Custom_arginfo.h"

extern zend_object_handlers php_scylladb_type_custom_handlers;
ZEND_METHOD(Cassandra_Type_Custom, __construct) {
  zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0,
                          "Instantiation of a " PHP_SCYLLADB_NAMESPACE
                          "\\Type\\Custom type is not supported.");
  RETURN_THROWS();
}

ZEND_METHOD(Cassandra_Type_Custom, name) {
  php_scylladb_type *custom;

  ZEND_PARSE_PARAMETERS_NONE();

  custom = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  RETVAL_STR_COPY(custom->data.custom.class_name);
}

ZEND_METHOD(Cassandra_Type_Custom, __toString) {
  php_scylladb_type *custom;

  ZEND_PARSE_PARAMETERS_NONE();

  custom = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  RETVAL_STR_COPY(custom->data.custom.class_name);
}

ZEND_METHOD(Cassandra_Type_Custom, create) {
  zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0,
                          "Instantiation of a " PHP_SCYLLADB_NAMESPACE
                          "\\Type\\Custom instance is not supported.");
  RETURN_THROWS();
}

HashTable *php_scylladb_type_custom_gc(
    zend_object *object,
    zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *php_scylladb_type_custom_properties(zend_object *object) {
  zval name;

  auto self = php_scylladb_type_object_fetch(object);
  HashTable *props = php_scylladb_properties_rebuild(object, 1);

  ZVAL_STR_COPY(&name, self->data.custom.class_name);

  (void)zend_hash_str_update(props, ZEND_STRL("name"), &name);
  return props;
}

int php_scylladb_type_custom_compare(zval *obj1, zval *obj2) {
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  auto type1 = PHP_SCYLLADB_GET_TYPE(obj1);
  auto type2 = PHP_SCYLLADB_GET_TYPE(obj2);

  return php_scylladb_type_compare(type1, type2);
}

void php_scylladb_type_custom_free(zend_object *object) {
  auto self = php_scylladb_type_object_fetch(object);

  if (self->data_type) cass_data_type_free(self->data_type);
  if (self->data.custom.class_name) {
    zend_string_release(self->data.custom.class_name);
    self->data.custom.class_name = nullptr;
  }

  zend_object_std_dtor(&self->zendObject);

}

zend_object *php_scylladb_type_custom_new(zend_class_entry *ce) {
  php_scylladb_type *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_type, ce, &php_scylladb_type_custom_handlers);

  self->type = CASS_VALUE_TYPE_CUSTOM;
  self->data_type = cass_data_type_new(self->type);
  self->data.custom.class_name = nullptr;

  return &self->zendObject;
}


