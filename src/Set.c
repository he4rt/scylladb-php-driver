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

#include "src/Set.h"

#include "Set_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_set_handlers;

int
php_scylladb_set_add(php_scylladb_set* set, zval* object)
{
  php_scylladb_set_entry* entry;
  php_scylladb_type* type;

  if (Z_TYPE_P(object) == IS_NULL) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                            "Invalid value: null is not supported inside sets");
    return 0;
  }

  type = PHP_SCYLLADB_GET_TYPE(&set->type);

  if (!php_scylladb_validate_object(object, &type->data.set.value_type )) {
    return 0;
  }

  HASH_FIND_ZVAL(set->entries, object, entry);
  if (entry == nullptr) {
    set->dirty = 1;
    entry      = (php_scylladb_set_entry*) emalloc(sizeof(php_scylladb_set_entry));
    ZVAL_COPY(&entry->value, object);
    HASH_ADD_ZVAL(set->entries, value, entry);
  }

  return 1;
}

static int
php_scylladb_set_del(php_scylladb_set* set, zval* object)
{
  php_scylladb_set_entry* entry;
  php_scylladb_type* type;
  int result = 0;

  type = PHP_SCYLLADB_GET_TYPE(&set->type);

  if (!php_scylladb_validate_object(object, &type->data.set.value_type )) {
    return 0;
  }

  HASH_FIND_ZVAL(set->entries, object, entry);
  if (entry != nullptr) {
    set->dirty = 1;
    if (entry == set->iter_temp) {
      set->iter_temp = (php_scylladb_set_entry*) set->iter_temp->hh.next;
    }
    HASH_DEL(set->entries, entry);
    zval_ptr_dtor(&entry->value);
    efree(entry);
    result = 1;
  }

  return result;
}

static int
php_scylladb_set_has(php_scylladb_set* set, zval* object)
{
  php_scylladb_set_entry* entry;
  php_scylladb_type* type;
  int result = 0;

  type = PHP_SCYLLADB_GET_TYPE(&set->type);

  if (!php_scylladb_validate_object(object, &type->data.set.value_type )) {
    return 0;
  }

  HASH_FIND_ZVAL(set->entries, object, entry);
  if (entry != nullptr) {
    result = 1;
  }

  return result;
}

static void
php_scylladb_set_populate(php_scylladb_set* set, zval* array)
{
  php_scylladb_set_entry *curr, *temp;
  HASH_ITER(hh, set->entries, curr, temp)
  {
    if (add_next_index_zval(array, &curr->value) != SUCCESS) {
      break;
    }
    Z_TRY_ADDREF_P(&curr->value);
  }
}

/* {{{ Set::__construct(type) */
PHP_METHOD(Cassandra_Set, __construct)
{
  php_scylladb_set* self;
  zval* type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(type)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SET(getThis());

  if (Z_TYPE_P(type) == IS_STRING) {
    CassValueType value_type;
    if (!php_scylladb_value_type(Z_STRVAL_P(type), &value_type ))
      return;
    self->type = php_scylladb_type_set_from_value_type(value_type );
  } else if (Z_TYPE_P(type) == IS_OBJECT && instanceof_function(Z_OBJCE_P(type), php_scylladb_type_ce )) {
    if (!php_scylladb_type_validate(type, "type" )) {
      return;
    }
    self->type = php_scylladb_type_set(type );
    Z_ADDREF_P(type);
  } else {
    INVALID_ARGUMENT(type, "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Type");
  }
}
/* }}} */

/* {{{ Set::type() */
PHP_METHOD(Cassandra_Set, type)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}
/* }}} */

/* {{{ Set::values() */
PHP_METHOD(Cassandra_Set, values)
{
  php_scylladb_set* set = nullptr;
  array_init(return_value);
  set = PHP_SCYLLADB_GET_SET(getThis());
  php_scylladb_set_populate(set, return_value );
}
/* }}} */

/* {{{ Set::add(value) */
PHP_METHOD(Cassandra_Set, add)
{
  php_scylladb_set* self = nullptr;

  zval* object;
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(object)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SET(getThis());

  if (php_scylladb_set_add(self, object ))
    RETURN_TRUE;

  RETURN_FALSE;
}
/* }}} */

/* {{{ Set::remove(value) */
PHP_METHOD(Cassandra_Set, remove)
{
  php_scylladb_set* self = nullptr;

  zval* object;
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(object)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SET(getThis());

  if (php_scylladb_set_del(self, object ))
    RETURN_TRUE;

  RETURN_FALSE;
}
/* }}} */

/* {{{ Set::has(value) */
PHP_METHOD(Cassandra_Set, has)
{
  php_scylladb_set* self = nullptr;

  zval* object;
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(object)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SET(getThis());

  if (php_scylladb_set_has(self, object ))
    RETURN_TRUE;

  RETURN_FALSE;
}
/* }}} */

/* {{{ Set::count() */
PHP_METHOD(Cassandra_Set, count)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  RETURN_LONG((long) HASH_COUNT(self->entries));
}
/* }}} */

/* {{{ Set::current() */
PHP_METHOD(Cassandra_Set, current)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  if (self->iter_curr != nullptr)
    RETURN_ZVAL(&self->iter_curr->value, 1, 0);
}
/* }}} */

/* {{{ Set::key() */
PHP_METHOD(Cassandra_Set, key)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  RETURN_LONG(self->iter_index);
}
/* }}} */

/* {{{ Set::next() */
PHP_METHOD(Cassandra_Set, next)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  self->iter_curr      = self->iter_temp;
  self->iter_temp      = self->iter_temp != nullptr ? (php_scylladb_set_entry*) self->iter_temp->hh.next : nullptr;
  self->iter_index++;
}
/* }}} */

/* {{{ Set::valid() */
PHP_METHOD(Cassandra_Set, valid)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  RETURN_BOOL(self->iter_curr != nullptr);
}
/* }}} */

/* {{{ Set::rewind() */
PHP_METHOD(Cassandra_Set, rewind)
{
  auto self = PHP_SCYLLADB_GET_SET(getThis());
  self->iter_curr      = self->entries;
  self->iter_temp      = self->entries != nullptr ? (php_scylladb_set_entry*) self->entries->hh.next : nullptr;
  self->iter_index     = 0;
}
/* }}} */



HashTable*
php_scylladb_set_gc(zend_object* object, zval** table, int* n)
{
  auto self = php_scylladb_set_object_fetch(object);
  zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
  zend_get_gc_buffer_add_zval(buffer, &self->type);
  php_scylladb_set_entry *curr, *temp;
  HASH_ITER(hh, self->entries, curr, temp)
  {
    zend_get_gc_buffer_add_zval(buffer, &curr->value);
  }
  zend_get_gc_buffer_use(buffer, table, n);
  return nullptr;
}

HashTable*
php_scylladb_set_properties(zend_object* object)
{
  zval values;

  auto self = php_scylladb_set_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, ZEND_STRL("type"), &self->type);
  Z_ADDREF_P(&self->type);


  array_init(&values);
  php_scylladb_set_populate(self, &values );
  zend_hash_sort(Z_ARRVAL_P(&values), php_scylladb_data_compare, 1);
  (void)zend_hash_str_update(props, ZEND_STRL("values"), &values);

  return props;
}

int
php_scylladb_set_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_scylladb_set_entry *curr, *temp;
  php_scylladb_set* set1;
  php_scylladb_set* set2;
  php_scylladb_type* type1;
  php_scylladb_type* type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  set1 = PHP_SCYLLADB_GET_SET(obj1);
  set2 = PHP_SCYLLADB_GET_SET(obj2);

  type1 = PHP_SCYLLADB_GET_TYPE(&set1->type);
  type2 = PHP_SCYLLADB_GET_TYPE(&set2->type);

  result = php_scylladb_type_compare(type1, type2 );
  if (result != 0)
    return result;

  if (HASH_COUNT(set1->entries) != HASH_COUNT(set2->entries)) {
    return HASH_COUNT(set1->entries) < HASH_COUNT(set2->entries) ? -1 : 1;
  }

  HASH_ITER(hh, set1->entries, curr, temp)
  {
    php_scylladb_set_entry* entry;
    HASH_FIND_ZVAL(set2->entries, &curr->value, entry);
    if (entry == nullptr) {
      return 1;
    }
  }

  return 0;
}

unsigned
php_scylladb_set_hash_value(zval* obj)
{
  unsigned hashv = 0;
  php_scylladb_set_entry *curr, *temp;
  auto self = PHP_SCYLLADB_GET_SET(obj);

  if (!self->dirty)
    return self->hashv;

  HASH_ITER(hh, self->entries, curr, temp)
  {
    hashv = php_scylladb_combine_hash(hashv, php_scylladb_value_hash(&curr->value ));
  }

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

void
php_scylladb_set_free(zend_object* object)
{
  auto self = php_scylladb_set_object_fetch(object);
  php_scylladb_set_entry *curr, *temp;

  HASH_ITER(hh, self->entries, curr, temp)
  {
    zval_ptr_dtor(&curr->value);
    HASH_DEL(self->entries, curr);
    efree(curr);
  }

  zval_ptr_dtor(&self->type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_set_new(zend_class_entry* ce)
{
  php_scylladb_set* self =
    PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_set, ce, &php_scylladb_set_handlers);

  self->entries = self->iter_curr = self->iter_temp = nullptr;
  self->iter_index                                  = 0;
  self->dirty                                       = 1;
  ZVAL_UNDEF(&self->type);

  php_scylladb_set_handlers.std.offset = XtOffsetOf(php_scylladb_set, zendObject);
  php_scylladb_set_handlers.std.free_obj = php_scylladb_set_free;
  return &self->zendObject;
}

void php_scylladb_set_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_set_handlers.std.offset = XtOffsetOf(php_scylladb_set, zendObject);
}
