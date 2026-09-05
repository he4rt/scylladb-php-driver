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
#include "php_scylladb_consistency.h"
#include "Numbers/NumberParser.h"

#include "ExecutionOptions_arginfo.h"

extern zend_object_handlers php_scylladb_execution_options_handlers;

static void init_execution_options(php_scylladb_execution_options *self)
{
    self->consistency = -1;
    self->serial_consistency = -1;
    self->page_size = -1;
    self->paging_state_token = nullptr;
    self->timestamp = INT64_MIN;
    self->idempotent = PHP_SCYLLADB_TRISTATE_UNSET;
    ZVAL_UNDEF(&self->arguments);
    ZVAL_UNDEF(&self->timeout);
    ZVAL_UNDEF(&self->retry_policy);
}

static zend_result build_from_array(php_scylladb_execution_options *self, zval *options, int copy)
{
    zval *consistency = nullptr;
    zval *serial_consistency = nullptr;
    zval *page_size = nullptr;
    zval *paging_state_token = nullptr;
    zval *timeout = nullptr;
    zval *arguments = nullptr;
    zval *retry_policy = nullptr;
    zval *timestamp = nullptr;
    zval *idempotent = nullptr;

    if ((consistency = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("consistency"))) != nullptr)
    {
        if (Z_TYPE_P(consistency) != IS_LONG)
        {
            throw_invalid_argument(consistency, "consistency", "one of " PHP_SCYLLADB_NAMESPACE "::CONSISTENCY_*");
            return FAILURE;
        }

        zend_long val = Z_LVAL_P(consistency);

        if (php_scylladb_validate_consistency((uint32_t)val) == -1)
        {
            throw_invalid_argument(consistency, "consistency", "one of " PHP_SCYLLADB_NAMESPACE "::CONSISTENCY_*");

            return FAILURE;
        }

        self->consistency = val;
    }

    if ((serial_consistency = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("serial_consistency"))) != nullptr)
    {
        if (Z_TYPE_P(serial_consistency) != IS_LONG)
        {
            throw_invalid_argument(serial_consistency, "serial_consistency",
                                   "either " PHP_SCYLLADB_NAMESPACE
                                   "::CONSISTENCY_SERIAL or Cassandra::CASS_CONSISTENCY_LOCAL_SERIAL");
            return FAILURE;
        }

        zend_long val = Z_LVAL_P(serial_consistency);

        if (php_scylladb_validate_serial_consistency((uint32_t)val) == -1)
        {
            throw_invalid_argument(serial_consistency, "serial_consistency",
                                   "either " PHP_SCYLLADB_NAMESPACE
                                   "::CONSISTENCY_SERIAL or Cassandra::CASS_CONSISTENCY_LOCAL_SERIAL");
            return FAILURE;
        }

        self->serial_consistency = val;
    }

    if ((page_size = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("page_size"))) != nullptr)
    {
        if (Z_TYPE_P(page_size) != IS_LONG ||
            Z_LVAL_P(page_size) <= 0)
        {
            throw_invalid_argument(page_size, "page_size", "greater than zero");
            return FAILURE;
        }
        self->page_size = (int)Z_LVAL_P(page_size);
    }


    if ((paging_state_token = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("paging_state_token"))) != nullptr)
    {
        if (Z_TYPE_P(paging_state_token) != IS_STRING)
        {
            throw_invalid_argument(paging_state_token, "paging_state_token", "a string");
            return FAILURE;
        }
        if (copy)
        {
            self->paging_state_token = zend_string_copy(Z_STR_P(paging_state_token));
        }
        else
        {
            self->paging_state_token = Z_STR_P(paging_state_token);
        }
    }

    if ((timeout = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("timeout"))) != nullptr)
    {
        if (!(Z_TYPE_P(timeout) == IS_LONG &&
              Z_LVAL_P(timeout) > 0) &&
            !(Z_TYPE_P(timeout) == IS_DOUBLE &&
              Z_DVAL_P(timeout) > 0) &&
            !(Z_TYPE_P(timeout) == IS_NULL))
        {
            throw_invalid_argument(timeout, "timeout",
                                   "a number of seconds greater than zero or null");
            return FAILURE;
        }

        if (copy)
        {
            ZVAL_COPY(&self->timeout, timeout);
        }
        else
        {
            self->timeout = *timeout;
        }
    }

    if ((arguments = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("arguments"))) != nullptr)
    {
        if (Z_TYPE_P(arguments) != IS_ARRAY)
        {
            throw_invalid_argument(arguments, "arguments", "an array");
            return FAILURE;
        }

        if (copy)
        {
            ZVAL_COPY(&self->arguments, arguments);
        }
        else
        {
            self->arguments = *arguments;
        }
    }

    if ((retry_policy = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("retry_policy"))) != nullptr)
    {
        if (Z_TYPE_P(retry_policy) != IS_OBJECT ||
            !instanceof_function(Z_OBJCE_P(retry_policy), php_scylladb_retry_policy_ce))
        {
            throw_invalid_argument(retry_policy, "retry_policy",
                                   "an instance of " PHP_SCYLLADB_NAMESPACE "\\RetryPolicy");
            return FAILURE;
        }

        if (copy)
        {
            ZVAL_COPY(&self->retry_policy, retry_policy);
        }
        else
        {
            self->retry_policy = *retry_policy;
        }
    }

    if ((timestamp = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("timestamp"))) != nullptr)
    {
        if (Z_TYPE_P(timestamp) == IS_LONG)
        {
            self->timestamp = Z_LVAL_P(timestamp);
        }
        else if (Z_TYPE_P(timestamp) == IS_STRING)
        {
            if (!php_scylladb_parse_bigint(Z_STR_P(timestamp), &self->timestamp))
            {
                return FAILURE;
            }
        }
        else
        {
            throw_invalid_argument(timestamp, "timestamp", "an integer or integer string");
            return FAILURE;
        }
    }

    if ((idempotent = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("idempotent"))) != nullptr)
    {
        if (Z_TYPE_P(idempotent) != IS_TRUE && Z_TYPE_P(idempotent) != IS_FALSE)
        {
            throw_invalid_argument(idempotent, "idempotent", "a boolean");
            return FAILURE;
        }

        self->idempotent = Z_TYPE_P(idempotent) == IS_TRUE ? PHP_SCYLLADB_TRISTATE_TRUE
                                                           : PHP_SCYLLADB_TRISTATE_FALSE;
    }

    return SUCCESS;
}

zend_result php_scylladb_execution_options_build_local_from_array(php_scylladb_execution_options *self, zval *options)
{
    init_execution_options(self);
    return build_from_array(self, options, 0);
}

ZEND_METHOD(Cassandra_ExecutionOptions, __construct)
{
    zval *options = nullptr;
    php_scylladb_execution_options *self = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_SCYLLADB_GET_EXECUTION_OPTIONS(ZEND_THIS);

    build_from_array(self, options, 1);
}

ZEND_METHOD(Cassandra_ExecutionOptions, __get)
{
    zend_string *name = nullptr;
    php_scylladb_execution_options *self = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_SCYLLADB_GET_EXECUTION_OPTIONS(ZEND_THIS);

    if (zend_string_equals_literal(name, "consistency"))
    {
        if (self->consistency == -1)
        {
            RETURN_NULL();
        }
        RETURN_LONG(self->consistency);
    }
    else if (zend_string_equals_literal(name, "serialConsistency"))
    {
        if (self->serial_consistency == -1)
        {
            RETURN_NULL();
        }
        RETURN_LONG(self->serial_consistency);
    }
    else if (zend_string_equals_literal(name, "pageSize"))
    {
        if (self->page_size == -1)
        {
            RETURN_NULL();
        }
        RETURN_LONG(self->page_size);
    }
    else if (zend_string_equals_literal(name, "pagingStateToken"))
    {
        if (!self->paging_state_token)
        {
            RETURN_NULL();
        }
        RETVAL_STR_COPY(self->paging_state_token);
    }
    else if (zend_string_equals_literal(name, "timeout"))
    {
        if (Z_ISUNDEF(self->timeout))
        {
            RETURN_NULL();
        }
        RETURN_ZVAL(&self->timeout, 1, 0);
    }
    else if (zend_string_equals_literal(name, "arguments"))
    {
        if (Z_ISUNDEF(self->arguments))
        {
            RETURN_NULL();
        }
        RETURN_ZVAL(&self->arguments, 1, 0);
    }
    else if (zend_string_equals_literal(name, "retryPolicy"))
    {
        if (Z_ISUNDEF(self->retry_policy))
        {
            RETURN_NULL();
        }
        RETURN_ZVAL(&self->retry_policy, 1, 0);
    }
    else if (zend_string_equals_literal(name, "timestamp"))
    {
        if (self->timestamp == INT64_MIN)
        {
            RETURN_NULL();
        }
        RETVAL_STR(zend_strpprintf(0, "%" PRId64, (int64_t)self->timestamp));
    }
    else if (zend_string_equals_literal(name, "idempotent"))
    {
        if (self->idempotent == PHP_SCYLLADB_TRISTATE_UNSET)
        {
            RETURN_NULL();
        }
        RETURN_BOOL(self->idempotent == PHP_SCYLLADB_TRISTATE_TRUE);
    }
}

HashTable *php_scylladb_execution_options_properties(zend_object *object)
{
    HashTable *props = zend_std_get_properties(object);

    return props;
}

int php_scylladb_execution_options_compare(zval *obj1, zval *obj2)
{
    PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}
HashTable *php_scylladb_execution_options_gc(zend_object *object, zval **table, int *n)
{
    auto self = php_scylladb_execution_options_object_fetch(object);
    zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
    zend_get_gc_buffer_add_zval(buffer, &self->timeout);
    zend_get_gc_buffer_add_zval(buffer, &self->arguments);
    zend_get_gc_buffer_add_zval(buffer, &self->retry_policy);
    zend_get_gc_buffer_use(buffer, table, n);

    return nullptr;
}


void php_scylladb_execution_options_free(zend_object *object)
{
    auto self = php_scylladb_execution_options_object_fetch(object);

    if (self->paging_state_token)
    {
        zend_string_release(self->paging_state_token);
    }
    zval_ptr_dtor(&self->arguments);
    zval_ptr_dtor(&self->timeout);
    zval_ptr_dtor(&self->retry_policy);

    zend_object_std_dtor(&self->zendObject);
}

zend_object* php_scylladb_execution_options_new(zend_class_entry *ce)
{
    php_scylladb_execution_options *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_execution_options, ce, &php_scylladb_execution_options_handlers);

    init_execution_options(self);
    return &self->zendObject;
}
