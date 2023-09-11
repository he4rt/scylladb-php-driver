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

BEGIN_EXTERN_C()
#include "RetryPolicy_arginfo.h"
zend_class_entry *php_scylladb_retry_policy_ce = nullptr;
END_EXTERN_C()


void php_scylladb_define_RetryPolicyDefault(zend_class_entry *);
void php_scylladb_define_RetryPolicyDowngradingConsistency(zend_class_entry *);
void php_scylladb_define_RetryPolicyFallthrough(zend_class_entry *);
void php_scylladb_define_RetryPolicyLogging(zend_class_entry *);

void php_scylladb_define_RetryPolicy() {
  php_scylladb_retry_policy_ce = register_class_Cassandra_RetryPolicy();
  php_scylladb_define_RetryPolicyDefault(php_scylladb_retry_policy_ce);
  php_scylladb_define_RetryPolicyDowngradingConsistency(php_scylladb_retry_policy_ce);
  php_scylladb_define_RetryPolicyFallthrough(php_scylladb_retry_policy_ce);
  php_scylladb_define_RetryPolicyLogging(php_scylladb_retry_policy_ce);
}
