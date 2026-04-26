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
#include <zend_smart_str.h>
#include "src/Map.h"
#include "Map_arginfo.h"

zend_class_entry *php_driver_type_map_ce = NULL;

ZEND_METHOD(Cassandra_Type_Map, __construct)
{
  zend_throw_exception_ex(php_driver_logic_exception_ce, 0 ,
    "Instantiation of a " PHP_DRIVER_NAMESPACE "\\Type\\Map type is not supported."
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
  php_driver_type *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());
  RETURN_ZVAL(&self->data.map.key_type, 1, 0);
}

ZEND_METHOD(Cassandra_Type_Map, valueType)
{
  php_driver_type *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());
  RETURN_ZVAL(&self->data.map.value_type, 1, 0);
}

ZEND_METHOD(Cassandra_Type_Map, __toString)
{
  php_driver_type *self;
  smart_str string = {NULL,0};

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());

  php_driver_type_string(self, &string );
  smart_str_0(&string);

  RETVAL_STRING(ZSTR_VAL(string.s));
  smart_str_free(&string);
}

ZEND_METHOD(Cassandra_Type_Map, create)
{
  php_driver_map *map;
  zval* args = NULL;
  int argc = 0, i;

  if (zend_parse_parameters(ZEND_NUM_ARGS() , "*",
                            &args, &argc) == FAILURE) {
    return;
  }

  if (argc % 2 == 1) {

    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                            "Not enough values, maps can only be created " \
                            "from an even number of values, where each odd " \
                            "value is a key and each even value is a value, " \
                            "e.g create(key, value, key, value, key, value)");
    return;
  }

  object_init_ex(return_value, php_driver_map_ce);
  map = PHP_DRIVER_GET_MAP(return_value);

  ZVAL_COPY(&map->type, getThis());

  if (argc > 0) {
    for (i = 0; i < argc; i += 2) {
      if (!php_driver_map_set(map,
                              &args[i],
                              &args[i + 1] )) {

        return;
      }
    }

  }
}


static zend_object_handlers php_driver_type_map_handlers;

static HashTable *
php_driver_type_map_gc(
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

static HashTable *
php_driver_type_map_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
#if PHP_MAJOR_VERSION >= 8
  php_driver_type *self  = PHP5TO7_ZEND_OBJECT_GET(type, object);
#else
  php_driver_type *self  = PHP_DRIVER_GET_TYPE(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, "keyType", sizeof("keyType") - 1, &self->data.map.key_type);
  Z_ADDREF_P(&self->data.map.key_type);

  (void)zend_hash_str_update(props, "valueType", sizeof("valueType") - 1, &self->data.map.value_type);
  Z_ADDREF_P(&self->data.map.value_type);

  return props;
}

static int
php_driver_type_map_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  php_driver_type* type1 = PHP_DRIVER_GET_TYPE(obj1);
  php_driver_type* type2 = PHP_DRIVER_GET_TYPE(obj2);

  return php_driver_type_compare(type1, type2 );
}

static void
php_driver_type_map_free(zend_object *object )
{
  php_driver_type *self = PHP5TO7_ZEND_OBJECT_GET(type, object);

  if (self->data_type) cass_data_type_free(self->data_type);
  zval_ptr_dtor(&self->data.map.key_type);
  zval_ptr_dtor(&self->data.map.value_type);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_type_map_new(zend_class_entry *ce )
{
  php_driver_type *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(type, ce);

  self->type = CASS_VALUE_TYPE_MAP;
  self->data_type = cass_data_type_new(self->type);
  ZVAL_UNDEF(&self->data.map.key_type);
  ZVAL_UNDEF(&self->data.map.value_type);

  PHP5TO7_ZEND_OBJECT_INIT_EX(type, type_map, self, ce);
}

void php_driver_define_TypeMap()
{
  php_driver_type_map_ce = register_class_Cassandra_Type_Map(php_driver_type_ce);
  memcpy(&php_driver_type_map_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_type_map_handlers.get_properties  = php_driver_type_map_properties;
  php_driver_type_map_handlers.get_gc          = php_driver_type_map_gc;
  php_driver_type_map_handlers.compare = php_driver_type_map_compare;
  php_driver_type_map_ce->create_object = php_driver_type_map_new;
}
END_EXTERN_C()