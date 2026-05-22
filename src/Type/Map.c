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

#include <zend_smart_str.h>
#include "src/Map.h"
#include "Map_arginfo.h"

extern zend_object_handlers php_scylladb_type_map_handlers;
ZEND_METHOD(Cassandra_Type_Map, __construct)
{
  zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0 ,
    "Instantiation of a " PHP_SCYLLADB_NAMESPACE "\\Type\\Map type is not supported."
  );
  return;
}

ZEND_METHOD(Cassandra_Type_Map, name)
{
  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  RETVAL_STRING("map");
}

ZEND_METHOD(Cassandra_Type_Map, keyType)
{
  php_scylladb_type *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());
  RETURN_ZVAL(&self->data.map.key_type, 1, 0);
}

ZEND_METHOD(Cassandra_Type_Map, valueType)
{
  php_scylladb_type *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());
  RETURN_ZVAL(&self->data.map.value_type, 1, 0);
}

ZEND_METHOD(Cassandra_Type_Map, __toString)
{
  php_scylladb_type *self;
  smart_str string = {NULL,0};

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_TYPE(getThis());

  php_scylladb_type_string(self, &string );
  smart_str_0(&string);

  RETVAL_STRING(ZSTR_VAL(string.s));
  smart_str_free(&string);
}

ZEND_METHOD(Cassandra_Type_Map, create)
{
  php_scylladb_map *map;
  zval* args = NULL;
  int argc = 0, i;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  if (argc % 2 == 1) {

    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                            "Not enough values, maps can only be created " \
                            "from an even number of values, where each odd " \
                            "value is a key and each even value is a value, " \
                            "e.g create(key, value, key, value, key, value)");
    return;
  }

  object_init_ex(return_value, php_scylladb_map_ce);
  map = PHP_SCYLLADB_GET_MAP(return_value);

  ZVAL_COPY(&map->type, getThis());

  if (argc > 0) {
    for (i = 0; i < argc; i += 2) {
      if (!php_scylladb_map_set(map,
                              &args[i],
                              &args[i + 1] )) {

        return;
      }
    }

  }
}

HashTable *
php_scylladb_type_map_gc(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object,
#else
        zendObject *object,
#endif
        zval** table, int *n
)
{
  *table = NULL;
  *n = 0;
  return NULL;
}

HashTable *
php_scylladb_type_map_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
#if PHP_MAJOR_VERSION >= 8
  php_scylladb_type *self  = php_scylladb_type_object_fetch(object);
#else
  php_scylladb_type *self  = PHP_SCYLLADB_GET_TYPE(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, ZEND_STRL("keyType"), &self->data.map.key_type);
  Z_ADDREF_P(&self->data.map.key_type);

  (void)zend_hash_str_update(props, ZEND_STRL("valueType"), &self->data.map.value_type);
  Z_ADDREF_P(&self->data.map.value_type);

  return props;
}

int
php_scylladb_type_map_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  php_scylladb_type* type1 = PHP_SCYLLADB_GET_TYPE(obj1);
  php_scylladb_type* type2 = PHP_SCYLLADB_GET_TYPE(obj2);

  return php_scylladb_type_compare(type1, type2 );
}

void
php_scylladb_type_map_free(zend_object *object )
{
  php_scylladb_type *self = php_scylladb_type_object_fetch(object);

  if (self->data_type) cass_data_type_free(self->data_type);
  zval_ptr_dtor(&self->data.map.key_type);
  zval_ptr_dtor(&self->data.map.value_type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_type_map_new(zend_class_entry *ce )
{
  php_scylladb_type *self =
      (php_scylladb_type *)ecalloc(1, sizeof(php_scylladb_type) + zend_object_properties_size(ce));

  self->type = CASS_VALUE_TYPE_MAP;
  self->data_type = cass_data_type_new(self->type);
  ZVAL_UNDEF(&self->data.map.key_type);
  ZVAL_UNDEF(&self->data.map.value_type);

  zend_object_std_init(&self->zendObject, ce);
  php_scylladb_type_map_handlers.offset = XtOffsetOf(php_scylladb_type, zendObject);
  php_scylladb_type_map_handlers.free_obj = php_scylladb_type_map_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_scylladb_type_map_handlers;
  return &self->zendObject;
}


