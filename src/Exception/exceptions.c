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

#include <php.h>
#include <ext/spl/spl_exceptions.h>

#include "exceptions_arginfo.h"
#include <Registry/Registry.h>

zend_class_entry *php_scylladb_exception_ce                  = nullptr;
zend_class_entry *php_scylladb_runtime_exception_ce          = nullptr;
zend_class_entry *php_scylladb_logic_exception_ce            = nullptr;
zend_class_entry *php_scylladb_domain_exception_ce           = nullptr;
zend_class_entry *php_scylladb_invalid_argument_exception_ce = nullptr;
zend_class_entry *php_scylladb_range_exception_ce            = nullptr;
zend_class_entry *php_scylladb_divide_by_zero_exception_ce   = nullptr;
zend_class_entry *php_scylladb_timeout_exception_ce          = nullptr;
zend_class_entry *php_scylladb_execution_exception_ce        = nullptr;
zend_class_entry *php_scylladb_validation_exception_ce       = nullptr;
zend_class_entry *php_scylladb_protocol_exception_ce         = nullptr;
zend_class_entry *php_scylladb_authentication_exception_ce   = nullptr;
zend_class_entry *php_scylladb_server_exception_ce           = nullptr;
zend_class_entry *php_scylladb_read_timeout_exception_ce     = nullptr;
zend_class_entry *php_scylladb_write_timeout_exception_ce    = nullptr;
zend_class_entry *php_scylladb_unavailable_exception_ce      = nullptr;
zend_class_entry *php_scylladb_truncate_exception_ce         = nullptr;
zend_class_entry *php_scylladb_invalid_query_exception_ce    = nullptr;
zend_class_entry *php_scylladb_invalid_syntax_exception_ce   = nullptr;
zend_class_entry *php_scylladb_unauthorized_exception_ce     = nullptr;
zend_class_entry *php_scylladb_unprepared_exception_ce       = nullptr;
zend_class_entry *php_scylladb_configuration_exception_ce    = nullptr;
zend_class_entry *php_scylladb_already_exists_exception_ce   = nullptr;
zend_class_entry *php_scylladb_is_bootstrapping_exception_ce = nullptr;
zend_class_entry *php_scylladb_overloaded_exception_ce       = nullptr;

/* All 23 exception classes register in one shot — they only depend on SPL
 * classes (which are globals available before MINIT) and on each other, so
 * there's no cross-module ordering to manage. The umbrella interface
 * Cassandra\Exception is the only ce_out the registry tracks; the children
 * are set as side effects (no other registry-owned class extends them). */
static zend_class_entry *php_scylladb_register_exceptions([[maybe_unused]] zend_class_entry *const *deps)
{
    /* Cassandra\Exception interface */
    php_scylladb_exception_ce = register_class_Cassandra_Exception();

    /* SPL-rooted classes that implement Cassandra\Exception */
    php_scylladb_runtime_exception_ce =
        register_class_Cassandra_Exception_RuntimeException(spl_ce_RuntimeException, php_scylladb_exception_ce);

    php_scylladb_logic_exception_ce =
        register_class_Cassandra_Exception_LogicException(spl_ce_LogicException, php_scylladb_exception_ce);

    php_scylladb_domain_exception_ce =
        register_class_Cassandra_Exception_DomainException(spl_ce_DomainException, php_scylladb_exception_ce);

    php_scylladb_invalid_argument_exception_ce =
        register_class_Cassandra_Exception_InvalidArgumentException(spl_ce_InvalidArgumentException, php_scylladb_exception_ce);

    php_scylladb_range_exception_ce =
        register_class_Cassandra_Exception_RangeException(spl_ce_RangeException, php_scylladb_exception_ce);

    /* Children of RangeException */
    php_scylladb_divide_by_zero_exception_ce =
        register_class_Cassandra_Exception_DivideByZeroException(php_scylladb_range_exception_ce);

    /* Children of RuntimeException */
    php_scylladb_timeout_exception_ce =
        register_class_Cassandra_Exception_TimeoutException(php_scylladb_runtime_exception_ce);

    php_scylladb_execution_exception_ce =
        register_class_Cassandra_Exception_ExecutionException(php_scylladb_runtime_exception_ce);

    php_scylladb_validation_exception_ce =
        register_class_Cassandra_Exception_ValidationException(php_scylladb_runtime_exception_ce);

    php_scylladb_protocol_exception_ce =
        register_class_Cassandra_Exception_ProtocolException(php_scylladb_runtime_exception_ce);

    php_scylladb_authentication_exception_ce =
        register_class_Cassandra_Exception_AuthenticationException(php_scylladb_runtime_exception_ce);

    php_scylladb_server_exception_ce =
        register_class_Cassandra_Exception_ServerException(php_scylladb_runtime_exception_ce);

    /* Children of ExecutionException */
    php_scylladb_read_timeout_exception_ce =
        register_class_Cassandra_Exception_ReadTimeoutException(php_scylladb_execution_exception_ce);

    php_scylladb_write_timeout_exception_ce =
        register_class_Cassandra_Exception_WriteTimeoutException(php_scylladb_execution_exception_ce);

    php_scylladb_unavailable_exception_ce =
        register_class_Cassandra_Exception_UnavailableException(php_scylladb_execution_exception_ce);

    php_scylladb_truncate_exception_ce =
        register_class_Cassandra_Exception_TruncateException(php_scylladb_execution_exception_ce);

    /* Children of ValidationException */
    php_scylladb_invalid_query_exception_ce =
        register_class_Cassandra_Exception_InvalidQueryException(php_scylladb_validation_exception_ce);

    php_scylladb_invalid_syntax_exception_ce =
        register_class_Cassandra_Exception_InvalidSyntaxException(php_scylladb_validation_exception_ce);

    php_scylladb_unauthorized_exception_ce =
        register_class_Cassandra_Exception_UnauthorizedException(php_scylladb_validation_exception_ce);

    php_scylladb_unprepared_exception_ce =
        register_class_Cassandra_Exception_UnpreparedException(php_scylladb_validation_exception_ce);

    php_scylladb_configuration_exception_ce =
        register_class_Cassandra_Exception_ConfigurationException(php_scylladb_validation_exception_ce);

    /* Children of ConfigurationException */
    php_scylladb_already_exists_exception_ce =
        register_class_Cassandra_Exception_AlreadyExistsException(php_scylladb_configuration_exception_ce);

    /* Children of ServerException */
    php_scylladb_is_bootstrapping_exception_ce =
        register_class_Cassandra_Exception_IsBootstrappingException(php_scylladb_server_exception_ce);

    php_scylladb_overloaded_exception_ce =
        register_class_Cassandra_Exception_OverloadedException(php_scylladb_server_exception_ce);

    return php_scylladb_exception_ce;
}

PHP_SCYLLADB_REGISTER_CLASS(
    exceptions,
    "Cassandra\\Exception",
    &php_scylladb_exception_ce,
    nullptr,
    php_scylladb_register_exceptions
)
