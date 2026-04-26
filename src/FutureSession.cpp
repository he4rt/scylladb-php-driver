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
#include "php_driver_globals.h"
#include "php_driver_types.h"
#include "util/future.h"
#include "util/ref.h"
BEGIN_EXTERN_C()
#include "FutureSession_arginfo.h"

zend_class_entry *php_driver_future_session_ce = NULL;

ZEND_METHOD(Cassandra_FutureSession, get)
{
  zval *timeout = NULL;
  CassError rc = CASS_OK;
  php_driver_session *session = NULL;
  php_driver_future_session *self = NULL;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_DRIVER_GET_FUTURE_SESSION(getThis());

  if (self->exception_message) {
    zend_throw_exception_ex(exception_class(self->exception_code),
                            self->exception_code, "%s", self->exception_message);
    return;
  }

  if (!Z_ISUNDEF(self->default_session)) {
    RETURN_ZVAL(&self->default_session, 1, 0);
  }

  object_init_ex(return_value, php_driver_default_session_ce);
  session = PHP_DRIVER_GET_SESSION(return_value);

  session->session = php_driver_add_ref(self->session);
  session->persist = self->persist;

  if (php_driver_future_wait_timed(self->future, timeout) == FAILURE) {
    if (self->persist && self->hash_key) {
      /* Remove timed-out pending session so the next request reconnects. */
      if (zend_hash_str_del(&EG(persistent_list), self->hash_key, self->hash_key_len) == SUCCESS) {
        self->future = NULL;
      }
    }
    return;
  }

  rc = cass_future_error_code(self->future);

  if (rc != CASS_OK) {
    const char *message;
    size_t message_len;
    cass_future_error_message(self->future, &message, &message_len);

    if (self->persist) {
      self->exception_message = estrndup(message, message_len);
      self->exception_code    = rc;

      if (zend_hash_str_del(&EG(persistent_list), self->hash_key, self->hash_key_len) == SUCCESS) {
        self->future = NULL;
      }

      zend_throw_exception_ex(exception_class(self->exception_code),
                              self->exception_code, "%s", self->exception_message);
      return;
    }

    zend_throw_exception_ex(exception_class(rc), rc,
                            "%.*s", (int)message_len, message);
    return;
  }

  ZVAL_COPY(&self->default_session, return_value);
}

static zend_object_handlers php_driver_future_session_handlers;

static HashTable *php_driver_future_session_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

static int php_driver_future_session_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj2);
}

static void php_driver_future_session_free(zend_object *object)
{
  php_driver_future_session *self = PHP5TO7_ZEND_OBJECT_GET(future_session, object);

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

  zval_ptr_dtor(&self->default_session);

  zend_object_std_dtor(&self->zendObject);
}

static zend_object *php_driver_future_session_new(zend_class_entry *ce)
{
  php_driver_future_session *self = PHP5TO7_ZEND_OBJECT_ECALLOC(future_session, ce);

  self->session           = nullptr;
  self->future            = nullptr;
  self->exception_message = nullptr;
  self->hash_key          = nullptr;
  self->persist           = cass_false;

  ZVAL_UNDEF(&self->default_session);

  PHP5TO7_ZEND_OBJECT_INIT(future_session, self, ce);
}

END_EXTERN_C()

void php_driver_define_FutureSession()
{
  php_driver_future_session_ce = register_class_Cassandra_FutureSession(php_driver_future_ce);
  php_driver_future_session_ce->create_object = php_driver_future_session_new;

  memcpy(&php_driver_future_session_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_future_session_handlers.get_properties = php_driver_future_session_properties;
  php_driver_future_session_handlers.compare = php_driver_future_session_compare;
  php_driver_future_session_handlers.clone_obj = nullptr;
}
