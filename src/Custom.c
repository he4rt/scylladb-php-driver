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

#include <Registry/Registry.h>

zend_class_entry* php_scylladb_custom_ce = NULL;

static zend_function_entry php_scylladb_custom_methods[] = {
  PHP_FE_END
};

static zend_class_entry *php_scylladb_register_custom(zend_class_entry *const *deps)
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_SCYLLADB_NAMESPACE "\\Custom", php_scylladb_custom_methods);
  zend_class_entry *out = zend_register_internal_class(&ce);
  zend_class_implements(out, 1, deps[0]);
  out->ce_flags |= ZEND_ACC_ABSTRACT;
  return out;
}

PHP_SCYLLADB_REGISTER_CLASS(
    custom,
    "Cassandra\\Custom",
    &php_scylladb_custom_ce,
    "Cassandra\\Value",
    php_scylladb_register_custom
)
