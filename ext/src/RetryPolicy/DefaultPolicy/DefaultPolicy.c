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

#include <RetryPolicy/RetryPolicy.h>

#include "DefaultPolicy.h"
#include "DefaultPolicy_arginfo.h"

zend_class_entry* phpDriverRetryPolicyDefaultCe = NULL;

static zend_object_handlers phpDriverRetryPolicyDefaultHandlers;

static void
PhpDriverRetryPolicyDefaultFree(zend_object* object)
{
  php_driver_retry_policy* self = PHP_DRIVER_RETRY_POLICY_OBJECT(object);

  cass_retry_policy_free(self->policy);
}

static zend_object*
PhpDriverRetryPolicyDefaultNew(zend_class_entry* ce)
{
  php_driver_retry_policy* self = make(php_driver_retry_policy);

  self->policy = cass_retry_policy_default_new();

  self->zval.handlers = &phpDriverRetryPolicyDefaultHandlers;

  return &self->zval;
}

void
PhpDriverDefineRetryPolicyDefault(zend_class_entry* retryPolicyInterface)
{
  phpDriverRetryPolicyDefaultCe                = register_class_Cassandra_RetryPolicy_DefaultPolicy(retryPolicyInterface);
  phpDriverRetryPolicyDefaultCe->create_object = PhpDriverRetryPolicyDefaultNew;

  memcpy(&phpDriverRetryPolicyDefaultHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  phpDriverRetryPolicyDefaultHandlers.free_obj = PhpDriverRetryPolicyDefaultFree;
  phpDriverRetryPolicyDefaultHandlers.offset   = XtOffsetOf(php_driver_retry_policy, zval);
}
