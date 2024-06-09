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

#include <php_driver.h>
#include <php_driver_types.h>
#include <SSLOptions/SSLOptions.h>

BEGIN_EXTERN_C()
#include "SSLOptions_arginfo.h"

zend_class_entry *php_driver_ssl_ce = nullptr;

static zend_object_handlers php_driver_ssl_handlers;

static int php_driver_ssl_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void php_driver_ssl_free(zend_object *object) {
  const auto *self = ZendCPP::ObjectFetch<php_scylladb_ssl>(object);
  cass_ssl_free(self->ssl);
}

static zend_object *php_driver_ssl_new(zend_class_entry *ce) {
  auto *self = ZendCPP::Allocate<php_scylladb_ssl>(ce, &php_driver_ssl_handlers);
  self->ssl = cass_ssl_new();

  return &self->zendObject;
}

void php_driver_define_SSLOptions() {
  php_driver_ssl_ce = register_class_Cassandra_SSLOptions();
  php_driver_ssl_ce->create_object = php_driver_ssl_new;

  ZendCPP::InitHandlers<php_scylladb_ssl>(&php_driver_ssl_handlers);
  php_driver_ssl_handlers.compare = php_driver_ssl_compare;
  php_driver_ssl_handlers.free_obj = php_driver_ssl_free;
  php_driver_ssl_handlers.clone_obj = nullptr;
}

PHP_SCYLLADB_API php_scylladb_ssl *php_scylladb_ssl_instantiate(zval *object) {
  zval val;
  if (object_init_ex(&val, php_driver_ssl_ce) != SUCCESS) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));
  php_scylladb_ssl *ssl = Z_SCYLLADB_SSL_P(object);
  return ssl;
}
END_EXTERN_C()