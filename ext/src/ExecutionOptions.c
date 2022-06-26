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

#include <RetryPolicy/RetryPolicy.h>

zend_class_entry* php_driver_execution_options_ce = NULL;

static void
init_execution_options(php_driver_execution_options* self)
{
  self->consistency             = -1;
  self->serial_consistency      = -1;
  self->page_size               = -1;
  self->paging_state_token      = NULL;
  self->paging_state_token_size = 0;
  self->timestamp               = INT64_MIN;
  ZVAL_UNDEF(&self->arguments);
  ZVAL_UNDEF(&self->timeout);
  ZVAL_UNDEF(&self->retry_policy);
}

static int
build_from_array(php_driver_execution_options* self, zval* options, int copy)
{
  zval* consistency        = NULL;
  zval* serial_consistency = NULL;
  zval* page_size          = NULL;
  zval* paging_state_token = NULL;
  zval* timeout            = NULL;
  zval* arguments          = NULL;
  zval* retry_policy       = NULL;
  zval* timestamp          = NULL;

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "consistency", sizeof("consistency"), consistency)) {
    if (php_driver_get_consistency(PHP5TO7_ZVAL_MAYBE_DEREF(consistency), &self->consistency) == FAILURE) {
      return FAILURE;
    }
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "serial_consistency", sizeof("serial_consistency"), serial_consistency)) {
    if (php_driver_get_serial_consistency(PHP5TO7_ZVAL_MAYBE_DEREF(serial_consistency), &self->serial_consistency) == FAILURE) {
      return FAILURE;
    }
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "page_size", sizeof("page_size"), page_size)) {
    if (Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(page_size)) != IS_LONG || Z_LVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(page_size)) <= 0) {
      throw_invalid_argument(PHP5TO7_ZVAL_MAYBE_DEREF(page_size), "page_size", "greater than zero");
      return FAILURE;
    }
    self->page_size = Z_LVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(page_size));
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "paging_state_token", sizeof("paging_state_token"), paging_state_token)) {
    if (Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(paging_state_token)) != IS_STRING) {
      throw_invalid_argument(PHP5TO7_ZVAL_MAYBE_DEREF(paging_state_token), "paging_state_token", "a string");
      return FAILURE;
    }
    if (copy) {
      self->paging_state_token = estrndup(Z_STRVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(paging_state_token)),
                                          Z_STRLEN_P(PHP5TO7_ZVAL_MAYBE_DEREF(paging_state_token)));
    } else {
      self->paging_state_token = Z_STRVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(paging_state_token));
    }
    self->paging_state_token_size = Z_STRLEN_P(PHP5TO7_ZVAL_MAYBE_DEREF(paging_state_token));
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "timeout", sizeof("timeout"), timeout)) {
    if (!(Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(timeout)) == IS_LONG && Z_LVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(timeout)) > 0) && !(Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(timeout)) == IS_DOUBLE && Z_DVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(timeout)) > 0) && !(Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(timeout)) == IS_NULL)) {
      throw_invalid_argument(PHP5TO7_ZVAL_MAYBE_DEREF(timeout), "timeout", "a number of seconds greater than zero or null");
      return FAILURE;
    }

    if (copy) {
      PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->timeout), PHP5TO7_ZVAL_MAYBE_DEREF(timeout));
    } else {
      self->timeout = *timeout;
    }
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "arguments", sizeof("arguments"), arguments)) {
    if (Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(arguments)) != IS_ARRAY) {
      throw_invalid_argument(PHP5TO7_ZVAL_MAYBE_DEREF(arguments), "arguments", "an array");
      return FAILURE;
    }

    if (copy) {
      PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->arguments), PHP5TO7_ZVAL_MAYBE_DEREF(arguments));
    } else {
      self->arguments = *arguments;
    }
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "retry_policy", sizeof("retry_policy"), retry_policy)) {
    if (Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(retry_policy)) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(retry_policy), phpDriverRetryPolicyInterfaceCe)) {
      throw_invalid_argument(PHP5TO7_ZVAL_MAYBE_DEREF(retry_policy),
                             "retry_policy",
                             "an instance of " PHP_DRIVER_NAMESPACE "\\RetryPolicy");
      return FAILURE;
    }

    if (copy) {
      PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->retry_policy), PHP5TO7_ZVAL_MAYBE_DEREF(retry_policy));
    } else {
      self->retry_policy = *retry_policy;
    }
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL_P(options), "timestamp", sizeof("timestamp"), timestamp)) {
    if (Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(timestamp)) == IS_LONG) {
      self->timestamp = Z_LVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(timestamp));
    } else if (Z_TYPE_P(PHP5TO7_ZVAL_MAYBE_DEREF(timestamp)) == IS_STRING) {
      if (!php_driver_parse_bigint(Z_STRVAL_P(PHP5TO7_ZVAL_MAYBE_DEREF(timestamp)),
                                   Z_STRLEN_P(PHP5TO7_ZVAL_MAYBE_DEREF(timestamp)),
                                   &self->timestamp)) {
        return FAILURE;
      }
    } else {
      throw_invalid_argument(PHP5TO7_ZVAL_MAYBE_DEREF(timestamp), "timestamp", "an integer or integer string");
      return FAILURE;
    }
  }
  return SUCCESS;
}

int
php_driver_execution_options_build_local_from_array(php_driver_execution_options* self, zval* options)
{
  init_execution_options(self);
  return build_from_array(self, options, 0);
}

PHP_METHOD(ExecutionOptions, __construct)
{
  zval* options                      = NULL;
  php_driver_execution_options* self = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &options) == FAILURE) {
    return;
  }

  if (!options)
    return;

  if (Z_TYPE_P(options) != IS_ARRAY) {
    INVALID_ARGUMENT(options, "an array");
  }

  self = PHP_DRIVER_GET_EXECUTION_OPTIONS(getThis());

  build_from_array(self, options, 1);
}

PHP_METHOD(ExecutionOptions, __get)
{
  char* name;
  size_t name_len;

  php_driver_execution_options* self = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &name, &name_len) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_EXECUTION_OPTIONS(getThis());

  if (name_len == 11 && strncmp("consistency", name, name_len) == 0) {
    if (self->consistency == -1) {
      RETURN_NULL();
    }
    RETURN_LONG(self->consistency);
  } else if (name_len == 17 && strncmp("serialConsistency", name, name_len) == 0) {
    if (self->serial_consistency == -1) {
      RETURN_NULL();
    }
    RETURN_LONG(self->serial_consistency);
  } else if (name_len == 8 && strncmp("pageSize", name, name_len) == 0) {
    if (self->page_size == -1) {
      RETURN_NULL();
    }
    RETURN_LONG(self->page_size);
  } else if (name_len == 16 && strncmp("pagingStateToken", name, name_len) == 0) {
    if (!self->paging_state_token) {
      RETURN_NULL();
    }
    RETURN_STRINGL(self->paging_state_token,
                   self->paging_state_token_size);
  } else if (name_len == 7 && strncmp("timeout", name, name_len) == 0) {
    if (PHP5TO7_ZVAL_IS_UNDEF(self->timeout)) {
      RETURN_NULL();
    }
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->timeout), 1, 0);
  } else if (name_len == 9 && strncmp("arguments", name, name_len) == 0) {
    if (PHP5TO7_ZVAL_IS_UNDEF(self->arguments)) {
      RETURN_NULL();
    }
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->arguments), 1, 0);
  } else if (name_len == 11 && strncmp("retryPolicy", name, name_len) == 0) {
    if (PHP5TO7_ZVAL_IS_UNDEF(self->retry_policy)) {
      RETURN_NULL();
    }
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->retry_policy), 1, 0);
  } else if (name_len == 9 && strncmp("timestamp", name, name_len) == 0) {
    char* string;
    if (self->timestamp == INT64_MIN) {
      RETURN_NULL();
    }
#ifdef WIN32
    spprintf(&string, 0, "%I64d", (long long int) self->timestamp);
#else
    spprintf(&string, 0, "%lld", (long long int) self->timestamp);
#endif
    RETVAL_STRING(string);
    efree(string);
  }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 0)
ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo___get, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_execution_options_methods[] = {
  PHP_ME(ExecutionOptions, __construct, arginfo__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR | ZEND_ACC_DEPRECATED)
    PHP_ME(ExecutionOptions, __get, arginfo___get, ZEND_ACC_PUBLIC)
      PHP_FE_END
};

static zend_object_handlers php_driver_execution_options_handlers;

static HashTable*
php_driver_execution_options_properties(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object
#else
  zval* object
#endif
)
{
  HashTable* props = zend_std_get_properties(object);

  return props;
}

static int
php_driver_execution_options_compare(zval* obj1, zval* obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_execution_options_free(zend_object* object)
{
  php_driver_execution_options* self =
    PHP5TO7_ZEND_OBJECT_GET(execution_options, object);

  if (self->paging_state_token) {
    efree(self->paging_state_token);
  }
  ZVAL_DESTROY(self->arguments);
  ZVAL_DESTROY(self->timeout);
  ZVAL_DESTROY(self->retry_policy);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_execution_options_new(zend_class_entry* ce)
{
  php_driver_execution_options* self =
    PHP5TO7_ZEND_OBJECT_ECALLOC(execution_options, ce);

  init_execution_options(self);

  PHP5TO7_ZEND_OBJECT_INIT(execution_options, self, ce);
}

void
php_driver_define_ExecutionOptions()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\ExecutionOptions", php_driver_execution_options_methods);
  php_driver_execution_options_ce = zend_register_internal_class(&ce);
  php_driver_execution_options_ce->ce_flags |= ZEND_ACC_FINAL;
  php_driver_execution_options_ce->create_object = php_driver_execution_options_new;

  memcpy(&php_driver_execution_options_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_execution_options_handlers.get_properties = php_driver_execution_options_properties;
#if PHP_MAJOR_VERSION >= 8
  php_driver_execution_options_handlers.compare = php_driver_execution_options_compare;
#else
  php_driver_execution_options_handlers.compare_objects = php_driver_execution_options_compare;
#endif
  php_driver_execution_options_handlers.clone_obj = NULL;
}
