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

#include "src/Collection.h"

#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include "Type/Conversions.h"
#include "Type/TypeFactory.h"
#include <zend_smart_str.h>

#include "Collection_arginfo.h"

extern zend_object_handlers php_scylladb_type_collection_handlers;
ZEND_METHOD(Cassandra_Type_Collection, __construct) {
  zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0,
                          "Instantiation of a " PHP_SCYLLADB_NAMESPACE
                          "\\Type\\Collection type is not supported.");
  return;
}

ZEND_METHOD(Cassandra_Type_Collection, name) {
  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  RETVAL_STRING("list");
}

ZEND_METHOD(Cassandra_Type_Collection, valueType) {
  php_scylladb_type* self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());
  RETURN_ZVAL(&self->data.collection.value_type, 1, 0);
}

ZEND_METHOD(Cassandra_Type_Collection, __toString) {
  php_scylladb_type* self;
  smart_str string = {nullptr,0};

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());

  php_scylladb_type_string(self, &string);
  smart_str_0(&string);

  RETVAL_STRING(ZSTR_VAL(string.s));
  smart_str_free(&string);
}

ZEND_METHOD(Cassandra_Type_Collection, create) {
  php_scylladb_type* self;
  php_scylladb_collection* collection;
  zval* args = nullptr;
  int argc = 0, i;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TYPE(getThis());

  object_init_ex(return_value, php_scylladb_collection_ce);
  collection = PHP_SCYLLADB_GET_COLLECTION(return_value);

  ZVAL_COPY(&collection->type, getThis());

  if (argc > 0) {
    for (i = 0; i < argc; i++) {
      if (!php_scylladb_validate_object(
              &args[i],
              &self->data.collection.value_type)) {

        return;
      }

      php_scylladb_collection_add(collection, &args[i]);
    }

  }
}

HashTable* php_scylladb_type_collection_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object* object,
#else
    zendObject* object,
#endif
    zval** table, int* n) {
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable* php_scylladb_type_collection_properties(
#if PHP_MAJOR_VERSION >= 8
    zend_object* object
#else
    zendObject* object
#endif
) {
#if PHP_MAJOR_VERSION >= 8
  auto self = php_scylladb_type_object_fetch(object);
#else
  auto self = PHP_SCYLLADB_GET_TYPE(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(1);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, ZEND_STRL("valueType"), &self->data.collection.value_type);
  Z_ADDREF_P(&self->data.collection.value_type);

  return props;
}

int php_scylladb_type_collection_compare(zval* obj1, zval* obj2) {
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  auto type1 = PHP_SCYLLADB_GET_TYPE(obj1);
  auto type2 = PHP_SCYLLADB_GET_TYPE(obj2);

  return php_scylladb_type_compare(type1, type2);
}

void php_scylladb_type_collection_free(zend_object* object) {
  auto self = php_scylladb_type_object_fetch(object);

  if (self->data_type) cass_data_type_free(self->data_type);
  zval_ptr_dtor(&self->data.collection.value_type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object* php_scylladb_type_collection_new(
    zend_class_entry* ce) {
  php_scylladb_type* self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_type, ce, &php_scylladb_type_collection_handlers);

  self->type = CASS_VALUE_TYPE_LIST;
  self->data_type = cass_data_type_new(self->type);
  ZVAL_UNDEF(&self->data.collection.value_type);

  php_scylladb_type_collection_handlers.offset = XtOffsetOf(php_scylladb_type, zendObject);
  php_scylladb_type_collection_handlers.free_obj = php_scylladb_type_collection_free;
  return &self->zendObject;
}


