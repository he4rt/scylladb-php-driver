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
#include "BatchStatement_arginfo.h"

extern zend_object_handlers php_scylladb_batch_statement_handlers;

void php_scylladb_batch_statement_entry_dtor(zval* dest)
{
    auto *batch_statement_entry = static_cast<php_scylladb_batch_statement_entry *>(Z_PTR_P(dest));

    zval_ptr_dtor(&batch_statement_entry->statement);
    zval_ptr_dtor(&batch_statement_entry->arguments);

    efree(batch_statement_entry);
}

ZEND_METHOD(Cassandra_BatchStatement, __construct)
{
    zend_long type = CASS_BATCH_TYPE_LOGGED;
    php_scylladb_statement *self = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(type)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_SCYLLADB_GET_STATEMENT(getThis());

    switch (type)
    {
    case CASS_BATCH_TYPE_LOGGED:
    case CASS_BATCH_TYPE_UNLOGGED:
    case CASS_BATCH_TYPE_COUNTER:
        self->data.batch.type = (CassBatchType)type;
        break;
    default:
        zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
            "type must be one of " PHP_SCYLLADB_NAMESPACE "::BATCH_LOGGED, "
            PHP_SCYLLADB_NAMESPACE "::BATCH_UNLOGGED or " PHP_SCYLLADB_NAMESPACE "::BATCH_COUNTER");
        return;
    }
}

ZEND_METHOD(Cassandra_BatchStatement, add)
{
    zval *statement = NULL;
    zval *arguments = NULL;
    php_scylladb_batch_statement_entry *batch_statement_entry = NULL;
    php_scylladb_statement *self = NULL;
    zval entry;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(statement)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_EX(arguments, 1, 0)
    ZEND_PARSE_PARAMETERS_END();

    if (Z_TYPE_P(statement) != IS_STRING &&
        (Z_TYPE_P(statement) != IS_OBJECT ||
         (!instanceof_function(Z_OBJCE_P(statement), php_scylladb_simple_statement_ce) &&
          !instanceof_function(Z_OBJCE_P(statement), php_scylladb_prepared_statement_ce))))
    {
        INVALID_ARGUMENT(statement, "a string, an instance of " PHP_SCYLLADB_NAMESPACE
                                    "\\SimpleStatement or an instance of " PHP_SCYLLADB_NAMESPACE "\\PreparedStatement");
    }

    self = PHP_SCYLLADB_GET_STATEMENT(getThis());

    batch_statement_entry = (php_scylladb_batch_statement_entry *)ecalloc(1, sizeof(php_scylladb_batch_statement_entry));

    ZVAL_COPY(&batch_statement_entry->statement, statement);

    if (arguments)
    {
        ZVAL_COPY(&batch_statement_entry->arguments, arguments);
    }

    ZVAL_PTR(&entry, batch_statement_entry);
    zend_hash_next_index_insert(&self->data.batch.statements, &entry);

    RETURN_ZVAL(getThis(), 1, 0);
}

HashTable *php_scylladb_batch_statement_properties(zend_object *object)
{
    HashTable *props = zend_std_get_properties(object);

    return props;
}

int php_scylladb_batch_statement_compare(zval *obj1, zval *obj2)
{
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_batch_statement_free(zend_object *object)
{
    php_scylladb_statement *self = php_scylladb_statement_object_fetch(object);

    zend_hash_destroy(&self->data.batch.statements);

    zend_object_std_dtor(&self->zendObject);
}

zend_object* php_scylladb_batch_statement_new(zend_class_entry *ce)
{
    php_scylladb_statement *self = (php_scylladb_statement *)ecalloc(1, sizeof(php_scylladb_statement) + zend_object_properties_size(ce));

    self->type = PHP_SCYLLADB_BATCH_STATEMENT;
    self->data.batch.type = CASS_BATCH_TYPE_LOGGED;
    zend_hash_init(&self->data.batch.statements, 0, NULL, (dtor_func_t)php_scylladb_batch_statement_entry_dtor, 0);

    zend_object_std_init(&self->zendObject, ce);
    self->zendObject.handlers = &php_scylladb_batch_statement_handlers;
    return &self->zendObject;
}

END_EXTERN_C()
