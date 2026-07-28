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

#include <RetryPolicy/RetryPolicy.h>

#include "DowngradingConsistency_arginfo.h"

extern zend_object_handlers php_scylladb_retry_policy_downgrading_consistency_handlers;

PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_downgrading_consistency_instantiate(zval *dst)
{
  if (object_init_ex(dst, php_scylladb_retry_policy_downgrading_consistency_ce) == FAILURE) {
    return nullptr;
  }

  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, Z_OBJ_P(dst));
}

void php_scylladb_retry_policy_downgrading_consistency_free(zend_object *object)
{
  php_scylladb_retry_policy *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, object);
  cass_retry_policy_free(self->policy);
  zend_object_std_dtor(object);
}

zend_object *php_scylladb_retry_policy_downgrading_consistency_new(zend_class_entry *ce) {
  php_scylladb_retry_policy *self = PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_retry_policy, ce, &php_scylladb_retry_policy_downgrading_consistency_handlers);
  self->policy = cass_retry_policy_downgrading_consistency_new();
  return &self->zendObject;
}
