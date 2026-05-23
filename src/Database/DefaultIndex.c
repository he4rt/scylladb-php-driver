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
#include "Database/ResultDecoder.h"
#include "Type/TypeFactory.h"

#include "DefaultIndex.h"

#include "DefaultIndex_arginfo.h"

extern zend_object_handlers php_scylladb_default_index_handlers;
zval
php_scylladb_create_index(zval *schema,
                           const CassIndexMeta *meta )
{
  zval result;
  php_scylladb_index *index;
  const char *name;
  size_t name_length;

  ZVAL_UNDEF(&result);

  object_init_ex(&result, php_scylladb_default_index_ce);

  index = PHP_SCYLLADB_GET_INDEX(&result);
  index->meta   = meta;
  ZVAL_COPY(&index->schema, schema);

  cass_index_meta_name(meta, &name, &name_length);

  ZVAL_STRINGL(&index->name, name, name_length);

  return result;
}

ZEND_METHOD(Cassandra_DefaultIndex, name)
{
  php_scylladb_index *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  RETURN_ZVAL(&self->name, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultIndex, target)
{
  php_scylladb_index *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  if (Z_ISUNDEF(self->target)) {
    const char *target;
    size_t target_length;
    cass_index_meta_target(self->meta, &target, &target_length);

    ZVAL_STRINGL(&self->target, target, target_length);
  }

  RETURN_ZVAL(&self->target, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultIndex, kind)
{
  php_scylladb_index *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  if (Z_ISUNDEF(self->kind)) {

    switch (cass_index_meta_type(self->meta)) {
      case CASS_INDEX_TYPE_KEYS:
        ZVAL_STRING(&self->kind, "keys");
        break;
      case CASS_INDEX_TYPE_CUSTOM:
        ZVAL_STRING(&self->kind, "custom");
        break;
      case CASS_INDEX_TYPE_COMPOSITES:
        ZVAL_STRING(&self->kind, "composites");
        break;
      default:
        ZVAL_STRING(&self->kind, "unknown");
        break;
    }
  }

  RETURN_ZVAL(&self->kind, 1, 0);
}

void php_scylladb_index_build_option(php_scylladb_index *index)
{
  const CassValue* options;

  array_init(&index->options);
  options = cass_index_meta_options(index->meta);
  if (options) {
    CassIterator* iterator = cass_iterator_from_map(options);
    while (cass_iterator_next(iterator)) {
      const char* key_str;
      size_t key_str_length;
      const char* value_str;
      size_t value_str_length;
      const CassValue* key = cass_iterator_get_map_key(iterator);
      const CassValue* value = cass_iterator_get_map_value(iterator);

      if (cass_value_get_string(key, &key_str, &key_str_length) != CASS_OK) {
        continue;
      }
      if (cass_value_get_string(value, &value_str, &value_str_length) != CASS_OK) {
        continue;
      }
      add_assoc_stringl_ex(&index->options, key_str, key_str_length, (char *)(value_str), (size_t)(value_str_length));
    }
  }
}

ZEND_METHOD(Cassandra_DefaultIndex, option)
{
  char *name;
  size_t name_len;
  php_scylladb_index *self;
  zval* result;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_index_build_option(self);
  }

  if ((result = zend_hash_str_find(Z_ARRVAL(self->options), name, name_len)) != nullptr) {
    RETURN_ZVAL(result, 1, 0);
  }
  RETURN_FALSE;
}

ZEND_METHOD(Cassandra_DefaultIndex, options)
{
  php_scylladb_index *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_index_build_option(self);
  }

  RETURN_ZVAL(&self->options, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultIndex, className)
{
  php_scylladb_index *self;
  zval* result;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_index_build_option(self);
  }

  if ((result = zend_hash_str_find(Z_ARRVAL(self->options), ZEND_STRL("class_name"))) != nullptr) {
    RETURN_ZVAL(result, 1, 0);
  }
  RETURN_FALSE;
}

ZEND_METHOD(Cassandra_DefaultIndex, isCustom)
{
  php_scylladb_index *self;
  int is_custom;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_INDEX(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_index_build_option(self);
  }

  is_custom = zend_hash_str_exists(Z_ARRVAL(self->options), ZEND_STRL("class_name"));
  RETURN_BOOL(is_custom);
}

HashTable *
php_scylladb_default_index_gc(zend_object *object, zval** table, int *n)
{
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *
php_scylladb_default_index_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

int
php_scylladb_default_index_compare(zval *obj1, zval *obj2 )
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_default_index_free(zend_object *object )
{
  auto self = php_scylladb_index_object_fetch(object);

  zval_ptr_dtor(&self->name);
  zval_ptr_dtor(&self->kind);
  zval_ptr_dtor(&self->target);
  zval_ptr_dtor(&self->options);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = nullptr;

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_default_index_new(zend_class_entry *ce )
{
  php_scylladb_index *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_index, ce, &php_scylladb_default_index_handlers);

  ZVAL_UNDEF(&self->name);
  ZVAL_UNDEF(&self->kind);
  ZVAL_UNDEF(&self->target);
  ZVAL_UNDEF(&self->options);

  ZVAL_UNDEF(&self->schema);
  self->meta = nullptr;

  php_scylladb_default_index_handlers.offset = XtOffsetOf(php_scylladb_index, zendObject);
  php_scylladb_default_index_handlers.free_obj = php_scylladb_default_index_free;
  return &self->zendObject;
}
