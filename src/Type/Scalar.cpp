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

#include <util/types.h>

#include "php_driver.h"
#include "php_driver_types.h"

BEGIN_EXTERN_C()
#include "Scalar_arginfo.h"
zend_class_entry *php_driver_type_scalar_ce = nullptr;

ZEND_METHOD(Cassandra_Type_Scalar, __construct) {
  zend_throw_exception_ex(
      php_driver_logic_exception_ce, 0,
      "Instantiation of a " PHP_DRIVER_NAMESPACE
      "\\Type\\Scalar objects directly is not "
      "supported, call varchar(), text(), blob(), ascii(), bigint(), "
      "smallint(), tinyint(), counter(), int(), varint(), boolean(), "
      "decimal(), double(), float(), inet(), timestamp(), uuid(), timeuuid(), "
      "map(), collection() or set() on " PHP_DRIVER_NAMESPACE
      "\\Type statically instead.");
}

ZEND_METHOD(Cassandra_Type_Scalar, name) {
  php_driver_type *self;
  const char *name;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());
  name = php_driver_scalar_type_name(self->type);
  RETVAL_STRING(name);
}

ZEND_METHOD(Cassandra_Type_Scalar, __toString) {
  php_driver_type *self;
  const char *name;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());
  name = php_driver_scalar_type_name(self->type);
  RETVAL_STRING(name);
}

ZEND_METHOD(Cassandra_Type_Scalar, create) {
  php_driver_scalar_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}


static zend_object_handlers php_driver_type_scalar_handlers;

static HashTable *php_driver_type_scalar_gc(zend_object *object, zval **table,
                                            int *n) {
  *table = nullptr;
  *n = 0;
  return NULL;
}

static HashTable *php_driver_type_scalar_properties(zend_object *object) {
  zval name;
  php_driver_type *self = PHP5TO7_ZEND_OBJECT_GET(type, object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(1);
  HashTable *props = object->properties;

  /* Used for comparison and 'text' is just an alias for 'varchar' */
  CassValueType type =
      self->type == CASS_VALUE_TYPE_TEXT ? CASS_VALUE_TYPE_VARCHAR : self->type;

  ZVAL_STRING(&name,
                      php_driver_scalar_type_name(type));
  (void)zend_hash_str_update(props, "name", sizeof("name") - 1, &name);
  return props;
}

static int php_driver_type_scalar_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)
  php_driver_type *type1 = PHP_DRIVER_GET_TYPE(obj1);
  php_driver_type *type2 = PHP_DRIVER_GET_TYPE(obj2);

  return php_driver_type_compare(type1, type2);
}

static void php_driver_type_scalar_free(zend_object *object) {
  php_driver_type *self = PHP5TO7_ZEND_OBJECT_GET(type, object);

  if (self->data_type) cass_data_type_free(self->data_type);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object* php_driver_type_scalar_new(zend_class_entry *ce) {
  auto self = PHP5TO7_ZEND_OBJECT_ECALLOC(type, ce);

  self->type = CASS_VALUE_TYPE_UNKNOWN;
  self->data_type = nullptr;

  PHP5TO7_ZEND_OBJECT_INIT_EX(type, type_scalar, self, ce);
}

void php_driver_define_TypeScalar() {
  php_driver_type_scalar_ce = register_class_Cassandra_Type_Scalar(php_driver_type_ce);
  memcpy(&php_driver_type_scalar_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_driver_type_scalar_handlers.get_properties =
      php_driver_type_scalar_properties;
  php_driver_type_scalar_handlers.get_gc = php_driver_type_scalar_gc;
  php_driver_type_scalar_handlers.compare = php_driver_type_scalar_compare;
  php_driver_type_scalar_ce->create_object = php_driver_type_scalar_new;
}
END_EXTERN_C()