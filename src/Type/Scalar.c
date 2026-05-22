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

#include "Type/TypeFactory.h"

#include "php_scylladb.h"
#include "php_scylladb_types.h"

#include "Scalar_arginfo.h"

extern zend_object_handlers php_scylladb_type_scalar_handlers;

ZEND_METHOD(Cassandra_Type_Scalar, __construct) {
  zend_throw_exception_ex(
      php_scylladb_logic_exception_ce, 0,
      "Instantiation of a " PHP_SCYLLADB_NAMESPACE
      "\\Type\\Scalar objects directly is not "
      "supported, call varchar(), text(), blob(), ascii(), bigint(), "
      "smallint(), tinyint(), counter(), int(), varint(), boolean(), "
      "decimal(), double(), float(), inet(), timestamp(), uuid(), timeuuid(), "
      "map(), collection() or set() on " PHP_SCYLLADB_NAMESPACE
      "\\Type statically instead.");
}

ZEND_METHOD(Cassandra_Type_Scalar, name) {
  php_scylladb_type *self;
  const char *name;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());
  name = php_scylladb_scalar_type_name(self->type);
  RETVAL_STRING(name);
}

ZEND_METHOD(Cassandra_Type_Scalar, __toString) {
  php_scylladb_type *self;
  const char *name;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());
  name = php_scylladb_scalar_type_name(self->type);
  RETVAL_STRING(name);
}

ZEND_METHOD(Cassandra_Type_Scalar, create) {
  php_scylladb_scalar_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}


HashTable *php_scylladb_type_scalar_gc(zend_object *object, zval **table,
                                            int *n) {
  *table = nullptr;
  *n = 0;
  return NULL;
}

HashTable *php_scylladb_type_scalar_properties(zend_object *object) {
  zval name;
  php_scylladb_type *self = php_scylladb_type_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(1);
  HashTable *props = object->properties;

  /* Used for comparison and 'text' is just an alias for 'varchar' */
  CassValueType type =
      self->type == CASS_VALUE_TYPE_TEXT ? CASS_VALUE_TYPE_VARCHAR : self->type;

  ZVAL_STRING(&name,
                      php_scylladb_scalar_type_name(type));
  (void)zend_hash_str_update(props, ZEND_STRL("name"), &name);
  return props;
}

int php_scylladb_type_scalar_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)
  php_scylladb_type *type1 = PHP_SCYLLADB_GET_TYPE(obj1);
  php_scylladb_type *type2 = PHP_SCYLLADB_GET_TYPE(obj2);

  return php_scylladb_type_compare(type1, type2);
}

void php_scylladb_type_scalar_free(zend_object *object) {
  php_scylladb_type *self = php_scylladb_type_object_fetch(object);

  if (self->data_type) cass_data_type_free(self->data_type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object* php_scylladb_type_scalar_new(zend_class_entry *ce) {
  php_scylladb_type *self = (php_scylladb_type *)ecalloc(1, sizeof(php_scylladb_type) + zend_object_properties_size(ce));

  self->type = CASS_VALUE_TYPE_UNKNOWN;
  self->data_type = nullptr;

  zend_object_std_init(&self->zendObject, ce);
  php_scylladb_type_scalar_handlers.offset = XtOffsetOf(php_scylladb_type, zendObject);
  php_scylladb_type_scalar_handlers.free_obj = php_scylladb_type_scalar_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_scylladb_type_scalar_handlers;
  return &self->zendObject;
}

