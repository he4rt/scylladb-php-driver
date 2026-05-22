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

  if (self->session == NULL || self->future == NULL) {
    zend_throw_exception_ex(php_driver_runtime_exception_ce, 0,
                            "FutureSession has no associated session (cached entry expired)");
    return;
  }

  object_init_ex(return_value, php_driver_default_session_ce);
  session = PHP_DRIVER_GET_SESSION(return_value);

  /* Transfer CassSession ownership: persistent path's psession owns it
     so the future was borrowing; non-persistent path's future owned it
     until now and the DefaultSession takes over. */
  session->session = self->session;
  session->persist = self->persist;
  if (!self->persist) {
    self->session = nullptr;   /* ownership transferred to DefaultSession */
  }

  if (php_driver_future_wait_timed(self->future, timeout) == FAILURE) {
    if (self->persist && self->cache_key) {
      /* Remove timed-out pending session so the next request reconnects. */
      if (zend_hash_index_del(&EG(persistent_list), self->cache_key) == SUCCESS) {
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

      if (zend_hash_index_del(&EG(persistent_list), self->cache_key) == SUCCESS) {
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
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

static void php_driver_future_session_free(zend_object *object)
{
  php_driver_future_session *self = php_driver_future_session_object_fetch(object);

  if (!self->persist && self->future) {
    cass_future_free(self->future);
  }

  /* Free CassSession only if non-persistent AND not transferred via get(). */
  if (!self->persist && self->session) {
    cass_session_free(self->session);
    self->session = nullptr;
  }

  if (self->exception_message) {
    efree(self->exception_message);
  }

  if (self->session_keyspace) {
    efree(self->session_keyspace);
    self->session_keyspace = nullptr;
  }

  zval_ptr_dtor(&self->default_session);

  zend_object_std_dtor(&self->zendObject);
}

static zend_object *php_driver_future_session_new(zend_class_entry *ce)
{
  php_driver_future_session *self = (php_driver_future_session *)ecalloc(1, sizeof(php_driver_future_session) + zend_object_properties_size(ce));

  self->session           = nullptr;
  self->future            = nullptr;
  self->exception_message = nullptr;
  self->cache_key         = 0;
  self->persist           = cass_false;

  ZVAL_UNDEF(&self->default_session);

  zend_object_std_init(&self->zendObject, ce);
  self->zendObject.handlers = &php_driver_future_session_handlers;
  return &self->zendObject;
}

void php_driver_define_FutureSession()
{
  php_driver_future_session_ce = register_class_Cassandra_FutureSession(php_driver_future_ce);
  php_driver_future_session_ce->create_object = php_driver_future_session_new;

  memcpy(&php_driver_future_session_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_future_session_handlers.offset = XtOffsetOf(php_driver_future_session, zendObject);
  php_driver_future_session_handlers.free_obj = php_driver_future_session_free;
  php_driver_future_session_handlers.get_properties = php_driver_future_session_properties;
  php_driver_future_session_handlers.compare = php_driver_future_session_compare;
  php_driver_future_session_handlers.clone_obj = nullptr;
}

END_EXTERN_C()
