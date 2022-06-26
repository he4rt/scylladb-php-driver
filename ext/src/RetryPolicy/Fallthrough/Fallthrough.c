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

#include "php_driver_types.h"

#include "Fallthrough.h"
#include "Fallthrough_arginfo.h"

#include <RetryPolicy/RetryPolicy.h>

zend_class_entry* phpDriverRetryPolicyFallthroughCe = NULL;

static zend_object_handlers phpDriverRetryPolicyFallthroughHandlers;

static void
PhpDriverRetryPolicyFallthroughFree(zend_object* object)
{
  php_driver_retry_policy* self = PHP_DRIVER_RETRY_POLICY_OBJECT(object);

  cass_retry_policy_free(self->policy);
}

static zend_object*
PhpDriverRetryPolicyFallthroughNew(zend_class_entry* ce)
{
  php_driver_retry_policy* self = make(php_driver_retry_policy);

  self->policy        = cass_retry_policy_fallthrough_new();
  self->zval.handlers = &phpDriverRetryPolicyFallthroughHandlers;

  return &self->zval;
}

void
PhpDriverDefineRetryPolicyFallthrough(zend_class_entry* retryPolicyInterface)
{
  phpDriverRetryPolicyFallthroughCe                = register_class_Cassandra_RetryPolicy_FallthroughPolicy(retryPolicyInterface);
  phpDriverRetryPolicyFallthroughCe->create_object = PhpDriverRetryPolicyFallthroughNew;

  memcpy(&phpDriverRetryPolicyFallthroughHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  phpDriverRetryPolicyFallthroughHandlers.free_obj = PhpDriverRetryPolicyFallthroughFree;
  phpDriverRetryPolicyFallthroughHandlers.offset   = XtOffsetOf(php_driver_retry_policy, zval);
}
