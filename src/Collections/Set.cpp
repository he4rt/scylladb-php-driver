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

#include "src/Collections/Set.h"

#include <php_driver.h>
#include <php_driver_types.h>
#include <util/collections.h>
#include <util/hash.h>
#include <util/types.h>
BEGIN_EXTERN_C()
zend_class_entry* php_driver_set_ce = NULL;

int php_driver_set_add(php_driver_set* set, zval* object) {
  php_driver_set_entry* entry;
  php_driver_type* type;

  if (Z_TYPE_P(object) == IS_NULL) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "Invalid value: null is not supported inside sets");
    return 0;
  }

  type = PHP_DRIVER_GET_TYPE(&set->type);

  if (!php_driver_validate_object(object, &type->data.set.value_type)) {
    return 0;
  }

  HASH_FIND_ZVAL(set->entries, object, entry);
  if (entry == NULL) {
    set->dirty = 1;
    entry = (php_driver_set_entry*)emalloc(sizeof(php_driver_set_entry));
    ZVAL_COPY(&entry->value, object);
    HASH_ADD_ZVAL(set->entries, value, entry);
  }

  return 1;
}

static int php_driver_set_del(php_driver_set* set, zval* object) {
  php_driver_set_entry* entry;
  php_driver_type* type;
  int result = 0;

  type = PHP_DRIVER_GET_TYPE(&set->type);

  if (!php_driver_validate_object(object, &type->data.set.value_type)) {
    return 0;
  }

  HASH_FIND_ZVAL(set->entries, object, entry);
  if (entry != NULL) {
    set->dirty = 1;
    if (entry == set->iter_temp) {
      set->iter_temp = (php_driver_set_entry*)set->iter_temp->hh.next;
    }
    HASH_DEL(set->entries, entry);
    zval_ptr_dtor(&entry->value);
    efree(entry);
    result = 1;
  }

  return result;
}

static int php_driver_set_has(php_driver_set* set, zval* object) {
  php_driver_set_entry* entry;
  php_driver_type* type;
  int result = 0;

  type = PHP_DRIVER_GET_TYPE(&set->type);

  if (!php_driver_validate_object(object, &type->data.set.value_type)) {
    return 0;
  }

  HASH_FIND_ZVAL(set->entries, object, entry);
  if (entry != NULL) {
    result = 1;
  }

  return result;
}

static void php_driver_set_populate(php_driver_set* set, zval* array) {
  php_driver_set_entry *curr, *temp;
  HASH_ITER(hh, set->entries, curr, temp) {
    if (add_next_index_zval(array, &curr->value) != SUCCESS) {
      break;
    }
    Z_TRY_ADDREF_P(&curr->value);
  }
}

/* {{{ Set::__construct(type) */
PHP_METHOD(Set, __construct) {
  php_driver_set* self;
  zval* type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &type) == FAILURE) return;

  self = PHP_DRIVER_GET_SET(getThis());

  if (Z_TYPE_P(type) == IS_STRING) {
    CassValueType value_type;
    if (!php_driver_value_type(Z_STRVAL_P(type), &value_type)) return;
    self->type = php_driver_type_set_from_value_type(value_type);
  } else if (Z_TYPE_P(type) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(type), php_driver_type_ce)) {
    if (!php_driver_type_validate(type, "type")) {
      return;
    }
    self->type = php_driver_type_set(type);
    Z_ADDREF_P(type);
  } else {
    INVALID_ARGUMENT(type, "a string or an instance of " PHP_DRIVER_NAMESPACE "\\Type");
  }
}
/* }}} */

/* {{{ Set::type() */
PHP_METHOD(Set, type) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}
/* }}} */

/* {{{ Set::values() */
PHP_METHOD(Set, values) {
  php_driver_set* set = NULL;
  array_init(return_value);
  set = PHP_DRIVER_GET_SET(getThis());
  php_driver_set_populate(set, return_value);
}
/* }}} */

/* {{{ Set::add(value) */
PHP_METHOD(Set, add) {
  php_driver_set* self = NULL;

  zval* object;
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &object) == FAILURE) return;

  self = PHP_DRIVER_GET_SET(getThis());

  if (php_driver_set_add(self, object)) RETURN_TRUE;

  RETURN_FALSE;
}
/* }}} */

/* {{{ Set::remove(value) */
PHP_METHOD(Set, remove) {
  php_driver_set* self = NULL;

  zval* object;
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &object) == FAILURE) return;

  self = PHP_DRIVER_GET_SET(getThis());

  if (php_driver_set_del(self, object)) RETURN_TRUE;

  RETURN_FALSE;
}
/* }}} */

/* {{{ Set::has(value) */
PHP_METHOD(Set, has) {
  php_driver_set* self = NULL;

  zval* object;
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &object) == FAILURE) return;

  self = PHP_DRIVER_GET_SET(getThis());

  if (php_driver_set_has(self, object)) RETURN_TRUE;

  RETURN_FALSE;
}
/* }}} */

/* {{{ Set::count() */
PHP_METHOD(Set, count) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  RETURN_LONG((long)HASH_COUNT(self->entries));
}
/* }}} */

/* {{{ Set::current() */
PHP_METHOD(Set, current) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  if (self->iter_curr != NULL) RETURN_ZVAL(&self->iter_curr->value, 1, 0);
}
/* }}} */

/* {{{ Set::key() */
PHP_METHOD(Set, key) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  RETURN_LONG(self->iter_index);
}
/* }}} */

/* {{{ Set::next() */
PHP_METHOD(Set, next) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  self->iter_curr = self->iter_temp;
  self->iter_temp =
      self->iter_temp != NULL ? (php_driver_set_entry*)self->iter_temp->hh.next : NULL;
  self->iter_index++;
}
/* }}} */

/* {{{ Set::valid() */
PHP_METHOD(Set, valid) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  RETURN_BOOL(self->iter_curr != NULL);
}
/* }}} */

/* {{{ Set::rewind() */
PHP_METHOD(Set, rewind) {
  php_driver_set* self = PHP_DRIVER_GET_SET(getThis());
  self->iter_curr = self->entries;
  self->iter_temp = self->entries != NULL ? (php_driver_set_entry*)self->entries->hh.next : NULL;
  self->iter_index = 0;
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_one, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, value)
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

static zend_function_entry php_driver_set_methods[] = {
    PHP_ME(Set, __construct, arginfo__construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC) PHP_ME(
        Set, type, arginfo_none, ZEND_ACC_PUBLIC) PHP_ME(Set, values, arginfo_none, ZEND_ACC_PUBLIC)
        PHP_ME(Set, add, arginfo_one, ZEND_ACC_PUBLIC)
            PHP_ME(Set, has, arginfo_one, ZEND_ACC_PUBLIC)
                PHP_ME(Set, remove, arginfo_one, ZEND_ACC_PUBLIC)
    /* Countable */
    PHP_ME(Set, count, arginfo_count, ZEND_ACC_PUBLIC)
    /* Iterator */
    PHP_ME(Set, current, arginfo_current, ZEND_ACC_PUBLIC) PHP_ME(
        Set, key, arginfo_key, ZEND_ACC_PUBLIC) PHP_ME(Set, next, arginfo_next, ZEND_ACC_PUBLIC)
        PHP_ME(Set, valid, arginfo_valid, ZEND_ACC_PUBLIC)
            PHP_ME(Set, rewind, arginfo_rewind, ZEND_ACC_PUBLIC) PHP_FE_END};

static zend_object_handlers php_driver_set_handlers;

static HashTable* php_driver_set_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object* object,
#else
    zendObject* object,
#endif
    zval** table, int* n) {
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable* php_driver_set_properties(
#if PHP_MAJOR_VERSION >= 8
    zend_object* object
#else
    zendObject* object
#endif
) {
  zval values;

#if PHP_MAJOR_VERSION >= 8
  php_driver_set* self = PHP5TO7_ZEND_OBJECT_GET(set, object);
#else
  php_driver_set* self = PHP_DRIVER_GET_SET(object);
#endif
  HashTable* props = zend_std_get_properties(object);

  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &self->type, sizeof(zval));
  Z_ADDREF_P(&self->type);

  array_init(&values);
  php_driver_set_populate(self, &values);
  zend_hash_sort(Z_ARRVAL_P(&values), php_driver_data_compare, 1);
  PHP5TO7_ZEND_HASH_UPDATE(props, "values", sizeof("values"), &values, sizeof(zval));

  return props;
}

static int php_driver_set_compare(zval* obj1, zval* obj2) {
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  php_driver_set_entry *curr, *temp;
  php_driver_set* set1;
  php_driver_set* set2;
  php_driver_type* type1;
  php_driver_type* type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  set1 = PHP_DRIVER_GET_SET(obj1);
  set2 = PHP_DRIVER_GET_SET(obj2);

  type1 = PHP_DRIVER_GET_TYPE(&set1->type);
  type2 = PHP_DRIVER_GET_TYPE(&set2->type);

  result = php_driver_type_compare(type1, type2);
  if (result != 0) return result;

  if (HASH_COUNT(set1->entries) != HASH_COUNT(set2->entries)) {
    return HASH_COUNT(set1->entries) < HASH_COUNT(set2->entries) ? -1 : 1;
  }

  HASH_ITER(hh, set1->entries, curr, temp) {
    php_driver_set_entry* entry;
    HASH_FIND_ZVAL(set2->entries, &curr->value, entry);
    if (entry == NULL) {
      return 1;
    }
  }

  return 0;
}

static void php_driver_set_free(zend_object* object) {
  php_driver_set* self = PHP5TO7_ZEND_OBJECT_GET(set, object);
  php_driver_set_entry *curr, *temp;

  HASH_ITER(hh, self->entries, curr, temp) {
    zval_ptr_dtor(&curr->value);
    HASH_DEL(self->entries, curr);
    efree(curr);
  }

  PHP5TO7_ZVAL_MAYBE_DESTROY(self->type);

  zend_object_std_dtor(&self->zendObject);
}

static zend_object* php_driver_set_new(zend_class_entry* ce) {
  php_driver_set* self = PHP5TO7_ZEND_OBJECT_ECALLOC(set, ce);

  self->entries = self->iter_curr = self->iter_temp = NULL;
  self->iter_index = 0;
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  PHP5TO7_ZEND_OBJECT_INIT(set, self, ce);
}

void php_driver_define_Set() {
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Set", php_driver_set_methods);
  php_driver_set_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_set_ce, 3, php_driver_value_ce, zend_ce_countable,
                        zend_ce_iterator);
  memcpy(&php_driver_set_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_set_handlers.get_properties = php_driver_set_properties;
  php_driver_set_handlers.get_gc = php_driver_set_gc;
  php_driver_set_handlers.compare = php_driver_set_compare;
  php_driver_set_ce->ce_flags |= ZEND_ACC_FINAL;
  php_driver_set_ce->create_object = php_driver_set_new;
  php_driver_set_handlers.clone_obj = NULL;
}

END_EXTERN_C()
