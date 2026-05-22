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

/* Class registration (global ce, handlers struct, register fn,
 * PHP_SCYLLADB_REGISTER_CLASS macro) is generated into
 * Fallthrough_descriptor.c from Fallthrough.stub.php — see
 * tools/gen_descriptor/gen_class_descriptor.php.
 *
 * This file only provides the user-specific callbacks (free, new) and
 * any public API (e.g. _instantiate). The descriptor's weak forward
 * declarations bind to whichever of these we define here, and skip the
 * ones we don't. */

extern zend_object_handlers php_scylladb_retry_policy_fallthrough_handlers;

PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_fallthrough_instantiate(zval *dst)
{
  zval val;

  if (object_init_ex(&val, php_scylladb_retry_policy_default_policy_ce) == FAILURE) {
    return nullptr;
  }

  ZVAL_OBJ(dst, Z_OBJ(val));

  php_scylladb_retry_policy *obj = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, Z_OBJ_P(dst));
  obj->policy = cass_retry_policy_fallthrough_new();
  return obj;
}

void php_scylladb_retry_policy_fallthrough_free(zend_object *object)
{
  php_scylladb_retry_policy *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, object);
  cass_retry_policy_free(self->policy);
  zend_object_std_dtor(object);
}

zend_object *php_scylladb_retry_policy_fallthrough_new(zend_class_entry *ce)
{
  php_scylladb_retry_policy *self = PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_retry_policy, ce, &php_scylladb_retry_policy_fallthrough_handlers);
  self->policy = cass_retry_policy_fallthrough_new();
  return &self->zendObject;
}
