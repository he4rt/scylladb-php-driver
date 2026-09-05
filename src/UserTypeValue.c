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
#include "Type/Conversions.h"
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"

#include "src/Type/UserType.h"
#include "src/UserTypeValue.h"

#include "UserTypeValue_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_user_type_value_handlers;

void
php_scylladb_user_type_value_set(php_scylladb_user_type_value *user_type_value,
                                  zend_string *name, zval *object)
{
  (void)zend_hash_update(&user_type_value->values, name, object);
  Z_TRY_ADDREF_P(object);
  user_type_value->dirty = 1;
}

static void
php_scylladb_user_type_value_populate(php_scylladb_user_type_value *user_type_value, zval *array)
{
  zend_string *name;
  php_scylladb_type *type;
  zval null;


  ZVAL_NULL(&null);

  type = PHP_SCYLLADB_GET_TYPE(&user_type_value->type);

  ZEND_HASH_FOREACH_STR_KEY(&type->data.udt.types, name) {
    zval *value = nullptr;
    if (name == nullptr) {
      continue;
    }
    size_t name_len = ZSTR_LEN(name);
    if ((value = zend_hash_find(&user_type_value->values, name)) != nullptr) {
      add_assoc_zval_ex(array, ZSTR_VAL(name), name_len, value);
      Z_TRY_ADDREF_P(value);
    } else {
      add_assoc_zval_ex(array, ZSTR_VAL(name), name_len, &null);
      Z_TRY_ADDREF_P(&null);
    }
  } ZEND_HASH_FOREACH_END();
}

/* {{{ UserTypeValue::__construct(types) */
ZEND_METHOD(Cassandra_UserTypeValue, __construct)
{
  php_scylladb_user_type_value *self;
  php_scylladb_type *type;
  HashTable *types = nullptr;
  zend_string *name;
  int index = 0;
  zval *current;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY_HT(types)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  self->type = php_scylladb_type_user_type();
  type = PHP_SCYLLADB_GET_TYPE(&self->type);

  ZEND_HASH_FOREACH_STR_KEY_VAL(types, name, current) {
    zval *sub_type = current;
    zval scalar_type;

    if (!name) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                              "Argument %d is not a string", index + 1);
      RETURN_THROWS();
    }
    index++;

    if (Z_TYPE_P(sub_type) == IS_STRING) {
      CassValueType value_type;
      if (!php_scylladb_value_type(Z_STR_P(sub_type), &value_type )) {
        return;
      }
      scalar_type = php_scylladb_type_scalar(value_type );
      if (!php_scylladb_type_user_type_add(type, name, &scalar_type )) {
        return;
      }
    } else if (Z_TYPE_P(sub_type) == IS_OBJECT &&
               instanceof_function(Z_OBJCE_P(sub_type), php_scylladb_type_ce )) {
      if (!php_scylladb_type_validate(sub_type, "sub_type" )) {
        return;
      }
      if (php_scylladb_type_user_type_add(type, name, sub_type )) {
        Z_ADDREF_P(sub_type);
      } else {
        return;
      }
    } else {
      INVALID_ARGUMENT(sub_type, "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Type");
    }
  } ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ UserTypeValue::type() */
ZEND_METHOD(Cassandra_UserTypeValue, type)
{
  auto self = PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  RETURN_ZVAL(&self->type, 1, 0);
}

/* {{{ UserTypeValue::values() */
ZEND_METHOD(Cassandra_UserTypeValue, values)
{
  php_scylladb_user_type_value *self = nullptr;
  self = PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);

  array_init(return_value);
  php_scylladb_user_type_value_populate(self, return_value );
}
/* }}} */

/* {{{ UserTypeValue::set(name, mixed) */
ZEND_METHOD(Cassandra_UserTypeValue, set)
{
  php_scylladb_user_type_value *self = nullptr;
  php_scylladb_type *type;
  zval *sub_type;
  zend_string *name = nullptr;
  zval *value = nullptr;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STR(name)
    Z_PARAM_ZVAL(value)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  type = PHP_SCYLLADB_GET_TYPE(&self->type);

  if ((sub_type = zend_hash_find(&type->data.udt.types, name)) == nullptr) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                            "Invalid name '%.*s'", (int)ZSTR_LEN(name), ZSTR_VAL(name));
    RETURN_THROWS();
  }

  if (!php_scylladb_validate_object(value,
                                     sub_type )) {
    return;
  }

  php_scylladb_user_type_value_set(self, name, value );
}
/* }}} */

/* {{{ UserTypeValue::get(name) */
ZEND_METHOD(Cassandra_UserTypeValue, get)
{
  php_scylladb_user_type_value *self = nullptr;
  php_scylladb_type *type;
  zval *sub_type;
  zend_string *name = nullptr;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  type = PHP_SCYLLADB_GET_TYPE(&self->type);

  if ((sub_type = zend_hash_find(&type->data.udt.types, name)) == nullptr) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                            "Invalid name '%.*s'", (int)ZSTR_LEN(name), ZSTR_VAL(name));
    RETURN_THROWS();
  }

  if ((value = zend_hash_find(&self->values, name)) != nullptr) {
    RETURN_ZVAL(value, 1, 0);
  }
}
/* }}} */

/* {{{ UserTypeValue::count() */
ZEND_METHOD(Cassandra_UserTypeValue, count)
{
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  php_scylladb_type *type =
      PHP_SCYLLADB_GET_TYPE(&self->type);
  RETURN_LONG(zend_hash_num_elements(&type->data.udt.types));
}
/* }}} */

/* {{{ UserTypeValue::current() */
ZEND_METHOD(Cassandra_UserTypeValue, current)
{
  zend_string* key;
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  php_scylladb_type *type =
      PHP_SCYLLADB_GET_TYPE(&self->type);
  if (zend_hash_get_current_key_ex(&type->data.udt.types, &key, nullptr, &self->pos) == HASH_KEY_IS_STRING) {
    zval *value;
    if ((value = zend_hash_str_find(&self->values, key->val, key->len)) != nullptr) {
      RETURN_ZVAL(value, 1, 0);
    }
  }
}
/* }}} */

/* {{{ UserTypeValue::key() */
ZEND_METHOD(Cassandra_UserTypeValue, key)
{
  zend_string* key;
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  php_scylladb_type *type =
      PHP_SCYLLADB_GET_TYPE(&self->type);
  if (zend_hash_get_current_key_ex(&type->data.udt.types, &key, nullptr, &self->pos) == HASH_KEY_IS_STRING) {
    RETURN_STR(key);
  }
}
/* }}} */

/* {{{ UserTypeValue::next() */
ZEND_METHOD(Cassandra_UserTypeValue, next)
{
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  php_scylladb_type *type =
      PHP_SCYLLADB_GET_TYPE(&self->type);
  zend_hash_move_forward_ex(&type->data.udt.types, &self->pos);
}
/* }}} */

/* {{{ UserTypeValue::valid() */
ZEND_METHOD(Cassandra_UserTypeValue, valid)
{
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  php_scylladb_type *type =
      PHP_SCYLLADB_GET_TYPE(&self->type);
  RETURN_BOOL(zend_hash_has_more_elements_ex(&type->data.udt.types, &self->pos) == SUCCESS);
}
/* }}} */

/* {{{ UserTypeValue::rewind() */
ZEND_METHOD(Cassandra_UserTypeValue, rewind)
{
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_GET_USER_TYPE_VALUE(ZEND_THIS);
  php_scylladb_type *type =
      PHP_SCYLLADB_GET_TYPE(&self->type);
  zend_hash_internal_pointer_reset_ex(&type->data.udt.types, &self->pos);
}
/* }}} */



HashTable *
php_scylladb_user_type_value_gc(zend_object *object, zval** table, int *n)
{
  auto self = php_scylladb_user_type_value_object_fetch(object);
  zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
  zend_get_gc_buffer_add_zval(buffer, &self->type);
  zval *current;
  ZEND_HASH_FOREACH_VAL(&self->values, current) {
    zend_get_gc_buffer_add_zval(buffer, current);
  } ZEND_HASH_FOREACH_END();
  zend_get_gc_buffer_use(buffer, table, n);
  return nullptr;
}

HashTable *
php_scylladb_user_type_value_properties(zend_object *object)
{
  zval values;

  auto self = php_scylladb_user_type_value_object_fetch(object);
  HashTable *props = php_scylladb_properties_rebuild(object, 2);

  (void)zend_hash_str_update(props, ZEND_STRL("type"), &self->type);
  Z_ADDREF_P(&self->type);


  array_init(&values);
  php_scylladb_user_type_value_populate(self, &values );
  (void)zend_hash_str_update(props, ZEND_STRL("values"), &values);

  return props;
}

int
php_scylladb_user_type_value_compare(zval *obj1, zval *obj2)
{
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  HashPosition pos1;
  HashPosition pos2;
  zval *current1;
  zval *current2;
  php_scylladb_user_type_value *user_type_value1;
  php_scylladb_user_type_value *user_type_value2;
  php_scylladb_type *type1;
  php_scylladb_type *type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  user_type_value1 = PHP_SCYLLADB_GET_USER_TYPE_VALUE(obj1);
  user_type_value2 = PHP_SCYLLADB_GET_USER_TYPE_VALUE(obj2);

  type1 = PHP_SCYLLADB_GET_TYPE(&user_type_value1->type);
  type2 = PHP_SCYLLADB_GET_TYPE(&user_type_value2->type);

  result = php_scylladb_type_compare(type1, type2 );
  if (result != 0) return result;

  if (zend_hash_num_elements(&user_type_value1->values) != zend_hash_num_elements(&user_type_value2->values)) {
    return zend_hash_num_elements(&user_type_value1->values) < zend_hash_num_elements(&user_type_value2->values) ? -1 : 1;
  }

  zend_hash_internal_pointer_reset_ex(&user_type_value1->values, &pos1);
  zend_hash_internal_pointer_reset_ex(&user_type_value2->values, &pos2);

  while ((current1 = zend_hash_get_current_data_ex(&user_type_value1->values, &pos1)) != nullptr &&
         (current2 = zend_hash_get_current_data_ex(&user_type_value2->values, &pos2)) != nullptr) {
    result = php_scylladb_value_compare(current1,
                                         current2 );
    if (result != 0) return result;
    zend_hash_move_forward_ex(&user_type_value1->values, &pos1);
    zend_hash_move_forward_ex(&user_type_value2->values, &pos2);
  }

  return 0;
}

unsigned
php_scylladb_user_type_value_hash_value(zval *obj)
{
  zval *current;
  unsigned hashv = 0;
  auto self = PHP_SCYLLADB_GET_USER_TYPE_VALUE(obj);

  if (!self->dirty) return self->hashv;

  ZEND_HASH_FOREACH_VAL(&self->values, current) {
    hashv = php_scylladb_combine_hash(hashv,
                                       php_scylladb_value_hash(current ));
  } ZEND_HASH_FOREACH_END();

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

void
php_scylladb_user_type_value_free(zend_object *object)
{
  php_scylladb_user_type_value *self =
      php_scylladb_user_type_value_object_fetch(object);

  zend_hash_destroy(&self->values);
  zval_ptr_dtor(&self->type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_user_type_value_new(zend_class_entry *ce)
{
  php_scylladb_user_type_value *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_user_type_value, ce, &php_scylladb_user_type_value_handlers);

  zend_hash_init(&self->values, 0, nullptr, ZVAL_PTR_DTOR, 0);
  self->pos = HT_INVALID_IDX;
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  php_scylladb_user_type_value_handlers.std.offset = offsetof(php_scylladb_user_type_value, zendObject);
  php_scylladb_user_type_value_handlers.std.free_obj = php_scylladb_user_type_value_free;
  return &self->zendObject;
}


void php_scylladb_user_type_value_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_user_type_value_handlers.std.offset = offsetof(php_scylladb_user_type_value, zendObject);
}
