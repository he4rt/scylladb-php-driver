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

#include <php_scylladb.h>
#include <php_scylladb_types.h>

#include "Type/Conversions.h"
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"

#include "Collection.h"


BEGIN_EXTERN_C()
#include "Collection_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_collection_handlers;

void
php_scylladb_collection_add(php_scylladb_collection *collection, zval *object)
{
  (void)zend_hash_next_index_insert(&collection->values, object);
  Z_TRY_ADDREF_P(object);
  collection->dirty = 1;
}

static int
php_scylladb_collection_del(php_scylladb_collection *collection, ulong index)
{
  if (zend_hash_index_del(&collection->values, index) == SUCCESS) {
    collection->dirty = 1;
    return 1;
  }

  return 0;
}

static int
php_scylladb_collection_get(php_scylladb_collection *collection, ulong index, zval *zvalue)
{
  zval *value;
  if ((value = zend_hash_index_find(&collection->values, (zend_ulong)(index))) != NULL) {
    *zvalue = *value;
    return 1;
  }
  return 0;
}

static int
php_scylladb_collection_find(php_scylladb_collection *collection, zval *object, long *index)
{
  zend_ulong num_key;
  zval *current;
  ZEND_HASH_FOREACH_NUM_KEY_VAL(&collection->values, num_key, current) {
    zval compare;
    is_equal_function(&compare, object, current);
    if ((Z_TYPE_P(&compare) == IS_TRUE)) {
      *index = (long) num_key;
      return 1;
    }
  } ZEND_HASH_FOREACH_END();

  return 0;
}

static void
php_scylladb_collection_populate(php_scylladb_collection *collection, zval *array)
{
  zval *current;
  ZEND_HASH_FOREACH_VAL(&collection->values, current) {
    if (add_next_index_zval(array, current) == SUCCESS)
      Z_TRY_ADDREF_P(current);
    else
      break;
  } ZEND_HASH_FOREACH_END();
}

/* {{{ Collection::__construct(type) */
PHP_METHOD(Cassandra_Collection, __construct)
{
  php_scylladb_collection *self;
  zval *type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(type)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_COLLECTION(getThis());

  if (Z_TYPE_P(type) == IS_STRING) {
    CassValueType value_type;
    if (!php_scylladb_value_type(Z_STRVAL_P(type), &value_type))
      return;
    self->type = php_scylladb_type_collection_from_value_type(value_type);
  } else if (Z_TYPE_P(type) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(type), php_scylladb_type_ce)) {
    if (!php_scylladb_type_validate(type, "type")) {
      return;
    }
    self->type = php_scylladb_type_collection(type);
    Z_ADDREF_P(type);
  } else {
    INVALID_ARGUMENT(type, "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Type");
  }
}
/* }}} */

/* {{{ Collection::type() */
PHP_METHOD(Cassandra_Collection, type)
{
  php_scylladb_collection *self = PHP_SCYLLADB_GET_COLLECTION(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}

/* {{{ Collection::values() */
PHP_METHOD(Cassandra_Collection, values)
{
  php_scylladb_collection *collection = NULL;
  array_init(return_value);
  collection = PHP_SCYLLADB_GET_COLLECTION(getThis());
  php_scylladb_collection_populate(collection, return_value);
}
/* }}} */

/* {{{ Collection::add(mixed) */
PHP_METHOD(Cassandra_Collection, add)
{
  php_scylladb_collection *self = NULL;
  zval* args = NULL;
  int argc = 0, i;
  php_scylladb_type *type;

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_VARIADIC('+', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_COLLECTION(getThis());
  type = PHP_SCYLLADB_GET_TYPE(&self->type);

  for (i = 0; i < argc; i++) {
    if (Z_TYPE_P(&args[i]) == IS_NULL) {

      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "Invalid value: null is not supported inside collections");
      RETURN_FALSE;
    }

    if (!php_scylladb_validate_object(&args[i],
                                    &type->data.collection.value_type)) {

      RETURN_FALSE;
    }
  }

  for (i = 0; i < argc; i++) {
    php_scylladb_collection_add(self, &args[i]);
  }


  RETVAL_LONG(zend_hash_num_elements(&self->values));
}
/* }}} */

/* {{{ Collection::get(int) */
PHP_METHOD(Cassandra_Collection, get)
{
  zend_long key;
  php_scylladb_collection *self = NULL;
  zval value;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_COLLECTION(getThis());

  if (php_scylladb_collection_get(self, (ulong) key, &value))
    RETURN_ZVAL(&value, 1, 0);
}
/* }}} */

/* {{{ Collection::find(mixed) */
PHP_METHOD(Cassandra_Collection, find)
{
  zval *object;
  php_scylladb_collection *collection = NULL;
  long index;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(object)
  ZEND_PARSE_PARAMETERS_END();

  collection = PHP_SCYLLADB_GET_COLLECTION(getThis());

  if (php_scylladb_collection_find(collection, object, &index))
    RETURN_LONG(index);
}
/* }}} */

/* {{{ Collection::count() */
PHP_METHOD(Cassandra_Collection, count)
{
  php_scylladb_collection *collection = PHP_SCYLLADB_GET_COLLECTION(getThis());
  RETURN_LONG(zend_hash_num_elements(&collection->values));
}
/* }}} */

/* {{{ Collection::current() */
PHP_METHOD(Cassandra_Collection, current)
{
  zval *current;
  php_scylladb_collection *collection = PHP_SCYLLADB_GET_COLLECTION(getThis());

  if ((current = zend_hash_get_current_data(&collection->values)) != NULL) {
    RETURN_ZVAL(current, 1, 0);
  }
}
/* }}} */

/* {{{ Collection::key() */
PHP_METHOD(Cassandra_Collection, key)
{
  zend_ulong num_key;
  php_scylladb_collection *collection = PHP_SCYLLADB_GET_COLLECTION(getThis());
  if (zend_hash_get_current_key(&collection->values, NULL, &num_key) == HASH_KEY_IS_LONG) {
    RETURN_LONG(num_key);
  }
}
/* }}} */

/* {{{ Collection::next() */
PHP_METHOD(Cassandra_Collection, next)
{
  php_scylladb_collection *collection = PHP_SCYLLADB_GET_COLLECTION(getThis());
  zend_hash_move_forward(&collection->values);
}
/* }}} */

/* {{{ Collection::valid() */
PHP_METHOD(Cassandra_Collection, valid)
{
  php_scylladb_collection *collection = PHP_SCYLLADB_GET_COLLECTION(getThis());
  RETURN_BOOL(zend_hash_has_more_elements(&collection->values) == SUCCESS);
}
/* }}} */

/* {{{ Collection::rewind() */
PHP_METHOD(Cassandra_Collection, rewind)
{
  php_scylladb_collection *collection = PHP_SCYLLADB_GET_COLLECTION(getThis());
  zend_hash_internal_pointer_reset(&collection->values);
}
/* }}} */

/* {{{ Collection::remove(key) */
PHP_METHOD(Cassandra_Collection, remove)
{
  zend_long index;
  php_scylladb_collection *collection = NULL;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(index)
  ZEND_PARSE_PARAMETERS_END();

  collection = PHP_SCYLLADB_GET_COLLECTION(getThis());

  if (php_scylladb_collection_del(collection, (ulong) index)) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */



HashTable *
php_scylladb_collection_gc(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object,
#else
        zendObject *object,
#endif
        zval** table, int *n)
{
  return zend_std_get_gc(object, table, n);
}

HashTable *
php_scylladb_collection_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
  zval values;

#if PHP_MAJOR_VERSION >= 8
  php_scylladb_collection  *self = php_scylladb_collection_object_fetch(object);
#else
  php_scylladb_collection  *self = PHP_SCYLLADB_GET_COLLECTION(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  Z_TRY_ADDREF_P(&self->type);
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &self->type);


  array_init(&values);
  php_scylladb_collection_populate(self, &values);
  (void)zend_hash_str_update(props, ZEND_STRL("values"), &values);

  return props;
}

int
php_scylladb_collection_compare(zval *obj1, zval *obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  HashPosition pos1;
  HashPosition pos2;
  zval *current1;
  zval *current2;
  php_scylladb_collection *collection1;
  php_scylladb_collection *collection2;
  php_scylladb_type *type1;
  php_scylladb_type *type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  collection1 = PHP_SCYLLADB_GET_COLLECTION(obj1);
  collection2 = PHP_SCYLLADB_GET_COLLECTION(obj2);

  type1 = PHP_SCYLLADB_GET_TYPE(&collection1->type);
  type2 = PHP_SCYLLADB_GET_TYPE(&collection2->type);

  result = php_scylladb_type_compare(type1, type2);
  if (result != 0) return result;

  if (zend_hash_num_elements(&collection1->values) != zend_hash_num_elements(&collection2->values)) {
    return zend_hash_num_elements(&collection1->values) < zend_hash_num_elements(&collection2->values) ? -1 : 1;
  }

  zend_hash_internal_pointer_reset_ex(&collection1->values, &pos1);
  zend_hash_internal_pointer_reset_ex(&collection2->values, &pos2);

  while ((current1 = zend_hash_get_current_data_ex(&collection1->values, &pos1)) != NULL &&
         (current2 = zend_hash_get_current_data_ex(&collection2->values, &pos2)) != NULL) {
    result = php_scylladb_value_compare(current1,
                                         current2);
    if (result != 0) return result;
    zend_hash_move_forward_ex(&collection1->values, &pos1);
    zend_hash_move_forward_ex(&collection2->values, &pos2);
  }

  return 0;
}

unsigned
php_scylladb_collection_hash_value(zval *obj)
{
  zval *current;
  unsigned hashv = 0;
  php_scylladb_collection *self = PHP_SCYLLADB_GET_COLLECTION(obj);

  if (!self->dirty) return self->hashv;

  ZEND_HASH_FOREACH_VAL(&self->values, current) {
    hashv = php_scylladb_combine_hash(hashv,
                                       php_scylladb_value_hash(current));
  } ZEND_HASH_FOREACH_END();

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

void
php_scylladb_collection_free(zend_object *object)
{
  php_scylladb_collection *self =
      php_scylladb_collection_object_fetch(object);

  zend_hash_destroy(&self->values);
  zval_ptr_dtor(&self->type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_collection_new(zend_class_entry *ce)
{
  php_scylladb_collection *self =
      (php_scylladb_collection *)ecalloc(1, sizeof(php_scylladb_collection) + zend_object_properties_size(ce));

  zend_hash_init(&self->values, 0, NULL, ZVAL_PTR_DTOR, 0);
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  zend_object_std_init(&self->zendObject, ce);
  php_scylladb_collection_handlers.std.offset = XtOffsetOf(php_scylladb_collection, zendObject);
  php_scylladb_collection_handlers.std.free_obj = php_scylladb_collection_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_scylladb_collection_handlers;
  return &self->zendObject;
}


void php_scylladb_collection_post_register(zend_class_entry *ce)
{
    (void)ce;
    php_scylladb_collection_handlers.std.offset = XtOffsetOf(php_scylladb_collection, zendObject);
}
END_EXTERN_C()
