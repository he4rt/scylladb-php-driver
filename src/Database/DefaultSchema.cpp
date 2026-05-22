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

BEGIN_EXTERN_C()
#include "DefaultSchema_arginfo.h"

extern zend_object_handlers php_scylladb_default_schema_handlers;
ZEND_METHOD(Cassandra_DefaultSchema, keyspace)
{
  char *name;
  size_t name_len;
  php_scylladb_schema *self;
  php_scylladb_keyspace *keyspace;
  const CassKeyspaceMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SCHEMA(getThis());
  meta = cass_schema_meta_keyspace_by_name_n(self->schema_meta, name, name_len);
  if (meta == NULL) {
    RETURN_FALSE;
  }

  object_init_ex(return_value, php_scylladb_default_keyspace_ce);
  keyspace = PHP_SCYLLADB_GET_KEYSPACE(return_value);
  ZVAL_COPY(&keyspace->schema, getThis());
  keyspace->meta   = meta;
}

ZEND_METHOD(Cassandra_DefaultSchema, keyspaces)
{
  php_scylladb_schema *self;
  CassIterator     *iterator;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self     = PHP_SCYLLADB_GET_SCHEMA(getThis());
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
    ZVAL_COPY(&keyspace->schema, getThis());
    keyspace->meta   = meta;
    add_assoc_zval_ex(return_value, keyspace_name, keyspace_name_len, &zkeyspace);
  }

  cass_iterator_free(iterator);
}

ZEND_METHOD(Cassandra_DefaultSchema, version)
{
  php_scylladb_schema *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_SCHEMA(getThis());
  RETURN_LONG(cass_schema_meta_snapshot_version(self->schema_meta));
}

HashTable *
php_scylladb_default_schema_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

int
php_scylladb_default_schema_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_default_schema_free(zend_object *object )
{
  php_scylladb_schema *self = php_scylladb_schema_object_fetch(object);

  if (self->schema_meta) {
    cass_schema_meta_free(self->schema_meta);
    self->schema_meta = NULL;
  }

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_default_schema_new(zend_class_entry *ce )
{
  php_scylladb_schema *self =
      (php_scylladb_schema *)ecalloc(1, sizeof(php_scylladb_schema) + zend_object_properties_size(ce));

  self->schema_meta = NULL;

  zend_object_std_init(&self->zendObject, ce);
  php_scylladb_default_schema_handlers.offset = XtOffsetOf(php_scylladb_schema, zendObject);
  php_scylladb_default_schema_handlers.free_obj = php_scylladb_default_schema_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_scylladb_default_schema_handlers;
  return &self->zendObject;
}

END_EXTERN_C()

