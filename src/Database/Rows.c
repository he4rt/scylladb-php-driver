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
#include "src/FutureRows.h"
#include "../FutureUtil.h"
#include "Database/ResultDecoder.h"

#include "Rows_arginfo.h"

extern zend_object_handlers php_scylladb_rows_handlers;
static void php_scylladb_rows_create(php_scylladb_rows *current, zval *result )
{
    php_scylladb_rows *rows;

    if (Z_ISUNDEF(current->next_rows))
    {
        if (php_scylladb_get_result(current->next_result, &current->next_rows ) ==
            FAILURE)
        {
            zval_ptr_dtor(&current->next_rows);
            return;
        }
    }

    object_init_ex(result, php_scylladb_rows_ce);
    rows = PHP_SCYLLADB_GET_ROWS(result);

    ZVAL_COPY(&rows->rows, &current->next_rows);

    if (cass_result_has_more_pages(current->next_result))
    {
        /* New Rows shares statement (refcounted resource) and the session
           zval; it takes ownership of next_result so the original Rows no
           longer holds it. */
        GC_ADDREF(current->statement);
        rows->statement = current->statement;
        ZVAL_COPY(&rows->session, &current->session);
        rows->result        = current->next_result;
        current->next_result = nullptr;
    }
}

ZEND_METHOD(Cassandra_Rows, __construct)
{
    zend_throw_exception_ex(php_scylladb_logic_exception_ce, 0 ,
                            "Instantiation of a " PHP_SCYLLADB_NAMESPACE "\\Rows objects directly is not supported, "
                            "call " PHP_SCYLLADB_NAMESPACE "\\Session::execute() or " PHP_SCYLLADB_NAMESPACE
                            "\\FutureRows::get() instead.");
    return;
}

ZEND_METHOD(Cassandra_Rows, count)
{
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
        return;

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(&self->rows)));
}

ZEND_METHOD(Cassandra_Rows, rewind)
{
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
        return;

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    zend_hash_internal_pointer_reset(Z_ARRVAL(self->rows));
}

ZEND_METHOD(Cassandra_Rows, current)
{
    if (zend_parse_parameters_none() == FAILURE)
    {
        return;
    }

    auto self = PHP_SCYLLADB_GET_ROWS(getThis());

    zval *entry = zend_hash_get_current_data(Z_ARRVAL(self->rows));

    if (entry != nullptr)
    {
        RETURN_ZVAL(entry, 1, 0);
    }
}

ZEND_METHOD(Cassandra_Rows, key)
{
    zend_ulong num_index;
    zend_string* str_index;
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
        return;

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    if (zend_hash_get_current_key(Z_ARRVAL(self->rows), &str_index, &num_index) ==
        HASH_KEY_IS_LONG)
        RETURN_LONG(num_index);
}

ZEND_METHOD(Cassandra_Rows, next)
{
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
    {
        return;
    }

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    zend_hash_move_forward(Z_ARRVAL(self->rows));
}

ZEND_METHOD(Cassandra_Rows, valid)
{
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
        return;

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    RETURN_BOOL(zend_hash_has_more_elements(Z_ARRVAL(self->rows)) == SUCCESS);
}

ZEND_METHOD(Cassandra_Rows, offsetExists)
{
    zval *offset;
    php_scylladb_rows *self = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(offset)
    ZEND_PARSE_PARAMETERS_END();

    if (Z_TYPE_P(offset) != IS_LONG || Z_LVAL_P(offset) < 0)
    {
        INVALID_ARGUMENT(offset, "a positive integer");
    }

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    RETURN_BOOL(zend_hash_index_exists(Z_ARRVAL(self->rows), (zend_ulong)Z_LVAL_P(offset)));
}

ZEND_METHOD(Cassandra_Rows, offsetGet)
{
    zval *offset;
    zval *value;
    php_scylladb_rows *self = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(offset)
    ZEND_PARSE_PARAMETERS_END();

    if (Z_TYPE_P(offset) != IS_LONG || Z_LVAL_P(offset) < 0)
    {
        INVALID_ARGUMENT(offset, "a positive integer");
    }

    self = PHP_SCYLLADB_GET_ROWS(getThis());
    if ((value = zend_hash_index_find(Z_ARRVAL(self->rows), (zend_ulong)(Z_LVAL_P(offset)))) != nullptr)
    {
        RETURN_ZVAL(value, 1, 0);
    }
}

ZEND_METHOD(Cassandra_Rows, offsetSet)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;

    zend_throw_exception_ex(php_scylladb_domain_exception_ce, 0 ,
                            "Cannot overwrite a row at a given offset, rows are immutable.");
    return;
}

ZEND_METHOD(Cassandra_Rows, offsetUnset)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;

    zend_throw_exception_ex(php_scylladb_domain_exception_ce, 0 ,
                            "Cannot delete a row at a given offset, rows are immutable.");
    return;
}

ZEND_METHOD(Cassandra_Rows, isLastPage)
{
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
        return;

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    if (self->result == nullptr && Z_ISUNDEF(self->next_rows) && Z_ISUNDEF(self->future_next_page))
    {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_METHOD(Cassandra_Rows, nextPage)
{
    zval *timeout = nullptr;
    auto self = PHP_SCYLLADB_GET_ROWS(getThis());

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(timeout)
    ZEND_PARSE_PARAMETERS_END();

    if (!self->next_result)
    {
        if (!Z_ISUNDEF(self->future_next_page))
        {
            php_scylladb_future_rows *future_rows = nullptr;

            if (Z_TYPE(self->future_next_page) != IS_OBJECT ||
                !instanceof_function(Z_OBJCE(self->future_next_page),
                                     php_scylladb_future_rows_ce ))
            {
                zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0 , "Unexpected future instance.");
                return;
            }

            future_rows = PHP_SCYLLADB_GET_FUTURE_ROWS(&self->future_next_page);

            if (php_scylladb_future_rows_get_result(future_rows, timeout ) == FAILURE)
            {
                return;
            }

            /* Take ownership of FutureRows' result — the FutureRows is
               about to be discarded along with the future_next_page. */
            self->next_result    = future_rows->result;
            future_rows->result  = nullptr;
        }
        else
        {
            const CassResult *result = nullptr;
            CassFuture *future = nullptr;

            if (self->result == nullptr)
            {
                return;
            }

            ASSERT_SUCCESS(cass_statement_set_paging_state((CassStatement *)self->statement->ptr,
                                                           self->result));

            future = cass_session_execute(
                PHP_SCYLLADB_GET_SESSION(&self->session)->session,
                (CassStatement *)self->statement->ptr);

            if (php_scylladb_future_wait_timed(future, timeout ) == FAILURE)
            {
                cass_future_free(future);
                return;
            }

            if (php_scylladb_future_is_error(future ) == FAILURE)
            {
                cass_future_free(future);
                return;
            }

            result = cass_future_get_result(future);
            if (!result)
            {
                cass_future_free(future);
                zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0 ,
                                        "Future doesn't contain a result.");
                return;
            }

            self->next_result = result;

            cass_future_free(future);
        }
    }

    /* Always create a new rows object to avoid creating a linked list of
     * objects.
     */
    php_scylladb_rows_create(self, return_value );
}

ZEND_METHOD(Cassandra_Rows, nextPageAsync)
{
    php_scylladb_rows *self = nullptr;
    php_scylladb_future_rows *future_rows = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
        return;

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    if (!Z_ISUNDEF(self->future_next_page))
    {
        RETURN_ZVAL(&self->future_next_page, 1, 0);
    }

    if (self->next_result)
    {
        php_scylladb_future_value *future_value;

        object_init_ex(&self->future_next_page, php_scylladb_future_value_ce);
        future_value = PHP_SCYLLADB_GET_FUTURE_VALUE(&self->future_next_page);

        php_scylladb_rows_create(self, &future_value->value );
        RETURN_ZVAL(&self->future_next_page, 1, 0);
    }

    if (self->result == nullptr)
    {
        object_init_ex(return_value, php_scylladb_future_value_ce);
        return;
    }

    ASSERT_SUCCESS(cass_statement_set_paging_state((CassStatement *)self->statement->ptr,
                                                   self->result));

    object_init_ex(&self->future_next_page, php_scylladb_future_rows_ce);
    future_rows = PHP_SCYLLADB_GET_FUTURE_ROWS(&self->future_next_page);

    GC_ADDREF(self->statement);
    future_rows->statement = self->statement;
    ZVAL_COPY(&future_rows->session, &self->session);
    future_rows->future = cass_session_execute(
        PHP_SCYLLADB_GET_SESSION(&self->session)->session,
        (CassStatement *)self->statement->ptr);

    RETURN_ZVAL(&self->future_next_page, 1, 0);
}

ZEND_METHOD(Cassandra_Rows, pagingStateToken)
{
    const char *paging_state;
    size_t paging_state_size;
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
    {
        return;
    }

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    if (self->result == nullptr)
        return;

    ASSERT_SUCCESS(
        cass_result_paging_state_token(self->result, &paging_state, &paging_state_size));
    RETVAL_STRINGL(paging_state, paging_state_size);
}

ZEND_METHOD(Cassandra_Rows, first)
{
    HashPosition pos;
    zval *entry;
    php_scylladb_rows *self = nullptr;

    if (zend_parse_parameters_none() == FAILURE)
    {
        return;
    }

    self = PHP_SCYLLADB_GET_ROWS(getThis());

    zend_hash_internal_pointer_reset_ex(Z_ARRVAL(self->rows), &pos);
    if ((entry = zend_hash_get_current_data(Z_ARRVAL(self->rows))) != nullptr)
    {
        RETVAL_ZVAL(entry, 1, 0);
    }
}

HashTable *php_scylladb_rows_properties(zend_object *object)
{
    HashTable *props = zend_std_get_properties(object);

    return props;
}

int php_scylladb_rows_compare(zval *obj1, zval *obj2)
{
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_rows_free(zend_object *object)
{
    auto self = php_scylladb_rows_object_fetch(object);

    if (self->result) {
        cass_result_free((CassResult *)self->result);
        self->result = nullptr;
    }
    if (self->next_result) {
        cass_result_free((CassResult *)self->next_result);
        self->next_result = nullptr;
    }
    if (self->statement) {
        zend_list_delete(self->statement);
        self->statement = nullptr;
    }
    if (!Z_ISUNDEF(self->session)) {
        zval_ptr_dtor(&self->session);
        ZVAL_UNDEF(&self->session);
    }

    zval_ptr_dtor(&self->rows);
    zval_ptr_dtor(&self->next_rows);
    zval_ptr_dtor(&self->future_next_page);

    zend_object_std_dtor(&self->zendObject);
}

zend_object *php_scylladb_rows_new(zend_class_entry *ce)
{
    php_scylladb_rows *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_rows, ce, &php_scylladb_rows_handlers);

    self->statement = nullptr;
    self->result = nullptr;
    self->next_result = nullptr;
    ZVAL_UNDEF(&self->session);
    ZVAL_UNDEF(&self->rows);
    ZVAL_UNDEF(&self->next_rows);
    ZVAL_UNDEF(&self->future_next_page);

  php_scylladb_rows_handlers.offset = XtOffsetOf(php_scylladb_rows, zendObject);
  php_scylladb_rows_handlers.free_obj = php_scylladb_rows_free;
  return &self->zendObject;
}
