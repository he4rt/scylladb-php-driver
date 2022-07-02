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

#include "Collection.h"
#include "php_driver.h"
#include "php_driver_types.h"
#include "util/collections.h"
#include "util/hash.h"
#include "util/types.h"

zend_class_entry *php_driver_collection_ce = NULL;

void
php_driver_collection_add(php_driver_collection* collection, zval* object)
{
  PHP5TO7_ZEND_HASH_NEXT_INDEX_INSERT(&collection->values, object, sizeof(zval *));
  Z_TRY_ADDREF_P(object);
  collection->dirty = 1;
}

static int
php_driver_collection_del(php_driver_collection *collection, uint64_t index)
{
  if (zend_hash_index_del(&collection->values, index) == SUCCESS) {
    collection->dirty = 1;
    return 1;
  }

  return 0;
}

static int
php_driver_collection_get(php_driver_collection* collection, uint64_t index, zval* zvalue)
{
  zval* value;
  if (PHP5TO7_ZEND_HASH_INDEX_FIND(&collection->values, index, value)) {
    *zvalue = *value;
    return 1;
  }
  return 0;
}

static int
php_driver_collection_find(php_driver_collection* collection, zval* object, long* index)
{
  zend_ulong num_key;
  zval* current;
  PHP5TO7_ZEND_HASH_FOREACH_NUM_KEY_VAL(&collection->values, num_key, current) {
    zval compare;
    is_equal_function(&compare, object, PHP5TO7_ZVAL_MAYBE_DEREF(current));
    if (PHP5TO7_ZVAL_IS_TRUE_P(&compare)) {
      *index = (long) num_key;
      return 1;
    }
  } PHP5TO7_ZEND_HASH_FOREACH_END(&collection->values);

  return 0;
}

static void
php_driver_collection_populate(php_driver_collection *collection, zval *array)
{
  zval* current;
  PHP5TO7_ZEND_HASH_FOREACH_VAL(&collection->values, current) {
    if (add_next_index_zval(array, PHP5TO7_ZVAL_MAYBE_DEREF(current)) == SUCCESS)
      Z_TRY_ADDREF_P(PHP5TO7_ZVAL_MAYBE_DEREF(current));
    else
      break;
  } PHP5TO7_ZEND_HASH_FOREACH_END(&collection->values);
}

/* {{{ Collection::__construct(type) */
PHP_METHOD(Collection, __construct)
{
  php_driver_collection *self;
  zval *type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &type) == FAILURE)
    return;

  self = PHP_DRIVER_GET_COLLECTION(getThis());

  if (Z_TYPE_P(type) == IS_STRING) {
    CassValueType value_type;
    if (!php_driver_value_type(Z_STRVAL_P(type), &value_type))
      return;
    self->type = php_driver_type_collection_from_value_type(value_type);
  } else if (Z_TYPE_P(type) == IS_OBJECT && instanceof_function(Z_OBJCE_P(type), php_driver_type_ce)) {
    if (!php_driver_type_validate(type, "type")) {
      return;
    }
    self->type = php_driver_type_collection(type);
    Z_ADDREF_P(type);
  } else {
    INVALID_ARGUMENT(type, "a string or an instance of " PHP_DRIVER_NAMESPACE "\\Type");
  }
}
/* }}} */

/* {{{ Collection::type() */
PHP_METHOD(Collection, type)
{
  php_driver_collection *self = PHP_DRIVER_GET_COLLECTION(getThis());
  RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->type), 1, 0);
}

/* {{{ Collection::values() */
PHP_METHOD(Collection, values)
{
  php_driver_collection *collection = NULL;
  array_init(return_value);
  collection = PHP_DRIVER_GET_COLLECTION(getThis());
  php_driver_collection_populate(collection, return_value);
}
/* }}} */

/* {{{ Collection::add(mixed) */
PHP_METHOD(Collection, add)
{
  php_driver_collection *self = NULL;
  zval* args = NULL;
  int argc = 0, i;
  php_driver_type *type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE)
    return;

  self = PHP_DRIVER_GET_COLLECTION(getThis());
  type = PHP_DRIVER_GET_TYPE(PHP5TO7_ZVAL_MAYBE_P(self->type));

  for (i = 0; i < argc; i++) {
    if (Z_TYPE_P(PHP5TO7_ZVAL_ARG(args[i])) == IS_NULL) {
            zend_throw_exception_ex(spl_ce_LogicException, 0,
                              "Invalid value: null is not supported inside collections");
      RETURN_FALSE;
    }

    if (!php_driver_validate_object(PHP5TO7_ZVAL_ARG(args[i]),
                                    PHP5TO7_ZVAL_MAYBE_P(type->data.collection.value_type))) {
            RETURN_FALSE;
    }
  }

  for (i = 0; i < argc; i++) {
    php_driver_collection_add(self, PHP5TO7_ZVAL_ARG(args[i]));
  }

    RETVAL_LONG(zend_hash_num_elements(&self->values));
}
/* }}} */

/* {{{ Collection::get(int) */
PHP_METHOD(Collection, get)
{
  long key;
  php_driver_collection *self = NULL;
  zval value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &key) == FAILURE)
    return;

  self = PHP_DRIVER_GET_COLLECTION(getThis());

  if (php_driver_collection_get(self, (uint64_t) key, &value))
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(value), 1, 0);
}
/* }}} */

/* {{{ Collection::find(mixed) */
PHP_METHOD(Collection, find)
{
  zval *object;
  php_driver_collection *collection = NULL;
  long index;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &object) == FAILURE)
    return;

  collection = PHP_DRIVER_GET_COLLECTION(getThis());

  if (php_driver_collection_find(collection, object, &index))
    RETURN_LONG(index);
}
/* }}} */

/* {{{ Collection::count() */
PHP_METHOD(Collection, count)
{
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  RETURN_LONG(zend_hash_num_elements(&collection->values));
}
/* }}} */

/* {{{ Collection::current() */
PHP_METHOD(Collection, current)
{
  zval* current;
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());

  if (PHP5TO7_ZEND_HASH_GET_CURRENT_DATA(&collection->values, current)) {
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_DEREF(current), 1, 0);
  }
}
/* }}} */

/* {{{ Collection::key() */
PHP_METHOD(Collection, key)
{
  zend_ulong num_key;
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  if (PHP5TO7_ZEND_HASH_GET_CURRENT_KEY(&collection->values, NULL, &num_key) == HASH_KEY_IS_LONG) {
    RETURN_LONG(num_key);
  }
}
/* }}} */

/* {{{ Collection::next() */
PHP_METHOD(Collection, next)
{
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  zend_hash_move_forward(&collection->values);
}
/* }}} */

/* {{{ Collection::valid() */
PHP_METHOD(Collection, valid)
{
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  RETURN_BOOL(zend_hash_has_more_elements(&collection->values) == SUCCESS);
}
/* }}} */

/* {{{ Collection::rewind() */
PHP_METHOD(Collection, rewind)
{
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  zend_hash_internal_pointer_reset(&collection->values);
}
/* }}} */

/* {{{ Collection::remove(key) */
PHP_METHOD(Collection, remove)
{
  long index;
  php_driver_collection *collection = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
    return;
  }

  collection = PHP_DRIVER_GET_COLLECTION(getThis());

  if (php_driver_collection_del(collection, (uint64_t) index)) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_value, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

#if PHP_MAJOR_VERSION >= 8
ZEND_BEGIN_ARG_INFO_EX(arginfo_values, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_VARIADIC_INFO(0, value)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_index, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_collection_methods[] = {
  PHP_ME(Collection, __construct, arginfo__construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
  PHP_ME(Collection, values, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, add, arginfo_values, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, get, arginfo_index, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, find, arginfo_value, ZEND_ACC_PUBLIC)
  /* Countable */
  PHP_ME(Collection, count, arginfo_none, ZEND_ACC_PUBLIC)
  /* Iterator */
  PHP_ME(Collection, current, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, key, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, next, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, valid, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, rewind, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Collection, remove, arginfo_index, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static php_driver_value_handlers php_driver_collection_handlers;

static HashTable*
php_driver_collection_gc(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object,
#else
  zval* object,
#endif
  zval** table,
  int* n)
{
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *
php_driver_collection_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
  zval* object
#endif
)
{
  zval values;

#if PHP_MAJOR_VERSION >= 8
  php_driver_collection  *self = PHP5TO7_ZEND_OBJECT_GET(collection, object);
#else
  php_driver_collection  *self = PHP_DRIVER_GET_COLLECTION(object);
#endif
  HashTable* props = zend_std_get_properties(object);

  PHP5TO7_ZEND_HASH_UPDATE(props,
                           "type", sizeof("type"),
                           PHP5TO7_ZVAL_MAYBE_P(self->type), sizeof(zval));
  Z_ADDREF_P(PHP5TO7_ZVAL_MAYBE_P(self->type));


  array_init(PHP5TO7_ZVAL_MAYBE_P(values));
  php_driver_collection_populate(self, PHP5TO7_ZVAL_MAYBE_P(values));
  PHP5TO7_ZEND_HASH_UPDATE(props, "values", sizeof("values"), PHP5TO7_ZVAL_MAYBE_P(values), sizeof(zval));

  return props;
}

static int
php_driver_collection_compare(zval* obj1, zval* obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  HashPosition pos1;
  HashPosition pos2;
  zval* current1;
  zval* current2;
  php_driver_collection *collection1;
  php_driver_collection *collection2;
  php_driver_type *type1;
  php_driver_type *type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  collection1 = PHP_DRIVER_GET_COLLECTION(obj1);
  collection2 = PHP_DRIVER_GET_COLLECTION(obj2);

  type1 = PHP_DRIVER_GET_TYPE(PHP5TO7_ZVAL_MAYBE_P(collection1->type));
  type2 = PHP_DRIVER_GET_TYPE(PHP5TO7_ZVAL_MAYBE_P(collection2->type));

  result = php_driver_type_compare(type1, type2);
  if (result != 0) return result;

  if (zend_hash_num_elements(&collection1->values) != zend_hash_num_elements(&collection2->values)) {
    return zend_hash_num_elements(&collection1->values) < zend_hash_num_elements(&collection2->values) ? -1 : 1;
  }

  zend_hash_internal_pointer_reset_ex(&collection1->values, &pos1);
  zend_hash_internal_pointer_reset_ex(&collection2->values, &pos2);

  while (PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&collection1->values, current1, &pos1) &&
         PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&collection2->values, current2, &pos2)) {
    result = php_driver_value_compare(PHP5TO7_ZVAL_MAYBE_DEREF(current1),
                                      PHP5TO7_ZVAL_MAYBE_DEREF(current2));
    if (result != 0) return result;
    zend_hash_move_forward_ex(&collection1->values, &pos1);
    zend_hash_move_forward_ex(&collection2->values, &pos2);
  }

  return 0;
}

static unsigned
php_driver_collection_hash_value(zval* obj)
{
  zval* current;
  unsigned hashv = 0;
  php_driver_collection *self = PHP_DRIVER_GET_COLLECTION(obj);

  if (!self->dirty) return self->hashv;

  PHP5TO7_ZEND_HASH_FOREACH_VAL(&self->values, current) {
    hashv = php_driver_combine_hash(hashv,
                                    php_driver_value_hash(PHP5TO7_ZVAL_MAYBE_DEREF(current)));
  } PHP5TO7_ZEND_HASH_FOREACH_END(&self->values);

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

static void
php_driver_collection_free(zend_object* object)
{
  php_driver_collection *self =
      PHP5TO7_ZEND_OBJECT_GET(collection, object);

  zend_hash_destroy(&self->values);
  ZVAL_DESTROY(self->type);

  zend_object_std_dtor(&self->zval);
  }

static zend_object*
php_driver_collection_new(zend_class_entry* ce)
{
  php_driver_collection *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(collection, ce);

  zend_hash_init(&self->values, 0, NULL, ZVAL_PTR_DTOR, 0);
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  PHP5TO7_ZEND_OBJECT_INIT(collection, self, ce);
}

void
php_driver_define_Collection()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Collection", php_driver_collection_methods);
  php_driver_collection_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_collection_ce, 1, php_driver_value_ce);
  memcpy(&php_driver_collection_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_collection_handlers.std.get_properties  = php_driver_collection_properties;
#if PHP_VERSION_ID >= 50400
  php_driver_collection_handlers.std.get_gc          = php_driver_collection_gc;
#endif
#if PHP_MAJOR_VERSION >= 8
  php_driver_collection_handlers.std.compare = php_driver_collection_compare;
#else
  php_driver_collection_handlers.std.compare_objects = php_driver_collection_compare;
#endif
  php_driver_collection_ce->ce_flags |= ZEND_ACC_FINAL;
  php_driver_collection_ce->create_object = php_driver_collection_new;
#if PHP_VERSION_ID < 80100
  zend_class_implements(php_driver_collection_ce, 2, spl_ce_Countable, zend_ce_iterator);
#else
  zend_class_implements(php_driver_collection_ce, 2, zend_ce_countable, zend_ce_iterator);
#endif

  php_driver_collection_handlers.hash_value = php_driver_collection_hash_value;
  php_driver_collection_handlers.std.clone_obj = NULL;
}
