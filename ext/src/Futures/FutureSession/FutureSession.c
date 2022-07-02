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

#include <CassandraDriver.h>
#include <Futures/FutureSession.h>

#include "util/FutureInterface.h"
#include "util/ref.h"

#include "FutureSession_arginfo.h"

zend_class_entry* php_driver_future_session_ce = NULL;

ZEND_METHOD(Cassandra_FutureSession, get)
{
  zval* timeout = NULL;
  CassError rc;
  php_driver_session* session = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &timeout) == FAILURE) {
    return;
  }

  php_driver_future_session* self = PHP_DRIVER_FUTURE_SESSION_THIS();

  if (self->exception_message) {
    zend_throw_exception_ex(exception_class(self->exception_code),
                            self->exception_code, "%s", self->exception_message);
    return;
  }

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->default_session)) {
    RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->default_session), 1, 0);
  }

  object_init_ex(return_value, php_driver_default_session_ce);
  session = PHP_DRIVER_GET_SESSION(return_value);

  session->session = php_driver_add_ref(self->session);
  session->persist = self->persist;

  if (php_driver_future_wait_timed(self->future, timeout) == FAILURE) {
    return;
  }

  rc = cass_future_error_code(self->future);

  if (rc != CASS_OK) {
    const char* message;
    size_t message_len;
    cass_future_error_message(self->future, &message, &message_len);

    if (self->persist) {
      self->exception_message = estrndup(message, message_len);
      self->exception_code    = rc;

      if (PHP5TO7_ZEND_HASH_DEL(&EG(persistent_list), self->hash_key, self->hash_key_len + 1)) {
        self->future = NULL;
      }

      zend_throw_exception_ex(exception_class(self->exception_code),
                              self->exception_code, "%s", self->exception_message);
      return;
    }

    zend_throw_exception_ex(exception_class(rc), rc,
                            "%.*s", (int) message_len, message);
    return;
  }

  PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->default_session), return_value);
}

static zend_object_handlers php_driver_future_session_handlers;

static HashTable*
php_driver_future_session_properties(
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
php_driver_future_session_compare(zval* obj1, zval* obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_future_session_free(zend_object* object)
{
  php_driver_future_session* self = PHP_DRIVER_FUTURE_SESSION_OBJECT(object);

  if (self->persist) {
    efree(self->hash_key);
  } else {
    if (self->future) {
      cass_future_free(self->future);
    }
  }

  php_driver_del_peref(&self->session, 1);

  if (self->exception_message) {
    efree(self->exception_message);
  }

  ZVAL_DESTROY(self->default_session);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_future_session_new(zend_class_entry* ce)
{
  php_driver_future_session* self = PHP5TO7_ZEND_OBJECT_ECALLOC(future_session, ce);

  self->session           = NULL;
  self->future            = NULL;
  self->exception_message = NULL;
  self->hash_key          = NULL;
  self->persist           = 0;

  ZVAL_UNDEF(&self->default_session);

  PHP5TO7_ZEND_OBJECT_INIT(future_session, self, ce);
}

void
php_driver_define_FutureSession(zend_class_entry* future_interface)
{
  php_driver_future_session_ce                = register_class_Cassandra_FutureSession(future_interface);
  php_driver_future_session_ce->create_object = php_driver_future_session_new;

  memcpy(&php_driver_future_session_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_future_session_handlers.get_properties = php_driver_future_session_properties;
  php_driver_future_session_handlers.compare        = php_driver_future_session_compare;
  php_driver_future_session_handlers.clone_obj      = NULL;
}
