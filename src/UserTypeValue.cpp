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
#include "Type/Conversions.h"
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"

#include "src/Type/UserType.h"
#include "src/UserTypeValue.h"
BEGIN_EXTERN_C()
#include "UserTypeValue_arginfo.h"
zend_class_entry *php_driver_user_type_value_ce = NULL;

void
php_driver_user_type_value_set(php_driver_user_type_value *user_type_value,
                                  const char *name, size_t name_length,
                                  zval *object )
{
  (void)zend_hash_str_update(&user_type_value->values, name, name_length, object);
  Z_TRY_ADDREF_P(object);
  user_type_value->dirty = 1;
}

static void
php_driver_user_type_value_populate(php_driver_user_type_value *user_type_value, zval *array )
{
  zend_string *name;
  php_driver_type *type;
  zval *current;
  zval null;


  ZVAL_NULL(&null);

  type = PHP_DRIVER_GET_TYPE(&user_type_value->type);

  ZEND_HASH_FOREACH_STR_KEY_VAL(&type->data.udt.types, name, current) {
    zval *value = NULL;
    size_t name_len = ZSTR_LEN(name);
    (void) current;
    if ((value = zend_hash_str_find(&user_type_value->values, ZSTR_VAL(name), name_len)) != NULL) {
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
  php_driver_user_type_value *self;
  php_driver_type *type;
  HashTable *types;
  zend_string *name;
  int index = 0;
  zval *current;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY_HT(types)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  self->type = php_driver_type_user_type();
  type = PHP_DRIVER_GET_TYPE(&self->type);

  ZEND_HASH_FOREACH_STR_KEY_VAL(types, name, current) {
    zval *sub_type = current;
    zval scalar_type;

    if (!name) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                              "Argument %d is not a string", index + 1);
      return;
    }
    index++;

    if (Z_TYPE_P(sub_type) == IS_STRING) {
      CassValueType value_type;
      if (!php_driver_value_type(Z_STRVAL_P(sub_type), &value_type )) {
        return;
      }
      scalar_type = php_driver_type_scalar(value_type );
      if (!php_driver_type_user_type_add(type,
                                            ZSTR_VAL(name), ZSTR_LEN(name),
                                            &scalar_type )) {
        return;
      }
    } else if (Z_TYPE_P(sub_type) == IS_OBJECT &&
               instanceof_function(Z_OBJCE_P(sub_type), php_driver_type_ce )) {
      if (!php_driver_type_validate(sub_type, "sub_type" )) {
        return;
      }
      if (php_driver_type_user_type_add(type,
                                           ZSTR_VAL(name), ZSTR_LEN(name),
                                           sub_type )) {
        Z_ADDREF_P(sub_type);
      } else {
        return;
      }
    } else {
      INVALID_ARGUMENT(sub_type, "a string or an instance of " PHP_DRIVER_NAMESPACE "\\Type");
    }
  } ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ UserTypeValue::type() */
ZEND_METHOD(Cassandra_UserTypeValue, type)
{
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}

/* {{{ UserTypeValue::values() */
ZEND_METHOD(Cassandra_UserTypeValue, values)
{
  php_driver_user_type_value *self = NULL;
  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());

  array_init(return_value);
  php_driver_user_type_value_populate(self, return_value );
}
/* }}} */

/* {{{ UserTypeValue::set(name, mixed) */
ZEND_METHOD(Cassandra_UserTypeValue, set)
{
  php_driver_user_type_value *self = NULL;
  php_driver_type *type;
  zval *sub_type;
  char *name;
  size_t name_length;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(name, name_length)
    Z_PARAM_ZVAL(value)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  if ((sub_type = zend_hash_str_find(&type->data.udt.types, name, name_length)) == NULL) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                            "Invalid name '%s'", name);
    return;
  }

  if (!php_driver_validate_object(value,
                                     sub_type )) {
    return;
  }

  php_driver_user_type_value_set(self, name, name_length, value );
}
/* }}} */

/* {{{ UserTypeValue::get(name) */
ZEND_METHOD(Cassandra_UserTypeValue, get)
{
  php_driver_user_type_value *self = NULL;
  php_driver_type *type;
  zval *sub_type;
  char *name;
  size_t name_length;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_length)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  if ((sub_type = zend_hash_str_find(&type->data.udt.types, name, name_length)) == NULL) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                            "Invalid name '%s'", name);
    return;
  }

  if ((value = zend_hash_str_find(&self->values, name, name_length)) != NULL) {
    RETURN_ZVAL(value, 1, 0);
  }
}
/* }}} */

/* {{{ UserTypeValue::count() */
ZEND_METHOD(Cassandra_UserTypeValue, count)
{
  php_driver_user_type_value *self =
      PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type =
      PHP_DRIVER_GET_TYPE(&self->type);
  RETURN_LONG(zend_hash_num_elements(&type->data.udt.types));
}
/* }}} */

/* {{{ UserTypeValue::current() */
ZEND_METHOD(Cassandra_UserTypeValue, current)
{
  zend_string* key;
  php_driver_user_type_value *self =
      PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type =
      PHP_DRIVER_GET_TYPE(&self->type);
  if (zend_hash_get_current_key_ex(&type->data.udt.types, &key, NULL, &self->pos) == HASH_KEY_IS_STRING) {
    zval *value;
    if ((value = zend_hash_str_find(&self->values, key->val, key->len)) != NULL) {
      RETURN_ZVAL(value, 1, 0);
    }
  }
}
/* }}} */

/* {{{ UserTypeValue::key() */
ZEND_METHOD(Cassandra_UserTypeValue, key)
{
  zend_string* key;
  php_driver_user_type_value *self =
      PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type =
      PHP_DRIVER_GET_TYPE(&self->type);
  if (zend_hash_get_current_key_ex(&type->data.udt.types, &key, NULL, &self->pos) == HASH_KEY_IS_STRING) {
    RETURN_STR(key);
  }
}
/* }}} */

/* {{{ UserTypeValue::next() */
ZEND_METHOD(Cassandra_UserTypeValue, next)
{
  php_driver_user_type_value *self =
      PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type =
      PHP_DRIVER_GET_TYPE(&self->type);
  zend_hash_move_forward_ex(&type->data.udt.types, &self->pos);
}
/* }}} */

/* {{{ UserTypeValue::valid() */
ZEND_METHOD(Cassandra_UserTypeValue, valid)
{
  php_driver_user_type_value *self =
      PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type =
      PHP_DRIVER_GET_TYPE(&self->type);
  RETURN_BOOL(zend_hash_has_more_elements_ex(&type->data.udt.types, &self->pos) == SUCCESS);
}
/* }}} */

/* {{{ UserTypeValue::rewind() */
ZEND_METHOD(Cassandra_UserTypeValue, rewind)
{
  php_driver_user_type_value *self =
      PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type =
      PHP_DRIVER_GET_TYPE(&self->type);
  zend_hash_internal_pointer_reset_ex(&type->data.udt.types, &self->pos);
}
/* }}} */


static php_driver_value_handlers php_driver_user_type_value_handlers;

static HashTable *
php_driver_user_type_value_gc(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object,
#else
        zendObject *object,
#endif
        zval** table, int *n
)
{
  return zend_std_get_gc(object, table, n);
}

static HashTable *
php_driver_user_type_value_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
  zval values;

#if PHP_MAJOR_VERSION >= 8
  php_driver_user_type_value *self = php_driver_user_type_value_object_fetch(object);
#else
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, ZEND_STRL("type"), &self->type);
  Z_ADDREF_P(&self->type);


  array_init(&values);
  php_driver_user_type_value_populate(self, &values );
  (void)zend_hash_str_update(props, ZEND_STRL("values"), &values);

  return props;
}

static int
php_driver_user_type_value_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  HashPosition pos1;
  HashPosition pos2;
  zval *current1;
  zval *current2;
  php_driver_user_type_value *user_type_value1;
  php_driver_user_type_value *user_type_value2;
  php_driver_type *type1;
  php_driver_type *type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  user_type_value1 = PHP_DRIVER_GET_USER_TYPE_VALUE(obj1);
  user_type_value2 = PHP_DRIVER_GET_USER_TYPE_VALUE(obj2);

  type1 = PHP_DRIVER_GET_TYPE(&user_type_value1->type);
  type2 = PHP_DRIVER_GET_TYPE(&user_type_value2->type);

  result = php_driver_type_compare(type1, type2 );
  if (result != 0) return result;

  if (zend_hash_num_elements(&user_type_value1->values) != zend_hash_num_elements(&user_type_value2->values)) {
    return zend_hash_num_elements(&user_type_value1->values) < zend_hash_num_elements(&user_type_value2->values) ? -1 : 1;
  }

  zend_hash_internal_pointer_reset_ex(&user_type_value1->values, &pos1);
  zend_hash_internal_pointer_reset_ex(&user_type_value2->values, &pos2);

  while ((current1 = zend_hash_get_current_data_ex(&user_type_value1->values, &pos1)) != NULL &&
         (current2 = zend_hash_get_current_data_ex(&user_type_value2->values, &pos2)) != NULL) {
    result = php_driver_value_compare(current1,
                                         current2 );
    if (result != 0) return result;
    zend_hash_move_forward_ex(&user_type_value1->values, &pos1);
    zend_hash_move_forward_ex(&user_type_value2->values, &pos2);
  }

  return 0;
}

static unsigned
php_driver_user_type_value_hash_value(zval *obj )
{
  zval *current;
  unsigned hashv = 0;
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(obj);

  if (!self->dirty) return self->hashv;

  ZEND_HASH_FOREACH_VAL(&self->values, current) {
    hashv = php_driver_combine_hash(hashv,
                                       php_driver_value_hash(current ));
  } ZEND_HASH_FOREACH_END();

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

static void
php_driver_user_type_value_free(zend_object *object )
{
  php_driver_user_type_value *self =
      php_driver_user_type_value_object_fetch(object);

  zend_hash_destroy(&self->values);
  zval_ptr_dtor(&self->type);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_user_type_value_new(zend_class_entry *ce )
{
  php_driver_user_type_value *self =
      (php_driver_user_type_value *)ecalloc(1, sizeof(php_driver_user_type_value) + zend_object_properties_size(ce));

  zend_hash_init(&self->values, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_MAJOR_VERSION >= 7
  self->pos = HT_INVALID_IDX;
#else
  self->pos = NULL;
#endif
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  zend_object_std_init(&self->zendObject, ce);
  php_driver_user_type_value_handlers.std.offset = XtOffsetOf(php_driver_user_type_value, zendObject);
  php_driver_user_type_value_handlers.std.free_obj = php_driver_user_type_value_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_user_type_value_handlers;
  return &self->zendObject;
}

void php_driver_define_UserTypeValue()
{
  php_driver_user_type_value_ce = register_class_Cassandra_UserTypeValue(php_driver_value_ce, zend_ce_countable, zend_ce_iterator);
  memcpy(&php_driver_user_type_value_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_user_type_value_handlers.std.get_properties  = php_driver_user_type_value_properties;
  php_driver_user_type_value_handlers.std.get_gc          = php_driver_user_type_value_gc;
  php_driver_user_type_value_handlers.std.compare = php_driver_user_type_value_compare;
  php_driver_user_type_value_ce->create_object = php_driver_user_type_value_new;

  php_driver_user_type_value_handlers.hash_value = php_driver_user_type_value_hash_value;
  php_driver_user_type_value_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()