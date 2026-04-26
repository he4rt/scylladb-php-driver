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

#include "php_driver.h"
#include "php_driver_types.h"
BEGIN_EXTERN_C()
#include "PreparedStatement_arginfo.h"

zend_class_entry *php_driver_prepared_statement_ce = NULL;

ZEND_METHOD(Cassandra_PreparedStatement, __construct)
{
}

static zend_object_handlers php_driver_prepared_statement_handlers;

static HashTable *
php_driver_prepared_statement_properties(
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

static int
php_driver_prepared_statement_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj2);
}

static void
php_driver_prepared_statement_free(zend_object *object )
{
  php_driver_statement *self = PHP5TO7_ZEND_OBJECT_GET(statement, object);

  if (self->data.prepared.prepared)
    cass_prepared_free(self->data.prepared.prepared);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_prepared_statement_new(zend_class_entry *ce )
{
  php_driver_statement *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(statement, ce);

  self->type = PHP_DRIVER_PREPARED_STATEMENT;
  self->data.prepared.prepared = NULL;

  PHP5TO7_ZEND_OBJECT_INIT_EX(statement, prepared_statement, self, ce);
}

END_EXTERN_C()

void php_driver_define_PreparedStatement()
{
  php_driver_prepared_statement_ce = register_class_Cassandra_PreparedStatement(php_driver_statement_ce);
  php_driver_prepared_statement_ce->create_object = php_driver_prepared_statement_new;

  memcpy(&php_driver_prepared_statement_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_prepared_statement_handlers.get_properties  = php_driver_prepared_statement_properties;
  php_driver_prepared_statement_handlers.compare = php_driver_prepared_statement_compare;
  php_driver_prepared_statement_handlers.clone_obj = NULL;
}