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

#include "DefaultSchema_arginfo.h"

extern zend_object_handlers php_scylladb_default_schema_handlers;
ZEND_METHOD(Cassandra_DefaultSchema, keyspace)
{
  zend_string *name = nullptr;
  php_scylladb_schema *self;
  php_scylladb_keyspace *keyspace;
  const CassKeyspaceMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SCHEMA(ZEND_THIS);
  meta = cass_schema_meta_keyspace_by_name_n(self->schema_meta, ZSTR_VAL(name), ZSTR_LEN(name));
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  object_init_ex(return_value, php_scylladb_default_keyspace_ce);
  keyspace = PHP_SCYLLADB_GET_KEYSPACE(return_value);
  ZVAL_COPY(&keyspace->schema, ZEND_THIS);
  keyspace->meta   = meta;
}

ZEND_METHOD(Cassandra_DefaultSchema, keyspaces)
{
  php_scylladb_schema *self;
  CassIterator     *iterator;

  ZEND_PARSE_PARAMETERS_NONE();

  self     = PHP_SCYLLADB_GET_SCHEMA(ZEND_THIS);
  iterator = cass_iterator_keyspaces_from_schema_meta(self->schema_meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassKeyspaceMeta  *meta;
    const CassValue         *value;
    const char              *keyspace_name;
    size_t                   keyspace_name_len;
    zval             zkeyspace;
    php_scylladb_keyspace      *keyspace;

    meta = cass_iterator_get_keyspace_meta(iterator);
    value = cass_keyspace_meta_field_by_name(meta, "keyspace_name");

    ASSERT_SUCCESS_BLOCK(cass_value_get_string(value, &keyspace_name, &keyspace_name_len),
      zval_ptr_dtor(return_value);
      return;
    );

    object_init_ex(&zkeyspace, php_scylladb_default_keyspace_ce);
    keyspace = PHP_SCYLLADB_GET_KEYSPACE(&zkeyspace);
    ZVAL_COPY(&keyspace->schema, ZEND_THIS);
    keyspace->meta   = meta;
    add_assoc_zval_ex(return_value, keyspace_name, keyspace_name_len, &zkeyspace);
  }

  cass_iterator_free(iterator);
}

ZEND_METHOD(Cassandra_DefaultSchema, version)
{
  php_scylladb_schema *self;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_SCHEMA(ZEND_THIS);
  RETURN_LONG(cass_schema_meta_snapshot_version(self->schema_meta));
}

HashTable *
php_scylladb_default_schema_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

int
php_scylladb_default_schema_compare(zval *obj1, zval *obj2 )
{
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_default_schema_free(zend_object *object )
{
  auto self = php_scylladb_schema_object_fetch(object);

  if (self->schema_meta) {
    cass_schema_meta_free(self->schema_meta);
    self->schema_meta = nullptr;
  }

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_default_schema_new(zend_class_entry *ce )
{
  php_scylladb_schema *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_schema, ce, &php_scylladb_default_schema_handlers);

  self->schema_meta = nullptr;

  return &self->zendObject;
}
