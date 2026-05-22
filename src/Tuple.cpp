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

#include "src/Type/Tuple.h"

#include "Tuple.h"
#include "Zend/zend_hash.h"
#include "php_driver.h"
#include "php_driver_types.h"
#include "util/collections.h"
#include "util/hash.h"
#include "util/types.h"
BEGIN_EXTERN_C()
#include "Tuple_arginfo.h"
zend_class_entry *php_driver_tuple_ce = NULL;

void
php_driver_tuple_set(php_driver_tuple *tuple, ulong index, zval *object )
{
  (void)zend_hash_index_update(&tuple->values, index, object);
  Z_TRY_ADDREF_P(object);
  tuple->dirty = 1;
}

static void
php_driver_tuple_populate(php_driver_tuple *tuple, zval *array )
{
  zend_ulong index;
  php_driver_type *type;
  zval *current;
  zval null;


  ZVAL_NULL(&null);

  type = PHP_DRIVER_GET_TYPE(&tuple->type);

  ZEND_HASH_FOREACH_NUM_KEY_VAL(&type->data.tuple.types, index, current) {
    zval *value = NULL;
    (void) current;
    if ((value = zend_hash_index_find(&tuple->values, (zend_ulong)(index))) != NULL) {
      if (add_next_index_zval(array, value) == SUCCESS)
        Z_TRY_ADDREF_P(value);
      else
        break;
    } else {
      if (add_next_index_zval(array, &null) == SUCCESS)
        Z_TRY_ADDREF_P(&null);
      else
        break;
    }
  } ZEND_HASH_FOREACH_END();

#if PHP_MAJOR_VERSION < 7
  zval_ptr_dtor(&null);
#endif
}

/* {{{ Tuple::__construct(types) */
ZEND_METHOD(Cassandra_Tuple, __construct)
{
  php_driver_tuple *self;
  php_driver_type *type;
  HashTable *types;
  zval *current;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY_HT(types)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_TUPLE(getThis());
  self->type = php_driver_type_tuple();
  type = PHP_DRIVER_GET_TYPE(&self->type);

  ZEND_HASH_FOREACH_VAL(types, current) {
    zval *sub_type = current;
    zval scalar_type;

    if (Z_TYPE_P(sub_type) == IS_STRING) {
      CassValueType value_type;
      if (!php_driver_value_type(Z_STRVAL_P(sub_type), &value_type )) {
        return;
      }
      scalar_type = php_driver_type_scalar(value_type );
      if (!php_driver_type_tuple_add(type,
                                        &scalar_type )) {
        return;
      }
    } else if (Z_TYPE_P(sub_type) == IS_OBJECT &&
               instanceof_function(Z_OBJCE_P(sub_type), php_driver_type_ce )) {
      if (!php_driver_type_validate(sub_type, "type" )) {
        return;
      }
      if (php_driver_type_tuple_add(type,
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

/* {{{ Tuple::type() */
ZEND_METHOD(Cassandra_Tuple, type)
{
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}

/* {{{ Tuple::values() */
ZEND_METHOD(Cassandra_Tuple, values)
{
  php_driver_tuple *self = NULL;
  array_init(return_value);
  self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_tuple_populate(self, return_value );
}
/* }}} */

/* {{{ Tuple::set(int, mixed) */
ZEND_METHOD(Cassandra_Tuple, set)
{
  php_driver_tuple *self = NULL;
  zend_long index;
  php_driver_type *type;
  zval *sub_type;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_LONG(index)
    Z_PARAM_ZVAL(value)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_TUPLE(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  if (index < 0 || index >= zend_hash_num_elements(&type->data.tuple.types)) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                            "Index out of bounds");
    return;
  }

  if ((sub_type = zend_hash_index_find(&type->data.tuple.types, (zend_ulong)(index))) == NULL ||
      !php_driver_validate_object(value,
                                  sub_type )) {
    return;
  }

  php_driver_tuple_set(self, index, value );
}
/* }}} */

/* {{{ Tuple::get(int) */
ZEND_METHOD(Cassandra_Tuple, get)
{
  php_driver_tuple *self = NULL;
  zend_long index;
  php_driver_type *type;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(index)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_TUPLE(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  if (index < 0 || index >= zend_hash_num_elements(&type->data.tuple.types)) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                            "Index out of bounds");
    return;
  }

  if ((value = zend_hash_index_find(&self->values, (zend_ulong)(index))) != NULL) {
    RETURN_ZVAL(value, 1, 0);
  }
}
/* }}} */

/* {{{ Tuple::count() */
ZEND_METHOD(Cassandra_Tuple, count)
{
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  RETURN_LONG(zend_hash_num_elements(&type->data.tuple.types));
}
/* }}} */

/* {{{ Tuple::current() */
ZEND_METHOD(Cassandra_Tuple, current)
{
  zend_ulong index;
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);

  if (zend_hash_get_current_key_ex(&type->data.tuple.types, NULL, &index, &self->pos) == HASH_KEY_IS_LONG) {
    zval *value;
    if ((value = zend_hash_index_find(&self->values, (zend_ulong)(index))) != NULL) {
      RETURN_ZVAL(value, 1, 0);
    }
  }
}
/* }}} */

/* {{{ Tuple::key() */
ZEND_METHOD(Cassandra_Tuple, key)
{
  zend_ulong index;
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  if (zend_hash_get_current_key_ex(&type->data.tuple.types, NULL, &index, &self->pos) == HASH_KEY_IS_LONG) {
    RETURN_LONG(index);
  }
}
/* }}} */

/* {{{ Tuple::next() */
ZEND_METHOD(Cassandra_Tuple, next)
{
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  zend_hash_move_forward_ex(&type->data.tuple.types, &self->pos);
}
/* }}} */

/* {{{ Tuple::valid() */
ZEND_METHOD(Cassandra_Tuple, valid)
{
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  RETURN_BOOL(zend_hash_has_more_elements_ex(&type->data.tuple.types, &self->pos) == SUCCESS);
}
/* }}} */

/* {{{ Tuple::rewind() */
ZEND_METHOD(Cassandra_Tuple, rewind)
{
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(getThis());
  php_driver_type *type = PHP_DRIVER_GET_TYPE(&self->type);
  zend_hash_internal_pointer_reset_ex(&type->data.tuple.types, &self->pos);
}
/* }}} */


static php_driver_value_handlers php_driver_tuple_handlers;

static HashTable *
php_driver_tuple_gc(
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
php_driver_tuple_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
  zval values;

#if PHP_MAJOR_VERSION >= 8
  php_driver_tuple  *self = php_driver_tuple_object_fetch(object);
#else
  php_driver_tuple  *self = PHP_DRIVER_GET_TUPLE(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, ZEND_STRL("type"), &self->type);
  Z_ADDREF_P(&self->type);


  array_init(&values);
  php_driver_tuple_populate(self, &values );
  (void)zend_hash_str_update(props, ZEND_STRL("values"), &values);

  return props;
}

static int
php_driver_tuple_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  HashPosition pos1;
  HashPosition pos2;
  zval *current1;
  zval *current2;
  php_driver_tuple *tuple1;
  php_driver_tuple *tuple2;
  php_driver_type *type1;
  php_driver_type *type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  tuple1 = PHP_DRIVER_GET_TUPLE(obj1);
  tuple2 = PHP_DRIVER_GET_TUPLE(obj2);

  type1 = PHP_DRIVER_GET_TYPE(&tuple1->type);
  type2 = PHP_DRIVER_GET_TYPE(&tuple2->type);

  result = php_driver_type_compare(type1, type2 );
  if (result != 0) return result;

  if (zend_hash_num_elements(&tuple1->values) != zend_hash_num_elements(&tuple2->values)) {
    return zend_hash_num_elements(&tuple1->values) < zend_hash_num_elements(&tuple2->values) ? -1 : 1;
  }

  zend_hash_internal_pointer_reset_ex(&tuple1->values, &pos1);
  zend_hash_internal_pointer_reset_ex(&tuple2->values, &pos2);

  while ((current1 = zend_hash_get_current_data_ex(&tuple1->values, &pos1)) != NULL &&
         (current2 = zend_hash_get_current_data_ex(&tuple2->values, &pos2)) != NULL) {
    result = php_driver_value_compare(current1,
                                         current2 );
    if (result != 0) return result;
    zend_hash_move_forward_ex(&tuple1->values, &pos1);
    zend_hash_move_forward_ex(&tuple2->values, &pos2);
  }

  return 0;
}

static unsigned
php_driver_tuple_hash_value(zval *obj )
{
  zval *current;
  unsigned hashv = 0;
  php_driver_tuple *self = PHP_DRIVER_GET_TUPLE(obj);

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
php_driver_tuple_free(zend_object *object )
{
  php_driver_tuple *self =
      php_driver_tuple_object_fetch(object);

  zend_hash_destroy(&self->values);
  zval_ptr_dtor(&self->type);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_tuple_new(zend_class_entry *ce )
{
  php_driver_tuple *self =
      (php_driver_tuple *)ecalloc(1, sizeof(php_driver_tuple) + zend_object_properties_size(ce));

  zend_hash_init(&self->values, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_MAJOR_VERSION >= 7
  self->pos = HT_INVALID_IDX;
#else
  self->pos = NULL;
#endif
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  zend_object_std_init(&self->zendObject, ce);
  php_driver_tuple_handlers.std.offset = XtOffsetOf(php_driver_tuple, zendObject);
  php_driver_tuple_handlers.std.free_obj = php_driver_tuple_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_tuple_handlers;
  return &self->zendObject;
}

void php_driver_define_Tuple()
{
  php_driver_tuple_ce = register_class_Cassandra_Tuple(php_driver_value_ce, zend_ce_countable, zend_ce_iterator);
  memcpy(&php_driver_tuple_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_tuple_handlers.std.get_properties  = php_driver_tuple_properties;
  php_driver_tuple_handlers.std.get_gc          = php_driver_tuple_gc;
  php_driver_tuple_handlers.std.compare = php_driver_tuple_compare;
  php_driver_tuple_ce->create_object = php_driver_tuple_new;

  php_driver_tuple_handlers.hash_value = php_driver_tuple_hash_value;
  php_driver_tuple_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()