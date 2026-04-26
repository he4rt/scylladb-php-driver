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
#include "util/consistency.h"
#include "util/math.h"
BEGIN_EXTERN_C()
#include "ExecutionOptions_arginfo.h"

zend_class_entry *php_driver_execution_options_ce = NULL;

static void init_execution_options(php_driver_execution_options *self)
{
    self->consistency = -1;
    self->serial_consistency = -1;
    self->page_size = -1;
    self->paging_state_token = NULL;
    self->paging_state_token_size = 0;
    self->timestamp = INT64_MIN;
    ZVAL_UNDEF(&self->arguments);
    ZVAL_UNDEF(&self->timeout);
    ZVAL_UNDEF(&self->retry_policy);
}

static zend_result build_from_array(php_driver_execution_options *self, zval *options, int copy)
{
    zval *consistency = NULL;
    zval *serial_consistency = NULL;
    zval *page_size = NULL;
    zval *paging_state_token = NULL;
    zval *timeout = NULL;
    zval *arguments = NULL;
    zval *retry_policy = NULL;
    zval *timestamp = NULL;

    if ((consistency = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("consistency"))) != NULL)
    {
        zend_long val = Z_LVAL_P(consistency);

        if (php_driver_validate_consistency(val) == -1)
        {
            throw_invalid_argument(consistency, "consistency", "one of " PHP_DRIVER_NAMESPACE "::CONSISTENCY_*");

            return FAILURE;
        }

        self->consistency = val;
    }

    if ((serial_consistency = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("serial_consistency"))) != NULL)
    {
        zend_long val = Z_LVAL_P(serial_consistency);

        if (php_driver_validate_serial_consistency(val) == -1)
        {
            throw_invalid_argument(serial_consistency, "serial_consistency",
                                   "either " PHP_DRIVER_NAMESPACE
                                   "::CONSISTENCY_SERIAL or Cassandra::CASS_CONSISTENCY_LOCAL_SERIAL");
            return FAILURE;
        }

        self->serial_consistency = val;
    }

    if ((page_size = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("page_size"))) != NULL)
    {
        if (Z_TYPE_P(page_size) != IS_LONG ||
            Z_LVAL_P(page_size) <= 0)
        {
            throw_invalid_argument(page_size, "page_size", "greater than zero");
            return FAILURE;
        }
        self->page_size = Z_LVAL_P(page_size);
    }

    if ((paging_state_token = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("paging_state_token"))) != NULL)
    {
        if (Z_TYPE_P(paging_state_token) != IS_STRING)
        {
            throw_invalid_argument(paging_state_token, "paging_state_token", "a string");
            return FAILURE;
        }
        if (copy)
        {
            self->paging_state_token = estrndup(Z_STRVAL_P(paging_state_token),
                                                Z_STRLEN_P(paging_state_token));
        }
        else
        {
            self->paging_state_token = Z_STRVAL_P(paging_state_token);
        }
        self->paging_state_token_size = Z_STRLEN_P(paging_state_token);
    }

    if ((timeout = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("timeout"))) != NULL)
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

    if ((arguments = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("arguments"))) != NULL)
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

    if ((retry_policy = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("retry_policy"))) != NULL)
    {
        if (Z_TYPE_P(retry_policy) != IS_OBJECT &&
            !instanceof_function(Z_OBJCE_P(retry_policy), php_scylladb_retry_policy_ce))
        {
            throw_invalid_argument(retry_policy, "retry_policy",
                                   "an instance of " PHP_DRIVER_NAMESPACE "\\RetryPolicy");
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

    if ((timestamp = zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("timestamp"))) != NULL)
    {
        if (Z_TYPE_P(timestamp) == IS_LONG)
        {
            self->timestamp = Z_LVAL_P(timestamp);
        }
        else if (Z_TYPE_P(timestamp) == IS_STRING)
        {
            if (!php_driver_parse_bigint(Z_STRVAL_P(timestamp),
                                         Z_STRLEN_P(timestamp), &self->timestamp))
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
    return SUCCESS;
}

int php_driver_execution_options_build_local_from_array(php_driver_execution_options *self, zval *options)
{
    init_execution_options(self);
    return build_from_array(self, options, 0);
}

ZEND_METHOD(Cassandra_ExecutionOptions, __construct)
{
    zval *options = NULL;
    php_driver_execution_options *self = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_DRIVER_GET_EXECUTION_OPTIONS(getThis());

    build_from_array(self, options, 1);
}

ZEND_METHOD(Cassandra_ExecutionOptions, __get)
{
    zend_string *name;
    php_driver_execution_options *self = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_DRIVER_GET_EXECUTION_OPTIONS(getThis());

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
        RETVAL_STRINGL(self->paging_state_token, self->paging_state_token_size);
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
        char *string;
        if (self->timestamp == INT64_MIN)
        {
            RETURN_NULL();
        }
        spprintf(&string, 0, "%lld", (long long int)self->timestamp);
        RETVAL_STRING(string);
        efree(string);
    }
}

static zend_object_handlers php_driver_execution_options_handlers;

static HashTable *php_driver_execution_options_properties(zend_object *object)
{
    HashTable *props = zend_std_get_properties(object);

    return props;
}

static int php_driver_execution_options_compare(zval *obj1, zval *obj2)
{
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return 1; /* different classes */

    return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj2);
}

static void php_driver_execution_options_free(zend_object *object)
{
    php_driver_execution_options *self = php_driver_execution_options_object_fetch(object);

    if (self->paging_state_token)
    {
        efree(self->paging_state_token);
    }
    zval_ptr_dtor(&self->arguments);
    zval_ptr_dtor(&self->timeout);
    zval_ptr_dtor(&self->retry_policy);

    zend_object_std_dtor(&self->zendObject);
}

static zend_object* php_driver_execution_options_new(zend_class_entry *ce)
{
    php_driver_execution_options *self = (php_driver_execution_options *)ecalloc(1, sizeof(php_driver_execution_options) + zend_object_properties_size(ce));

    init_execution_options(self);

    zend_object_std_init(&self->zendObject, ce);
    self->zendObject.handlers = &php_driver_execution_options_handlers;
    return &self->zendObject;
}

END_EXTERN_C()

void php_driver_define_ExecutionOptions()
{
    php_driver_execution_options_ce = register_class_Cassandra_ExecutionOptions();
    php_driver_execution_options_ce->create_object = php_driver_execution_options_new;

    memcpy(&php_driver_execution_options_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_execution_options_handlers.offset = XtOffsetOf(php_driver_execution_options, zendObject);
    php_driver_execution_options_handlers.free_obj = php_driver_execution_options_free;
    php_driver_execution_options_handlers.get_properties = php_driver_execution_options_properties;
    php_driver_execution_options_handlers.compare = php_driver_execution_options_compare;
    php_driver_execution_options_handlers.clone_obj = NULL;
}
