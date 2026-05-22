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
#include "PreparedStatement_arginfo.h"

extern zend_object_handlers php_scylladb_prepared_statement_handlers;

ZEND_METHOD(Cassandra_PreparedStatement, __construct)
{
}

HashTable *
php_scylladb_prepared_statement_properties(
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
php_scylladb_prepared_statement_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_prepared_statement_free(zend_object *object )
{
  php_scylladb_statement *self = php_scylladb_statement_object_fetch(object);

  if (self->data.prepared.prepared)
    cass_prepared_free(self->data.prepared.prepared);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_prepared_statement_new(zend_class_entry *ce )
{
  php_scylladb_statement *self =
      (php_scylladb_statement *)ecalloc(1, sizeof(php_scylladb_statement) + zend_object_properties_size(ce));

  self->type = PHP_SCYLLADB_PREPARED_STATEMENT;
  self->data.prepared.prepared = NULL;

  zend_object_std_init(&self->zendObject, ce);
  self->zendObject.handlers = &php_scylladb_prepared_statement_handlers;
  return &self->zendObject;
}

END_EXTERN_C()