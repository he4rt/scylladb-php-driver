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

#include <cassandra.h>
#include <spl/spl_exceptions.h>
#include <zend_exceptions.h>

#include "LoggingRetryPolicy_arginfo.h"

#include <RetryPolicy/RetryPolicy.h>

zend_class_entry* phpDriverRetryPolicyLoggingCe = NULL;

ZEND_METHOD(Cassandra_RetryPolicy_LoggingRetryPolicy, __construct)
{
  zval* child_policy = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
                            &child_policy, phpDriverRetryPolicyInterfaceCe)
      == FAILURE) {
    return;
  }

  if (instanceof_function(Z_OBJCE_P(child_policy),
                          phpDriverRetryPolicyLoggingCe)) {
    zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0,
                            "Cannot add a Cassandra\\RetryPolicy\\LoggingRetryPolicy as child policy of Cassandra\\RetryPolicy\\LoggingRetryPolicy");
    return;
  }

  php_driver_retry_policy* self         = PHP_DRIVER_RETRY_POLICY_THIS();
  php_driver_retry_policy* retry_policy = PHP_DRIVER_RETRY_POLICY_ZVAL_TO_OBJECT(child_policy);
  self->policy                          = cass_retry_policy_logging_new(retry_policy->policy);
}

static zend_object_handlers php_driver_retry_policy_logging_handlers;

static void
PhpDriverRetryPolicyLoggingFree(zend_object* object)
{
  php_driver_retry_policy* self = PHP_DRIVER_RETRY_POLICY_OBJECT(object);

  if (self->policy) {
    cass_retry_policy_free(self->policy);
  }
}

static zend_object*
PhpDriverRetryPolicyLoggingNew(zend_class_entry* ce)
{
  php_driver_retry_policy* self = make(php_driver_retry_policy);

  self->policy = NULL;

  self->zval.handlers = &php_driver_retry_policy_logging_handlers;

  return &self->zval;
}

void
PhpDriverDefineRetryPolicyLogging(zend_class_entry* retryPolicyInterfaceCe)
{
  phpDriverRetryPolicyLoggingCe                = register_class_Cassandra_RetryPolicy_LoggingRetryPolicy(retryPolicyInterfaceCe);
  phpDriverRetryPolicyLoggingCe->create_object = PhpDriverRetryPolicyLoggingNew;

  memcpy(&php_driver_retry_policy_logging_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_retry_policy_logging_handlers.free_obj = PhpDriverRetryPolicyLoggingFree;
  php_driver_retry_policy_logging_handlers.offset   = XtOffsetOf(php_driver_retry_policy, zval);
}
