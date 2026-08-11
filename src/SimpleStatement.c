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

#include "SimpleStatement_arginfo.h"

extern zend_object_handlers php_scylladb_simple_statement_handlers;

ZEND_METHOD(Cassandra_SimpleStatement, __construct)
{
  zend_string *cql = nullptr;
  php_scylladb_statement *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(cql)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_STATEMENT(getThis());

  self->data.simple.cql = estrndup(ZSTR_VAL(cql), ZSTR_LEN(cql));
}

PHP_SCYLLADB_DEFINE_IDEMPOTENCE_METHODS(Cassandra_SimpleStatement)

HashTable *
php_scylladb_simple_statement_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

int
php_scylladb_simple_statement_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_simple_statement_free(zend_object *object)
{
  auto self = php_scylladb_statement_object_fetch(object);

  if (self->data.simple.cql) {
    efree(self->data.simple.cql);
    self->data.simple.cql = nullptr;
  }

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_simple_statement_new(zend_class_entry *ce)
{
  php_scylladb_statement *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_statement, ce, &php_scylladb_simple_statement_handlers);

  self->type = PHP_SCYLLADB_SIMPLE_STATEMENT;
  self->data.simple.cql  = nullptr;
  return &self->zendObject;
}
