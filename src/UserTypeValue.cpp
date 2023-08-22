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

#include "src/UserTypeValue.h"

#include "php_driver.h"
#include "php_driver_types.h"
#include "src/Type/UserType.h"
#include "util/collections.h"
#include "util/hash.h"
#include "util/types.h"
BEGIN_EXTERN_C()
zend_class_entry *php_driver_user_type_value_ce = NULL;

void php_driver_user_type_value_set(php_driver_user_type_value *user_type_value, const char *name,
                                    size_t name_length, zval *object) {
  PHP5TO7_ZEND_HASH_UPDATE(&user_type_value->values, name, name_length + 1, object, sizeof(zval *));
  Z_TRY_ADDREF_P(object);
  user_type_value->dirty = 1;
}

static void php_driver_user_type_value_populate(php_driver_user_type_value *user_type_value,
                                                zval *array) {
  char *name;
  php_driver_type *type;
  zval *current;
  zval null;

  ZVAL_NULL(&null);

  type = PHP_DRIVER_GET_TYPE(&user_type_value->type);

  PHP5TO7_ZEND_HASH_FOREACH_STR_KEY_VAL(&type->data.udt.types, name, current) {
    zval *value = NULL;
    size_t name_len = strlen(name);
    (void)current;
    if (PHP5TO7_ZEND_HASH_FIND(&user_type_value->values, name, name_len + 1, value)) {
      PHP5TO7_ADD_ASSOC_ZVAL_EX(array, name, name_len + 1, value);
      Z_TRY_ADDREF_P(value);
    } else {
      PHP5TO7_ADD_ASSOC_ZVAL_EX(array, name, name_len + 1, &null);
      Z_TRY_ADDREF_P(&null);
    }
  }
  PHP5TO7_ZEND_HASH_FOREACH_END(&type->data.udt.types);
}

/* {{{ UserTypeValue::__construct(types) */
PHP_METHOD(UserTypeValue, __construct) {
  php_driver_user_type_value *self;
  php_driver_type *type;
  HashTable *types;
  char *name;
  int index = 0;
  zval *current;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "h", &types) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  self->type = php_driver_type_user_type();
  type = PHP_DRIVER_GET_TYPE(&self->type);

  PHP5TO7_ZEND_HASH_FOREACH_STR_KEY_VAL(types, name, current) {
    zval *sub_type = current;
    zval scalar_type;

    if (!name) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Argument %d is not a string", index + 1);
      return;
    }
    index++;

    if (Z_TYPE_P(sub_type) == IS_STRING) {
      CassValueType value_type;
      if (!php_driver_value_type(Z_STRVAL_P(sub_type), &value_type)) {
        return;
      }
      scalar_type = php_driver_type_scalar(value_type);
      if (!php_driver_type_user_type_add(type, name, strlen(name), &scalar_type)) {
        return;
      }
    } else if (Z_TYPE_P(sub_type) == IS_OBJECT &&
               instanceof_function(Z_OBJCE_P(sub_type), php_driver_type_ce)) {
      if (!php_driver_type_validate(sub_type, "sub_type")) {
        return;
      }
      if (php_driver_type_user_type_add(type, name, strlen(name), sub_type)) {
        Z_ADDREF_P(sub_type);
      } else {
        return;
      }
    } else {
      INVALID_ARGUMENT(sub_type, "a string or an instance of " PHP_DRIVER_NAMESPACE "\\Type");
    }
  }
  PHP5TO7_ZEND_HASH_FOREACH_END(types);
}
/* }}} */

/* {{{ UserTypeValue::type() */
PHP_METHOD(UserTypeValue, type) {
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}

/* {{{ UserTypeValue::values() */
PHP_METHOD(UserTypeValue, values) {
  php_driver_user_type_value *self = NULL;
  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());

  array_init(return_value);
  php_driver_user_type_value_populate(self, return_value);
}
/* }}} */

/* {{{ UserTypeValue::set(name, mixed) */
PHP_METHOD(UserTypeValue, set) {
  php_driver_user_type_value *self = NULL;
  php_driver_type *type;
  zval *sub_type;
  char *name;
  size_t name_length;
  zval *value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &name, &name_length, &value) == FAILURE) return;

  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  if (!PHP5TO7_ZEND_HASH_FIND(&type->data.udt.types, name, name_length + 1, sub_type)) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "Invalid name '%s'", name);
    return;
  }

  if (!php_driver_validate_object(value, sub_type)) {
    return;
  }

  php_driver_user_type_value_set(self, name, name_length, value);
}
/* }}} */

/* {{{ UserTypeValue::get(name) */
PHP_METHOD(UserTypeValue, get) {
  php_driver_user_type_value *self = NULL;
  php_driver_type *type;
  zval *sub_type;
  char *name;
  size_t name_length;
  zval *value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &name, &name_length) == FAILURE) return;

  self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  if (!PHP5TO7_ZEND_HASH_FIND(&type->data.udt.types, name, name_length + 1, sub_type)) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "Invalid name '%s'", name);
    return;
  }

  if (PHP5TO7_ZEND_HASH_FIND(&self->values, name, name_length + 1, value)) {
    RETURN_ZVAL(value, 1, 0);
  }
}
/* }}} */

/* {{{ UserTypeValue::count() */
PHP_METHOD(UserTypeValue, count) {
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  RETURN_LONG(zend_hash_num_elements(&type->data.udt.types));
}
/* }}} */

/* {{{ UserTypeValue::current() */
PHP_METHOD(UserTypeValue, current) {
  zend_string *key;
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  if (PHP5TO7_ZEND_HASH_GET_CURRENT_KEY_EX(&type->data.udt.types, &key, NULL, &self->pos) ==
      HASH_KEY_IS_STRING) {
    zval *value;
#if PHP_MAJOR_VERSION >= 7
    if (PHP5TO7_ZEND_HASH_FIND(&self->values, key->val, key->len + 1, value)) {
#else
    if (PHP5TO7_ZEND_HASH_FIND(&self->values, key, strlen(key) + 1, value)) {
#endif
      RETURN_ZVAL(value, 1, 0);
    }
  }
}
/* }}} */

/* {{{ UserTypeValue::key() */
PHP_METHOD(UserTypeValue, key) {
  zend_string *key;
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  if (PHP5TO7_ZEND_HASH_GET_CURRENT_KEY_EX(&type->data.udt.types, &key, NULL, &self->pos) ==
      HASH_KEY_IS_STRING) {
#if PHP_MAJOR_VERSION >= 7
    RETURN_STR(key);
#else
    RETURN_STRING(key, 1);
#endif
  }
}
/* }}} */

/* {{{ UserTypeValue::next() */
PHP_METHOD(UserTypeValue, next) {
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  zend_hash_move_forward_ex(&type->data.udt.types, &self->pos);
}
/* }}} */

/* {{{ UserTypeValue::valid() */
PHP_METHOD(UserTypeValue, valid) {
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  RETURN_BOOL(zend_hash_has_more_elements_ex(&type->data.udt.types, &self->pos) == SUCCESS);
}
/* }}} */

/* {{{ UserTypeValue::rewind() */
PHP_METHOD(UserTypeValue, rewind) {
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  zend_hash_internal_pointer_reset_ex(&type->data.udt.types, &self->pos);
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, types)
ZEND_END_ARG_INFO()

#if PHP_MAJOR_VERSION >= 8
ZEND_BEGIN_ARG_INFO_EX(arginfo_name_value, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_value, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_name, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_key, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_next, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_rewind, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_user_type_value_methods[] = {
  PHP_ME(UserTypeValue, __construct, arginfo__construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
      PHP_ME(UserTypeValue, type, arginfo_none, ZEND_ACC_PUBLIC)
          PHP_ME(UserTypeValue, values, arginfo_none, ZEND_ACC_PUBLIC)
#if PHP_MAJOR_VERSION >= 8
              PHP_ME(UserTypeValue, set, arginfo_name_value, ZEND_ACC_PUBLIC)
#else
              PHP_ME(UserTypeValue, set, arginfo_value, ZEND_ACC_PUBLIC)
#endif
                  PHP_ME(UserTypeValue, get, arginfo_name, ZEND_ACC_PUBLIC)
  /* Countable */
  PHP_ME(UserTypeValue, count, arginfo_count, ZEND_ACC_PUBLIC)
  /* Iterator */
  PHP_ME(UserTypeValue, current, arginfo_current, ZEND_ACC_PUBLIC)
      PHP_ME(UserTypeValue, key, arginfo_key, ZEND_ACC_PUBLIC)
          PHP_ME(UserTypeValue, next, arginfo_next, ZEND_ACC_PUBLIC)
              PHP_ME(UserTypeValue, valid, arginfo_valid, ZEND_ACC_PUBLIC)
                  PHP_ME(UserTypeValue, rewind, arginfo_rewind, ZEND_ACC_PUBLIC) PHP_FE_END
};

static zend_object_handlers php_driver_user_type_value_handlers;

static HashTable *php_driver_user_type_value_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object,
#else
    zendObject *object,
#endif
    zval **table, int *n) {
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *php_driver_user_type_value_properties(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object
#else
    zendObject *object
#endif
) {
  zval values;

#if PHP_MAJOR_VERSION >= 8
  php_driver_user_type_value *self = PHP5TO7_ZEND_OBJECT_GET(user_type_value, object);
#else
  php_driver_user_type_value *self = PHP_DRIVER_GET_USER_TYPE_VALUE(object);
#endif
  HashTable *props = zend_std_get_properties(object);

  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &self->type, sizeof(zval));
  Z_ADDREF_P(&self->type);

  array_init(&values);
  php_driver_user_type_value_populate(self, &values);
  PHP5TO7_ZEND_HASH_UPDATE(props, "values", sizeof("values"), &values, sizeof(zval));

  return props;
}

static int php_driver_user_type_value_compare(zval *obj1, zval *obj2) {
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

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  user_type_value1 = PHP_DRIVER_GET_USER_TYPE_VALUE(obj1);
  user_type_value2 = PHP_DRIVER_GET_USER_TYPE_VALUE(obj2);

  type1 = PHP_DRIVER_GET_TYPE(&user_type_value1->type);
  type2 = PHP_DRIVER_GET_TYPE(&user_type_value2->type);

  result = php_driver_type_compare(type1, type2);
  if (result != 0) return result;

  if (zend_hash_num_elements(&user_type_value1->values) !=
      zend_hash_num_elements(&user_type_value2->values)) {
    return zend_hash_num_elements(&user_type_value1->values) <
                   zend_hash_num_elements(&user_type_value2->values)
               ? -1
               : 1;
  }

  zend_hash_internal_pointer_reset_ex(&user_type_value1->values, &pos1);
  zend_hash_internal_pointer_reset_ex(&user_type_value2->values, &pos2);

  while (PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&user_type_value1->values, current1, &pos1) &&
         PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&user_type_value2->values, current2, &pos2)) {
    result = php_driver_value_compare(current1, current2);
    if (result != 0) return result;
    zend_hash_move_forward_ex(&user_type_value1->values, &pos1);
    zend_hash_move_forward_ex(&user_type_value2->values, &pos2);
  }

  return 0;
}

static void php_driver_user_type_value_free(zend_object *object) {
  php_driver_user_type_value *self = PHP5TO7_ZEND_OBJECT_GET(user_type_value, object);

  zend_hash_destroy(&self->values);
  PHP5TO7_ZVAL_MAYBE_DESTROY(self->type);

  zend_object_std_dtor(&self->zendObject);
}

static zend_object *php_driver_user_type_value_new(zend_class_entry *ce) {
  auto *self = PHP5TO7_ZEND_OBJECT_ECALLOC(user_type_value, ce);

  zend_hash_init(&self->values, 0, NULL, ZVAL_PTR_DTOR, 0);
  self->pos = HT_INVALID_IDX;
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  PHP5TO7_ZEND_OBJECT_INIT(user_type_value, self, ce);
}

void php_driver_define_UserTypeValue() {
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\UserTypeValue", php_driver_user_type_value_methods);
  php_driver_user_type_value_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_user_type_value_ce, 3, php_driver_value_ce, zend_ce_countable,
                        zend_ce_iterator);
  php_driver_user_type_value_ce->ce_flags |= ZEND_ACC_FINAL;
  php_driver_user_type_value_ce->create_object = php_driver_user_type_value_new;
  
  memcpy(&php_driver_user_type_value_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_driver_user_type_value_handlers.get_properties = php_driver_user_type_value_properties;
  php_driver_user_type_value_handlers.get_gc = php_driver_user_type_value_gc;
  php_driver_user_type_value_handlers.compare = php_driver_user_type_value_compare;

  php_driver_user_type_value_handlers.clone_obj = NULL;
}
END_EXTERN_C()