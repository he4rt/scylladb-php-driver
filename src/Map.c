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

#include "Map.h"

#include "php_scylladb.h"
#include "php_scylladb_types.h"
#include "Type/Conversions.h"
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"

#include "Map_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_map_handlers;

int
php_scylladb_map_set(php_scylladb_map *map, zval *zkey, zval *zvalue)
{
  php_scylladb_map_entry *entry;
  php_scylladb_type *type;

  if (Z_TYPE_P(zkey) == IS_NULL) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                            "Invalid key: null is not supported inside maps");
    return 0;
  }

  if (Z_TYPE_P(zvalue) == IS_NULL) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
                            "Invalid value: null is not supported inside maps");
    return 0;
  }

  type = PHP_SCYLLADB_GET_TYPE(&map->type);

  if (!php_scylladb_validate_object(zkey, &type->data.map.key_type )) {
    return 0;
  }

  if (!php_scylladb_validate_object(zvalue, &type->data.map.value_type )) {
    return 0;
  }

  map->dirty = 1;
  HASH_FIND_ZVAL(map->entries, zkey, entry);
  if (entry == nullptr) {
    entry = (php_scylladb_map_entry *) emalloc(sizeof(php_scylladb_map_entry));
    ZVAL_COPY(&entry->key, zkey);
    ZVAL_COPY(&entry->value, zvalue);
    HASH_ADD_ZVAL(map->entries, key, entry);
  } else {
    zval prev_value = entry->value;
    ZVAL_COPY(&entry->value, zvalue);
    zval_ptr_dtor(&prev_value);
  }

  return 1;
}

static int
php_scylladb_map_get(php_scylladb_map *map, zval *zkey, zval *zvalue)
{
  php_scylladb_map_entry *entry;
  php_scylladb_type *type;
  int result = 0;

  type = PHP_SCYLLADB_GET_TYPE(&map->type);

  if (!php_scylladb_validate_object(zkey, &type->data.map.key_type )) {
    return 0;
  }

  HASH_FIND_ZVAL(map->entries, zkey, entry);
  if (entry != nullptr) {
    *zvalue = entry->value;
    result = 1;
  }

  return result;
}

static int
php_scylladb_map_del(php_scylladb_map *map, zval *zkey)
{
  php_scylladb_map_entry *entry;
  php_scylladb_type *type;
  int result = 0;

  type = PHP_SCYLLADB_GET_TYPE(&map->type);

  if (!php_scylladb_validate_object(zkey, &type->data.map.key_type )) {
    return 0;
  }

  HASH_FIND_ZVAL(map->entries, zkey, entry);
  if (entry != nullptr) {
    map->dirty = 1;
    if (entry == map->iter_temp) {
      map->iter_temp = (php_scylladb_map_entry *)map->iter_temp->hh.next;
    }
    HASH_DEL(map->entries, entry);
    zval_ptr_dtor(&entry->key);
    zval_ptr_dtor(&entry->value);
    efree(entry);
    result = 1;
  }

  return result;
}

static int
php_scylladb_map_has(php_scylladb_map *map, zval *zkey)
{
  php_scylladb_map_entry *entry;
  php_scylladb_type *type;
  int result = 0;

  type = PHP_SCYLLADB_GET_TYPE(&map->type);

  if (!php_scylladb_validate_object(zkey, &type->data.map.key_type )) {
    return 0;
  }

  HASH_FIND_ZVAL(map->entries, zkey, entry);
  if (entry != nullptr) {
    result = 1;
  }

  return result;
}

static void
php_scylladb_map_populate_keys(const php_scylladb_map *map, zval *array)
{
  php_scylladb_map_entry *curr,  *temp;
  HASH_ITER(hh, map->entries, curr, temp) {
    if (add_next_index_zval(array, &curr->key) != SUCCESS) {
      break;
    }
    Z_TRY_ADDREF_P(&curr->key);
  }
}

static void
php_scylladb_map_populate_values(const php_scylladb_map *map, zval *array)
{
  php_scylladb_map_entry *curr, *temp;
  HASH_ITER(hh, map->entries, curr, temp) {
    if (add_next_index_zval(array, &curr->value) != SUCCESS) {
      break;
    }
    Z_TRY_ADDREF_P(&curr->value);
  }
}

/* {{{ Map::__construct(type, type) */
PHP_METHOD(Cassandra_Map, __construct)
{
  php_scylladb_map *self;
  zval *key_type;
  zval *value_type;
  zval scalar_key_type;
  zval scalar_value_type;

  ZVAL_UNDEF(&scalar_key_type);
  ZVAL_UNDEF(&scalar_value_type);

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ZVAL(key_type)
    Z_PARAM_ZVAL(value_type)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (Z_TYPE_P(key_type) == IS_STRING) {
    CassValueType type;
    if (!php_scylladb_value_type(Z_STRVAL_P(key_type), &type ))
      return;
    scalar_key_type = php_scylladb_type_scalar(type );
    key_type = &scalar_key_type;
  } else if (Z_TYPE_P(key_type) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(key_type), php_scylladb_type_ce )) {
    if (!php_scylladb_type_validate(key_type, "keyType" )) {
      return;
    }
    Z_ADDREF_P(key_type);
  } else {
    throw_invalid_argument(key_type,
                           "keyType",
                           "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Type" );
    return;
  }

  if (Z_TYPE_P(value_type) == IS_STRING) {
    CassValueType type;
    if (!php_scylladb_value_type(Z_STRVAL_P(value_type), &type ))
      return;
    scalar_value_type = php_scylladb_type_scalar(type );
    value_type = &scalar_value_type;
  } else if (Z_TYPE_P(value_type) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(value_type), php_scylladb_type_ce )) {
    if (!php_scylladb_type_validate(value_type, "valueType" )) {
      return;
    }
    Z_ADDREF_P(value_type);
  } else {
    zval_ptr_dtor(key_type);
    throw_invalid_argument(value_type,
                           "valueType",
                           "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Type" );
    return;
  }

  self->type = php_scylladb_type_map(key_type, value_type );
}
/* }}} */

/* {{{ Map::type() */
PHP_METHOD(Cassandra_Map, type)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}
/* }}} */

PHP_METHOD(Cassandra_Map, keys)
{
  php_scylladb_map *self = nullptr;
  array_init(return_value);
  self = PHP_SCYLLADB_GET_MAP(getThis());
  php_scylladb_map_populate_keys(self, return_value );
}

PHP_METHOD(Cassandra_Map, values)
{
  php_scylladb_map *self = nullptr;
  array_init(return_value);
  self = PHP_SCYLLADB_GET_MAP(getThis());
  php_scylladb_map_populate_values(self, return_value );
}

PHP_METHOD(Cassandra_Map, set)
{
  zval *key;
  php_scylladb_map *self = nullptr;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ZVAL(key)
    Z_PARAM_ZVAL(value)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (php_scylladb_map_set(self, key, value ))
    RETURN_TRUE;

  RETURN_FALSE;
}

PHP_METHOD(Cassandra_Map, get)
{
  zval *key;
  php_scylladb_map *self = nullptr;
  zval value;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (php_scylladb_map_get(self, key, &value ))
    RETURN_ZVAL(&value, 1, 0);
}

PHP_METHOD(Cassandra_Map, remove)
{
  zval *key;
  php_scylladb_map *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (php_scylladb_map_del(self, key ))
    RETURN_TRUE;

  RETURN_FALSE;
}

PHP_METHOD(Cassandra_Map, has)
{
  zval *key;
  php_scylladb_map *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (php_scylladb_map_has(self, key ))
    RETURN_TRUE;

  RETURN_FALSE;
}

PHP_METHOD(Cassandra_Map, count)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  RETURN_LONG((long)HASH_COUNT(self->entries));
}

PHP_METHOD(Cassandra_Map, current)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  if (self->iter_curr != nullptr)
    RETURN_ZVAL(&self->iter_curr->value, 1, 0);
}

PHP_METHOD(Cassandra_Map, key)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  if (self->iter_curr != nullptr)
    RETURN_ZVAL(&self->iter_curr->key, 1, 0);
}

PHP_METHOD(Cassandra_Map, next)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  self->iter_curr = self->iter_temp;
  self->iter_temp = self->iter_temp != nullptr ? (php_scylladb_map_entry *)self->iter_temp->hh.next : nullptr;
}

PHP_METHOD(Cassandra_Map, valid)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  RETURN_BOOL(self->iter_curr != nullptr);
}

PHP_METHOD(Cassandra_Map, rewind)
{
  auto self = PHP_SCYLLADB_GET_MAP(getThis());
  self->iter_curr = self->entries;
  self->iter_temp = self->entries != nullptr ? (php_scylladb_map_entry *)self->entries->hh.next : nullptr;
}

PHP_METHOD(Cassandra_Map, offsetSet)
{
  zval *key;
  php_scylladb_map *self = nullptr;
  zval *value;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ZVAL(key)
    Z_PARAM_ZVAL(value)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  php_scylladb_map_set(self, key, value );
}

PHP_METHOD(Cassandra_Map, offsetGet)
{
  zval *key;
  php_scylladb_map *self = nullptr;
  zval value;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (php_scylladb_map_get(self, key, &value ))
    RETURN_ZVAL(&value, 1, 0);
}

PHP_METHOD(Cassandra_Map, offsetUnset)
{
  zval *key;
  php_scylladb_map *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  php_scylladb_map_del(self, key );
}

PHP_METHOD(Cassandra_Map, offsetExists)
{
  zval *key;
  php_scylladb_map *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(key)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MAP(getThis());

  if (php_scylladb_map_has(self, key ))
    RETURN_TRUE;

  RETURN_FALSE;
}



HashTable *
php_scylladb_map_gc(zend_object *object, zval** table, int *n)
{
  auto self = php_scylladb_map_object_fetch(object);
  zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
  zend_get_gc_buffer_add_zval(buffer, &self->type);
  php_scylladb_map_entry *curr, *temp;
  HASH_ITER(hh, self->entries, curr, temp)
  {
    zend_get_gc_buffer_add_zval(buffer, &curr->key);
    zend_get_gc_buffer_add_zval(buffer, &curr->value);
  }
  zend_get_gc_buffer_use(buffer, table, n);
  return nullptr;
}

HashTable *
php_scylladb_map_properties(zend_object *object)
{
  zval keys;
  zval values;

  auto self = php_scylladb_map_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(3);
  HashTable *props = object->properties;

  (void)zend_hash_str_update(props, ZEND_STRL("type"), &self->type);
  Z_ADDREF_P(&self->type);


  array_init(&keys);
  php_scylladb_map_populate_keys(self, &keys );
  zend_hash_sort(Z_ARRVAL_P(&keys), php_scylladb_data_compare, 1);
  (void)zend_hash_str_update(props, ZEND_STRL("keys"), &keys);


  array_init(&values);
  php_scylladb_map_populate_values(self, &values );
  zend_hash_sort(Z_ARRVAL_P(&values), php_scylladb_data_compare, 1);
  (void)zend_hash_str_update(props, ZEND_STRL("values"), &values);

  return props;
}

int
php_scylladb_map_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_scylladb_map_entry *curr, *temp;
  php_scylladb_map *map1;
  php_scylladb_map *map2;
  php_scylladb_type *type1;
  php_scylladb_type *type2;
  int result;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  map1 = PHP_SCYLLADB_GET_MAP(obj1);
  map2 = PHP_SCYLLADB_GET_MAP(obj2);

  type1 = PHP_SCYLLADB_GET_TYPE(&map1->type);
  type2 = PHP_SCYLLADB_GET_TYPE(&map2->type);

  result = php_scylladb_type_compare(type1, type2 );
  if (result != 0) return result;

  if (HASH_COUNT(map1->entries) != HASH_COUNT(map2->entries)) {
   return HASH_COUNT(map1->entries) < HASH_COUNT(map2->entries) ? -1 : 1;
  }

  HASH_ITER(hh, map1->entries, curr, temp) {
    php_scylladb_map_entry *entry = nullptr;
    HASH_FIND_ZVAL(map2->entries, &curr->key, entry);
    if (entry == nullptr) {
      return 1;
    }
    result = php_scylladb_value_compare(&curr->value,
                                      &entry->value );
    if (result != 0) return result;
  }

  return 0;
}

unsigned
php_scylladb_map_hash_value(zval *obj)
{
  auto self = PHP_SCYLLADB_GET_MAP(obj);
  php_scylladb_map_entry *curr, *temp;
  unsigned hashv = 0;

  if (!self->dirty) return self->hashv;

  HASH_ITER(hh, self->entries, curr, temp) {
    hashv = php_scylladb_combine_hash(hashv,
                                       php_scylladb_value_hash(&curr->key ));
    hashv = php_scylladb_combine_hash(hashv,
                                       php_scylladb_value_hash(&curr->value ));
  }

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

void
php_scylladb_map_free(zend_object *object)
{
  auto self = php_scylladb_map_object_fetch(object);
  php_scylladb_map_entry *curr, *temp;

  HASH_ITER(hh, self->entries, curr, temp) {
    zval_ptr_dtor(&curr->key);
    zval_ptr_dtor(&curr->value);
    HASH_DEL(self->entries, curr);
    efree(curr);
  }

  zval_ptr_dtor(&self->type);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_map_new(zend_class_entry *ce)
{
  php_scylladb_map *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_map, ce, &php_scylladb_map_handlers);

  self->entries = self->iter_curr = self->iter_temp = nullptr;
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  php_scylladb_map_handlers.std.offset = XtOffsetOf(php_scylladb_map, zendObject);
  php_scylladb_map_handlers.std.free_obj = php_scylladb_map_free;
  return &self->zendObject;
}


void php_scylladb_map_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_map_handlers.std.offset = XtOffsetOf(php_scylladb_map, zendObject);
}
