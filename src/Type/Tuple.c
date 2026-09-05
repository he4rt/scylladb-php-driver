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

#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include "Type/Conversions.h"
#include "Type/TypeFactory.h"
#include <zend_smart_str.h>

#include "Tuple_arginfo.h"

extern zend_object_handlers php_scylladb_type_tuple_handlers;
int
php_scylladb_type_tuple_add(php_scylladb_type* type,
                          zval* zsub_type )
{
  auto sub_type = PHP_SCYLLADB_GET_TYPE(zsub_type);
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
  zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0 ,
                          "Instantiation of a " PHP_SCYLLADB_NAMESPACE "\\Type\\Tuple type is not supported.");
  RETURN_THROWS();
}

ZEND_METHOD(Cassandra_Type_Tuple, name)
{
  ZEND_PARSE_PARAMETERS_NONE();

  RETVAL_STRING("tuple");
}

ZEND_METHOD(Cassandra_Type_Tuple, types)
{
  php_scylladb_type* self;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  array_init(return_value);
  zend_hash_copy(Z_ARRVAL_P(return_value), &self->data.tuple.types, (copy_ctor_func_t)zval_add_ref);
}

ZEND_METHOD(Cassandra_Type_Tuple, __toString)
{
  php_scylladb_type* self;
  smart_str string = {nullptr,0};

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  php_scylladb_type_string(self, &string );
  smart_str_0(&string);

  RETVAL_STRING(ZSTR_VAL(string.s));
  smart_str_free(&string);
}

ZEND_METHOD(Cassandra_Type_Tuple, create)
{
  php_scylladb_type* self;
  php_scylladb_tuple* tuple;
  zval* args = nullptr;
  uint32_t argc          = 0, i;
  uint32_t num_types;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  object_init_ex(return_value, php_scylladb_tuple_ce);
  tuple = PHP_SCYLLADB_GET_TUPLE(return_value);

  ZVAL_COPY(&tuple->type, ZEND_THIS);

  num_types = zend_hash_num_elements(&self->data.tuple.types);

  if (argc > 0) {
    if (argc != num_types) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce,
                              0 ,
                              "Invalid number of elements given. Expected %d arguments.",
                              zend_hash_num_elements(&self->data.tuple.types));

      return;
    }

    for (i = 0; i < argc; i++) {
      zval* sub_type;

      if ((sub_type = zend_hash_index_find(&self->data.tuple.types, (zend_ulong)(i))) == nullptr || !php_scylladb_validate_object(&args[i], sub_type )) {

        return;
      }

      php_scylladb_tuple_set(tuple, (zend_ulong)i, &args[i] );
    }

  }
}

HashTable*
php_scylladb_type_tuple_gc(
  zend_object* object,
  zval** table,
  int* n )
{
  /* Expose the sub-type objects held in the C struct so the cycle collector
   * accounts for them instead of treating this type as reference-free. */
  auto self = php_scylladb_type_object_fetch(object);
  zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();

  zval *current;
  ZEND_HASH_FOREACH_VAL(&self->data.tuple.types, current) {
    zend_get_gc_buffer_add_zval(buffer, current);
  } ZEND_HASH_FOREACH_END();

  zend_get_gc_buffer_use(buffer, table, n);
  return nullptr;
}

HashTable*
php_scylladb_type_tuple_properties(
  zend_object* object
)
{
  zval types;
  auto self = php_scylladb_type_object_fetch(object);
  HashTable *props = php_scylladb_properties_rebuild(object, 1);

  array_init(&types);
  zend_hash_copy(Z_ARRVAL(types), &self->data.tuple.types, (copy_ctor_func_t)zval_add_ref);
  (void)zend_hash_str_update(props, ZEND_STRL("types"), &types);

  return props;
}

int
php_scylladb_type_tuple_compare(zval* obj1, zval* obj2 )
{
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  auto type1 = PHP_SCYLLADB_GET_TYPE(obj1);
  auto type2 = PHP_SCYLLADB_GET_TYPE(obj2);

  return php_scylladb_type_compare(type1, type2 );
}

void
php_scylladb_type_tuple_free(zend_object* object )
{
  auto self = php_scylladb_type_object_fetch(object);

  if (self->data_type)
    cass_data_type_free(self->data_type);
  zend_hash_destroy(&self->data.tuple.types);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_type_tuple_new(zend_class_entry* ce )
{
  php_scylladb_type* self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_type, ce, &php_scylladb_type_tuple_handlers);

  self->type      = CASS_VALUE_TYPE_TUPLE;
  self->data_type = cass_data_type_new(self->type);
  zend_hash_init(&self->data.tuple.types, 0, nullptr, ZVAL_PTR_DTOR, 0);

  return &self->zendObject;
}


