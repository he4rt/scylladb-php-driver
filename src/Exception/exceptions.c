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

zend_class_entry *php_driver_exception_ce                  = NULL;
zend_class_entry *php_driver_runtime_exception_ce          = NULL;
zend_class_entry *php_driver_logic_exception_ce            = NULL;
zend_class_entry *php_driver_domain_exception_ce           = NULL;
zend_class_entry *php_driver_invalid_argument_exception_ce = NULL;
zend_class_entry *php_driver_range_exception_ce            = NULL;
zend_class_entry *php_driver_divide_by_zero_exception_ce   = NULL;
zend_class_entry *php_driver_timeout_exception_ce          = NULL;
zend_class_entry *php_driver_execution_exception_ce        = NULL;
zend_class_entry *php_driver_validation_exception_ce       = NULL;
zend_class_entry *php_driver_protocol_exception_ce         = NULL;
zend_class_entry *php_driver_authentication_exception_ce   = NULL;
zend_class_entry *php_driver_server_exception_ce           = NULL;
zend_class_entry *php_driver_read_timeout_exception_ce     = NULL;
zend_class_entry *php_driver_write_timeout_exception_ce    = NULL;
zend_class_entry *php_driver_unavailable_exception_ce      = NULL;
zend_class_entry *php_driver_truncate_exception_ce         = NULL;
zend_class_entry *php_driver_invalid_query_exception_ce    = NULL;
zend_class_entry *php_driver_invalid_syntax_exception_ce   = NULL;
zend_class_entry *php_driver_unauthorized_exception_ce     = NULL;
zend_class_entry *php_driver_unprepared_exception_ce       = NULL;
zend_class_entry *php_driver_configuration_exception_ce    = NULL;
zend_class_entry *php_driver_already_exists_exception_ce   = NULL;
zend_class_entry *php_driver_is_bootstrapping_exception_ce = NULL;
zend_class_entry *php_driver_overloaded_exception_ce       = NULL;

void php_driver_define_Exceptions(void)
{
    /* Cassandra\Exception interface */
    php_driver_exception_ce = register_class_Cassandra_Exception();

    /* SPL-rooted classes that implement Cassandra\Exception */
    php_driver_runtime_exception_ce =
        register_class_Cassandra_Exception_RuntimeException(spl_ce_RuntimeException, php_driver_exception_ce);

    php_driver_logic_exception_ce =
        register_class_Cassandra_Exception_LogicException(spl_ce_LogicException, php_driver_exception_ce);

    php_driver_domain_exception_ce =
        register_class_Cassandra_Exception_DomainException(spl_ce_DomainException, php_driver_exception_ce);

    php_driver_invalid_argument_exception_ce =
        register_class_Cassandra_Exception_InvalidArgumentException(spl_ce_InvalidArgumentException, php_driver_exception_ce);

    php_driver_range_exception_ce =
        register_class_Cassandra_Exception_RangeException(spl_ce_RangeException, php_driver_exception_ce);

    /* Children of RangeException */
    php_driver_divide_by_zero_exception_ce =
        register_class_Cassandra_Exception_DivideByZeroException(php_driver_range_exception_ce);

    /* Children of RuntimeException */
    php_driver_timeout_exception_ce =
        register_class_Cassandra_Exception_TimeoutException(php_driver_runtime_exception_ce);

    php_driver_execution_exception_ce =
        register_class_Cassandra_Exception_ExecutionException(php_driver_runtime_exception_ce);

    php_driver_validation_exception_ce =
        register_class_Cassandra_Exception_ValidationException(php_driver_runtime_exception_ce);

    php_driver_protocol_exception_ce =
        register_class_Cassandra_Exception_ProtocolException(php_driver_runtime_exception_ce);

    php_driver_authentication_exception_ce =
        register_class_Cassandra_Exception_AuthenticationException(php_driver_runtime_exception_ce);

    php_driver_server_exception_ce =
        register_class_Cassandra_Exception_ServerException(php_driver_runtime_exception_ce);

    /* Children of ExecutionException */
    php_driver_read_timeout_exception_ce =
        register_class_Cassandra_Exception_ReadTimeoutException(php_driver_execution_exception_ce);

    php_driver_write_timeout_exception_ce =
        register_class_Cassandra_Exception_WriteTimeoutException(php_driver_execution_exception_ce);

    php_driver_unavailable_exception_ce =
        register_class_Cassandra_Exception_UnavailableException(php_driver_execution_exception_ce);

    php_driver_truncate_exception_ce =
        register_class_Cassandra_Exception_TruncateException(php_driver_execution_exception_ce);

    /* Children of ValidationException */
    php_driver_invalid_query_exception_ce =
        register_class_Cassandra_Exception_InvalidQueryException(php_driver_validation_exception_ce);

    php_driver_invalid_syntax_exception_ce =
        register_class_Cassandra_Exception_InvalidSyntaxException(php_driver_validation_exception_ce);

    php_driver_unauthorized_exception_ce =
        register_class_Cassandra_Exception_UnauthorizedException(php_driver_validation_exception_ce);

    php_driver_unprepared_exception_ce =
        register_class_Cassandra_Exception_UnpreparedException(php_driver_validation_exception_ce);

    php_driver_configuration_exception_ce =
        register_class_Cassandra_Exception_ConfigurationException(php_driver_validation_exception_ce);

    /* Children of ConfigurationException */
    php_driver_already_exists_exception_ce =
        register_class_Cassandra_Exception_AlreadyExistsException(php_driver_configuration_exception_ce);

    /* Children of ServerException */
    php_driver_is_bootstrapping_exception_ce =
        register_class_Cassandra_Exception_IsBootstrappingException(php_driver_server_exception_ce);

    php_driver_overloaded_exception_ce =
        register_class_Cassandra_Exception_OverloadedException(php_driver_server_exception_ce);
}
