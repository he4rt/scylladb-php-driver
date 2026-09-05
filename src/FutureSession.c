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
#include "php_scylladb_globals.h"
#include "php_scylladb_types.h"
#include "FutureUtil.h"
#include "FutureNotifier.h"

#include "FutureSession_arginfo.h"

extern zend_object_handlers php_scylladb_future_session_handlers;

ZEND_METHOD(Cassandra_FutureSession, __construct)
{
}

ZEND_METHOD(Cassandra_FutureSession, get)
{
  zval *timeout = nullptr;
  CassError rc = CASS_OK;
  php_scylladb_session *session = nullptr;
  php_scylladb_future_session *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_FUTURE_SESSION(ZEND_THIS);

  if (self->exception_message) {
    zend_throw_exception_ex(exception_class(self->exception_code),
                            self->exception_code, "%.*s",
                            (int)ZSTR_LEN(self->exception_message),
                            ZSTR_VAL(self->exception_message));
    RETURN_THROWS();
  }

  if (!Z_ISUNDEF(self->default_session)) {
    RETURN_ZVAL(&self->default_session, 1, 0);
  }

  if (self->session == nullptr || self->future == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "FutureSession has no associated session (cached entry expired)");
    RETURN_THROWS();
  }

  object_init_ex(return_value, php_scylladb_default_session_ce);
  session = PHP_SCYLLADB_GET_SESSION(return_value);

  /* Transfer CassSession ownership: persistent path's psession owns it
     so the future was borrowing; non-persistent path's future owned it
     until now and the DefaultSession takes over. */
  session->session = self->session;
  session->persist = self->persist;
  session->default_consistency = self->default_consistency;
  session->default_page_size = self->default_page_size;
  session->cache_key = self->cache_key;
  session->keyspace = self->session_keyspace ? zend_string_copy(self->session_keyspace) : nullptr;

  if (!Z_ISUNDEF(self->default_timeout)) {
    ZVAL_COPY(&session->default_timeout, &self->default_timeout);
  }

  if (!self->persist) {
    self->session = nullptr;   /* ownership transferred to DefaultSession */
  }

  if (php_scylladb_future_wait_coro(self->future, &self->notifier, self->reactor_reg, timeout) ==
      FAILURE) {
    if (self->persist && self->cache_key) {
      /* Remove timed-out pending session so the next request reconnects. */
      if (zend_hash_index_del(&EG(persistent_list), self->cache_key) == SUCCESS) {
        self->future = nullptr;
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
      self->exception_message = zend_string_init(message, message_len, 0);
      self->exception_code    = rc;

      if (zend_hash_index_del(&EG(persistent_list), self->cache_key) == SUCCESS) {
        self->future = nullptr;
      }

      zend_throw_exception_ex(exception_class(self->exception_code),
                              self->exception_code, "%.*s",
                            (int)ZSTR_LEN(self->exception_message),
                            ZSTR_VAL(self->exception_message));
      return;
    }

    zend_throw_exception_ex(exception_class(rc), rc,
                            "%.*s", (int)message_len, message);
    return;
  }

  ZVAL_COPY(&self->default_session, return_value);
}

ZEND_METHOD(Cassandra_FutureSession, getResource)
{
  ZEND_PARSE_PARAMETERS_NONE();

  auto self = PHP_SCYLLADB_GET_FUTURE_SESSION(ZEND_THIS);

  if (!php_scylladb_future_claim_notifier(Z_OBJ_P(ZEND_THIS), self->reactor_reg)) {
    return;
  }

  /* Resolved already (persistent hit), errored, or degenerate (no future):
     get() won't block, so return an eagerly-readable stream. */
  if (!Z_ISUNDEF(self->default_session) || self->exception_message || self->future == nullptr) {
    php_scylladb_future_get_ready_resource(&self->notifier, &self->notify_stream, return_value);
    return;
  }

  php_scylladb_future_get_resource(self->future, &self->notifier, &self->notify_stream, return_value);
}

ZEND_METHOD(Cassandra_FutureSession, isReady)
{
  ZEND_PARSE_PARAMETERS_NONE();

  auto self = PHP_SCYLLADB_GET_FUTURE_SESSION(ZEND_THIS);
  RETURN_BOOL(!Z_ISUNDEF(self->default_session) || self->exception_message ||
              self->future == nullptr || php_scylladb_future_is_ready(self->future));
}

HashTable *php_scylladb_future_session_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object);

  return props;
}

int php_scylladb_future_session_compare(zval *obj1, zval *obj2)
{
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}
HashTable *php_scylladb_future_session_gc(zend_object *object, zval **table, int *n)
{
    auto self = php_scylladb_future_session_object_fetch(object);
    zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
    zend_get_gc_buffer_add_zval(buffer, &self->default_session);
    zend_get_gc_buffer_add_zval(buffer, &self->default_timeout);
    zend_get_gc_buffer_add_zval(buffer, &self->notify_stream);
    zend_get_gc_buffer_use(buffer, table, n);

    return nullptr;
}


void php_scylladb_future_session_free(zend_object *object)
{
  auto self = php_scylladb_future_session_object_fetch(object);

  if (!self->persist && self->future) {
    cass_future_free(self->future);
  }

  /* Free CassSession only if non-persistent AND not transferred via get(). */
  if (!self->persist && self->session) {
    cass_session_free(self->session);
    self->session = nullptr;
  }

  if (self->exception_message) {
    zend_string_release(self->exception_message);
  }

  if (self->session_keyspace) {
    zend_string_release(self->session_keyspace);
    self->session_keyspace = nullptr;
  }

  zval_ptr_dtor(&self->default_session);
  zval_ptr_dtor(&self->default_timeout);

  if (!Z_ISUNDEF(self->notify_stream)) {
    zval_ptr_dtor(&self->notify_stream);
    ZVAL_UNDEF(&self->notify_stream);
  }
  php_scylladb_notifier_unref(self->notifier);
  self->notifier = nullptr;

  zend_object_std_dtor(&self->zendObject);
}

zend_object *php_scylladb_future_session_new(zend_class_entry *ce)
{
  php_scylladb_future_session *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_future_session, ce, &php_scylladb_future_session_handlers);

  self->session           = nullptr;
  self->future            = nullptr;
  self->exception_message = nullptr;
  self->cache_key         = 0;
  self->persist           = cass_false;
  self->default_consistency = PHP_SCYLLADB_DEFAULT_CONSISTENCY;
  self->default_page_size = PHP_SCYLLADB_DEFAULT_PAGE_SIZE_N;
  self->notifier          = nullptr;
  self->reactor_reg       = nullptr;

  ZVAL_UNDEF(&self->default_session);
  ZVAL_UNDEF(&self->notify_stream);
  ZVAL_UNDEF(&self->default_timeout);
  return &self->zendObject;
}
