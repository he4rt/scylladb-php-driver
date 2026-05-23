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

#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include <SSLOptions/SSLOptions.h>

#include "SSLOptions_arginfo.h"

extern zend_object_handlers php_scylladb_ssl_options_handlers;

int php_scylladb_ssl_options_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_ssl_options_free(zend_object *object) {
  const php_scylladb_ssl *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_ssl, object);
  cass_ssl_free(self->ssl);
  zend_object_std_dtor(object);
}

zend_object *php_scylladb_ssl_options_new(zend_class_entry *ce) {
  php_scylladb_ssl *self = PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_ssl, ce, &php_scylladb_ssl_options_handlers);
  self->ssl = cass_ssl_new();
  if (self->ssl == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassSsl context");
  }

  return &self->zendObject;
}

void php_scylladb_ssl_options_post_register([[maybe_unused]] zend_class_entry *ce) {
  php_scylladb_ssl_options_handlers.clone_obj = nullptr;
}

PHP_SCYLLADB_API php_scylladb_ssl *php_scylladb_ssl_instantiate(zval *object) {
  zval val;
  if (object_init_ex(&val, php_scylladb_ssl_options_ce) != SUCCESS) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));
  auto ssl = Z_SCYLLADB_SSL_P(object);
  return ssl;
}
