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
#include "src/UserTypeValue.h"
#include "Type/Conversions.h"
#include "Type/TypeFactory.h"

#include <zend_smart_str.h>

#include "UserType_arginfo.h"

extern zend_object_handlers php_scylladb_type_user_type_handlers;
bool php_scylladb_type_user_type_add(php_scylladb_type *type, zend_string *name, zval *zsub_type)
{
  auto sub_type = PHP_SCYLLADB_GET_TYPE(zsub_type);
  if (cass_data_type_add_sub_type_by_name_n(type->data_type, ZSTR_VAL(name), ZSTR_LEN(name),
                                            sub_type->data_type) != CASS_OK) {
    return false;
  }
  (void)zend_hash_add(&type->data.udt.types, name, zsub_type);
  return true;
}

ZEND_METHOD(Cassandra_Type_UserType, __construct)
{
  zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0 ,
    "Instantiation of a " PHP_SCYLLADB_NAMESPACE "\\Type\\UserType type is not supported."
  );
  RETURN_THROWS();
}

ZEND_METHOD(Cassandra_Type_UserType, withName)
{
  zend_string *name = nullptr;
  php_scylladb_type *self;
  php_scylladb_type *user_type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  object_init_ex(return_value, php_scylladb_type_user_type_ce);
  user_type = PHP_SCYLLADB_GET_TYPE(return_value);
  user_type->data_type = cass_data_type_new_from_existing(self->data_type);

  user_type->data.udt.type_name = zend_string_copy(name);
  ASSERT_SUCCESS(cass_data_type_set_type_name_n(user_type->data_type, ZSTR_VAL(name),
                                                ZSTR_LEN(name)));

  if (self->data.udt.keyspace) {
    user_type->data.udt.keyspace = zend_string_copy(self->data.udt.keyspace);
  }

  zend_hash_copy(&user_type->data.udt.types, &self->data.udt.types, (copy_ctor_func_t)zval_add_ref);
}

ZEND_METHOD(Cassandra_Type_UserType, name)
{
  php_scylladb_type *self;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  if (!self->data.udt.type_name)
    RETURN_NULL();

  RETVAL_STR_COPY(self->data.udt.type_name);
}

ZEND_METHOD(Cassandra_Type_UserType, withKeyspace)
{
  zend_string *keyspace = nullptr;
  php_scylladb_type *self;
  php_scylladb_type *user_type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(keyspace)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  object_init_ex(return_value, php_scylladb_type_user_type_ce);
  user_type = PHP_SCYLLADB_GET_TYPE(return_value);
  user_type->data_type = cass_data_type_new_from_existing(self->data_type);

  if (self->data.udt.type_name) {
    user_type->data.udt.type_name = zend_string_copy(self->data.udt.type_name);
  }

  user_type->data.udt.keyspace = zend_string_copy(keyspace);
  ASSERT_SUCCESS(cass_data_type_set_keyspace_n(user_type->data_type, ZSTR_VAL(keyspace),
                                               ZSTR_LEN(keyspace)));

  zend_hash_copy(&user_type->data.udt.types, &self->data.udt.types, (copy_ctor_func_t)zval_add_ref);
}

ZEND_METHOD(Cassandra_Type_UserType, keyspace)
{
  php_scylladb_type *self;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  if (!self->data.udt.keyspace)
    RETURN_NULL();

  RETVAL_STR_COPY(self->data.udt.keyspace);
}

ZEND_METHOD(Cassandra_Type_UserType, types)
{
  php_scylladb_type *self;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  array_init(return_value);
  zend_hash_copy(Z_ARRVAL_P(return_value), &self->data.udt.types, (copy_ctor_func_t)zval_add_ref);
}

ZEND_METHOD(Cassandra_Type_UserType, __toString)
{
  php_scylladb_type *self;
  smart_str string = {nullptr,0};

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  php_scylladb_type_string(self, &string );
  smart_str_0(&string);

  RETVAL_STRING(ZSTR_VAL(string.s));
  smart_str_free(&string);
}

ZEND_METHOD(Cassandra_Type_UserType, create)
{
  php_scylladb_type *self;
  php_scylladb_user_type_value *user_type_value;
  zval* args = nullptr;
  uint32_t argc = 0, i;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TYPE(ZEND_THIS);

  object_init_ex(return_value, php_scylladb_user_type_value_ce);
  user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(return_value);

  ZVAL_COPY(&user_type_value->type, ZEND_THIS);

  if (argc > 0) {
    if (argc % 2 == 1) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                              "Not enough name/value pairs, user_types can only be created " \
                              "from an even number of name/value pairs, where each odd " \
                              "argument is a name and each even argument is a value, " \
                              "e.g user_type(name, value, name, value, name, value)");

      return;
    }

    for (i = 0; i < argc; i += 2) {
      zval *name = &args[i];
      zval *value = &args[i + 1];
      zval *sub_type;
      if (Z_TYPE_P(name) != IS_STRING) {
        zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                                "Argument %d is not a string", i + 1);

        return;
      }
      if ((sub_type = zend_hash_str_find(&self->data.udt.types, Z_STRVAL_P(name), Z_STRLEN_P(name))) == nullptr) {
        zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce,
                                0 ,
                                "Invalid name '%s'", Z_STRVAL_P(name));

        return;
      }
      if (!php_scylladb_validate_object(value,
                                      sub_type )) {

        return;
      }
      php_scylladb_user_type_value_set(user_type_value, Z_STR_P(name), value );
    }

  }
}

HashTable *
php_scylladb_type_user_type_gc(
        zend_object *object,
        zval** table, int *n
)
{
  auto self = php_scylladb_type_object_fetch(object);
  zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
  zval *current;
  ZEND_HASH_FOREACH_VAL(&self->data.udt.types, current) {
    zend_get_gc_buffer_add_zval(buffer, current);
  } ZEND_HASH_FOREACH_END();
  zend_get_gc_buffer_use(buffer, table, n);
  return nullptr;
}

HashTable *
php_scylladb_type_user_type_properties(
        zend_object *object
)
{
  zval keyspace;
  zval name;
  zval types;

  auto self = php_scylladb_type_object_fetch(object);
  HashTable *props = php_scylladb_properties_rebuild(object, 3);

  if (self->data.udt.keyspace) {
    ZVAL_STR_COPY(&keyspace, self->data.udt.keyspace);
  } else {
    ZVAL_NULL(&keyspace);
  }
  (void)zend_hash_str_update(props, ZEND_STRL("keyspace"), &keyspace);

  if (self->data.udt.type_name) {
    ZVAL_STR_COPY(&name, self->data.udt.type_name);
  } else {
    ZVAL_NULL(&name);
  }
  (void)zend_hash_str_update(props, ZEND_STRL("name"), &name);

  array_init(&types);
  zend_hash_copy(Z_ARRVAL(types), &self->data.udt.types, (copy_ctor_func_t)zval_add_ref);
  (void)zend_hash_str_update(props, ZEND_STRL("types"), &types);

  return props;
}

int
php_scylladb_type_user_type_compare(zval *obj1, zval *obj2 )
{
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  auto type1 = PHP_SCYLLADB_GET_TYPE(obj1);
  auto type2 = PHP_SCYLLADB_GET_TYPE(obj2);

  return php_scylladb_type_compare(type1, type2 );
}

void
php_scylladb_type_user_type_free(zend_object *object )
{
  auto self = php_scylladb_type_object_fetch(object);

  if (self->data_type) cass_data_type_free(self->data_type);
  if (self->data.udt.keyspace) zend_string_release(self->data.udt.keyspace);
  if (self->data.udt.type_name) zend_string_release(self->data.udt.type_name);
  zend_hash_destroy(&self->data.udt.types);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_type_user_type_new(zend_class_entry *ce )
{
  php_scylladb_type *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_type, ce, &php_scylladb_type_user_type_handlers);

  self->type = CASS_VALUE_TYPE_UDT;
  self->data_type = nullptr;
  self->data.udt.keyspace = self->data.udt.type_name = nullptr;
  zend_hash_init(&self->data.udt.types, 0, nullptr, ZVAL_PTR_DTOR, 0);

  return &self->zendObject;
}


