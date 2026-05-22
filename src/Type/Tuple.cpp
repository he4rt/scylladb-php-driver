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

#include "src/Tuple.h"

#include <php_driver.h>
#include <php_driver_types.h>
#include "Util/collections.h"
#include "Util/types.h"
#include <zend_smart_str.h>
BEGIN_EXTERN_C()
#include "Tuple_arginfo.h"
zend_class_entry* php_driver_type_tuple_ce = NULL;

int
php_driver_type_tuple_add(php_driver_type* type,
                          zval* zsub_type )
{
  php_driver_type* sub_type = PHP_DRIVER_GET_TYPE(zsub_type);
  if (cass_data_type_add_sub_type(type->data_type,
                                  sub_type->data_type)
      != CASS_OK) {
    return 0;
  }
  (void)zend_hash_next_index_insert(&type->data.tuple.types, zsub_type);
  return 1;
}

ZEND_METHOD(Cassandra_Type_Tuple, __construct)
{
  zend_throw_exception_ex(php_driver_logic_exception_ce, 0 ,
                          "Instantiation of a " PHP_DRIVER_NAMESPACE "\\Type\\Tuple type is not supported.");
  return;
}

ZEND_METHOD(Cassandra_Type_Tuple, name)
{
  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  RETVAL_STRING("tuple");
}

ZEND_METHOD(Cassandra_Type_Tuple, types)
{
  php_driver_type* self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());

  array_init(return_value);
  zend_hash_copy(Z_ARRVAL_P(return_value), &self->data.tuple.types, (copy_ctor_func_t)zval_add_ref);
}

ZEND_METHOD(Cassandra_Type_Tuple, __toString)
{
  php_driver_type* self;
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

ZEND_METHOD(Cassandra_Type_Tuple, create)
{
  php_driver_type* self;
  php_driver_tuple* tuple;
  zval* args = NULL;
  int argc               = 0, i, num_types;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_TYPE(getThis());

  object_init_ex(return_value, php_driver_tuple_ce);
  tuple = PHP_DRIVER_GET_TUPLE(return_value);

  ZVAL_COPY(&tuple->type, getThis());

  num_types = zend_hash_num_elements(&self->data.tuple.types);

  if (argc > 0) {
    if (argc != num_types) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce,
                              0 ,
                              "Invalid number of elements given. Expected %d arguments.",
                              zend_hash_num_elements(&self->data.tuple.types));

      return;
    }

    for (i = 0; i < argc; i++) {
      zval* sub_type;

      if ((sub_type = zend_hash_index_find(&self->data.tuple.types, (zend_ulong)(i))) == NULL || !php_driver_validate_object(&args[i], sub_type )) {

        return;
      }

      php_driver_tuple_set(tuple, i, &args[i] );
    }


  }
}


static zend_object_handlers php_driver_type_tuple_handlers;

static HashTable*
php_driver_type_tuple_gc(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object,
#else
  zendObject* object,
#endif
  zval** table,
  int* n )
{
  *table = NULL;
  *n     = 0;
  return NULL;
}

static HashTable*
php_driver_type_tuple_properties(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object
#else
  zendObject* object
#endif
)
{
  zval types;
#if PHP_MAJOR_VERSION >= 8
  php_driver_type* self = php_driver_type_object_fetch(object);
#else
  php_driver_type* self                          = PHP_DRIVER_GET_TYPE(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(1);
  HashTable *props = object->properties;

  array_init(&types);
  zend_hash_copy(Z_ARRVAL(types), &self->data.tuple.types, (copy_ctor_func_t)zval_add_ref);
  (void)zend_hash_str_update(props, ZEND_STRL("types"), &types);

  return props;
}

static int
php_driver_type_tuple_compare(zval* obj1, zval* obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  php_driver_type* type1 = PHP_DRIVER_GET_TYPE(obj1);
  php_driver_type* type2 = PHP_DRIVER_GET_TYPE(obj2);

  return php_driver_type_compare(type1, type2 );
}

static void
php_driver_type_tuple_free(zend_object* object )
{
  php_driver_type* self = php_driver_type_object_fetch(object);

  if (self->data_type)
    cass_data_type_free(self->data_type);
  zend_hash_destroy(&self->data.tuple.types);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_type_tuple_new(zend_class_entry* ce )
{
  php_driver_type* self = (php_driver_type *)ecalloc(1, sizeof(php_driver_type) + zend_object_properties_size(ce));

  self->type      = CASS_VALUE_TYPE_TUPLE;
  self->data_type = cass_data_type_new(self->type);
  zend_hash_init(&self->data.tuple.types, 0, NULL, ZVAL_PTR_DTOR, 0);

  zend_object_std_init(&self->zendObject, ce);
  php_driver_type_tuple_handlers.offset = XtOffsetOf(php_driver_type, zendObject);
  php_driver_type_tuple_handlers.free_obj = php_driver_type_tuple_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_type_tuple_handlers;
  return &self->zendObject;
}

void
php_driver_define_TypeTuple()
{
  php_driver_type_tuple_ce = register_class_Cassandra_Type_Tuple(php_driver_type_ce);
  memcpy(&php_driver_type_tuple_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_type_tuple_handlers.get_properties = php_driver_type_tuple_properties;
  php_driver_type_tuple_handlers.get_gc = php_driver_type_tuple_gc;
  php_driver_type_tuple_handlers.compare = php_driver_type_tuple_compare;
  php_driver_type_tuple_ce->create_object = php_driver_type_tuple_new;
}
END_EXTERN_C()