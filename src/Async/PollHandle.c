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

/*
 * Cassandra\Async\PollHandle — a driver completion fd as an Io\Poll\Handle.
 *
 * PHP 8.6 added an epoll/kqueue/event-port polling API (RFC: poll_api) plus an
 * extension-facing handle interface: a class registers php_poll_handle_ops and
 * Io\Poll\Context::add() pulls the descriptor out at C level. This file is
 * compiled only when the PHP being built against ships main/php_poll.h — see
 * PHP_SCYLLADB_ENABLE_POLL_API in the root CMakeLists.txt.
 *
 * Io\Poll\Handle rejects user classes (interface_gets_implemented), but not
 * internal ones, so the class registry resolves the interface by FQN the same
 * way it resolves an SPL parent.
 *
 * Against the existing stream path this saves a dup() and a php_stream per
 * watched future: the handle holds a notifier reference and reports that
 * notifier's own read end. It never owns the descriptor — the notifier closes
 * it on its last unref, which cannot happen while a handle holds a reference.
 *
 * The completion protocol does not change. The descriptor is level-triggered
 * and the consumer still calls Reactor::poll() (drains the fd) or Future::get()
 * on a readable event. Event::EdgeTriggered works too, because both the
 * eventfd counter and the pipe write produce a fresh edge per completion.
 */

#include "php_scylladb.h"
#include "php_scylladb_types.h"

#include <php_poll.h>

#include "FutureNotifier.h"
#include "Async/PollHandle.h"
#include "Async/Reactor.h"

#include <Registry/Registry.h>

#include "Async/PollHandle_arginfo.h"

zend_class_entry* php_scylladb_async_poll_handle_ce = nullptr;

static zend_object_handlers php_scylladb_async_poll_handle_handlers;

/* ─── php_poll_handle_ops ────────────────────────────────────────────────────
 * handle_data is the php_scylladb_notifier the handle holds a reference to. */

static php_socket_t
poll_handle_get_fd(php_poll_handle_object* handle)
{
  int fd = php_scylladb_notifier_fd((const php_scylladb_notifier*)handle->handle_data);
  return fd < 0 ? SOCK_ERR : (php_socket_t)fd;
}

static int
poll_handle_is_valid(php_poll_handle_object* handle)
{
  return poll_handle_get_fd(handle) != SOCK_ERR;
}

static void
poll_handle_cleanup(php_poll_handle_object* handle)
{
  php_scylladb_notifier_unref((php_scylladb_notifier*)handle->handle_data);
  handle->handle_data = nullptr;
}

static php_poll_handle_ops poll_handle_ops = {
  .get_fd   = poll_handle_get_fd,
  .is_valid = poll_handle_is_valid,
  .cleanup  = poll_handle_cleanup,
};

static zend_object*
php_scylladb_async_poll_handle_new(zend_class_entry* ce)
{
  php_poll_handle_object* intern =
      php_poll_handle_object_create(sizeof(php_poll_handle_object), ce, &poll_handle_ops);
  return &intern->std;
}

/* Instantiate a handle over `notifier`, taking a reference for the object. */
static void
poll_handle_instantiate(php_scylladb_notifier* notifier, zval* return_value)
{
  object_init_ex(return_value, php_scylladb_async_poll_handle_ce);
  php_scylladb_notifier_addref(notifier);
  PHP_POLL_HANDLE_OBJ_FROM_ZV(return_value)->handle_data = notifier;
}

/* ─── Cassandra\Async\PollHandle ─────────────────────────────────────────────*/

ZEND_METHOD(Cassandra_Async_PollHandle, __construct)
{
}

ZEND_METHOD(Cassandra_Async_PollHandle, reactor)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_notifier* notifier = nullptr;
  zval*                  slot     = nullptr;

  if (!php_scylladb_reactor_poll_slots(&notifier, &slot)) {
    return;
  }

  if (Z_ISUNDEF_P(slot)) {
    poll_handle_instantiate(notifier, slot);
  }

  RETURN_COPY(slot);
}

int
php_scylladb_poll_handle_for_future(zval* future_zv, zval* return_value)
{
  php_scylladb_future_slots slots;

  if (!php_scylladb_future_slots_get(future_zv, &slots) ||
      !php_scylladb_future_claim_notifier(Z_OBJ_P(future_zv), *slots.reactor_reg)) {
    return FAILURE;
  }

  int rc = slots.future == nullptr
               ? php_scylladb_future_ensure_ready_notifier(slots.notifier)
               : php_scylladb_future_ensure_notifier(slots.future, slots.notifier);
  if (rc == FAILURE) {
    return FAILURE;
  }

  poll_handle_instantiate(*slots.notifier, return_value);
  return SUCCESS;
}

ZEND_METHOD(Cassandra_Async_PollHandle, forFuture)
{
  zval* future_zv = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(future_zv, php_scylladb_future_ce)
  ZEND_PARSE_PARAMETERS_END();

  php_scylladb_poll_handle_for_future(future_zv, return_value);
}

ZEND_METHOD(Cassandra_Async_PollHandle, getFileDescriptor)
{
  ZEND_PARSE_PARAMETERS_NONE();

  php_socket_t fd = poll_handle_get_fd(PHP_POLL_HANDLE_OBJ_FROM_ZV(ZEND_THIS));
  RETURN_LONG(fd == SOCK_ERR ? -1 : (zend_long)fd);
}

ZEND_METHOD(Cassandra_Async_PollHandle, isValid)
{
  ZEND_PARSE_PARAMETERS_NONE();

  RETURN_BOOL(poll_handle_is_valid(PHP_POLL_HANDLE_OBJ_FROM_ZV(ZEND_THIS)));
}

/* ─── registration ───────────────────────────────────────────────────────────
 * Hand-written rather than generated: the object layout is php-src's
 * php_poll_handle_object, so the descriptor generator's `offsetof(..., zendObject)`
 * does not apply. deps[0] is Io\Poll\Handle, resolved by the registry through
 * zend_lookup_class (ext/standard registers it well before a shared extension
 * starts up). */

static zend_class_entry*
php_scylladb_register_async_poll_handle(zend_class_entry* const* deps)
{
  zend_class_entry* ce = register_class_Cassandra_Async_PollHandle(deps[0]);
  ce->create_object    = php_scylladb_async_poll_handle_new;

  memcpy(&php_scylladb_async_poll_handle_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_scylladb_async_poll_handle_handlers.offset    = offsetof(php_poll_handle_object, std);
  php_scylladb_async_poll_handle_handlers.free_obj  = php_poll_handle_object_free;
  php_scylladb_async_poll_handle_handlers.clone_obj = nullptr;
  ce->default_object_handlers = &php_scylladb_async_poll_handle_handlers;

  return ce;
}

PHP_SCYLLADB_REGISTER_CLASS(async_poll_handle,
                            "Cassandra\\Async\\PollHandle",
                            &php_scylladb_async_poll_handle_ce,
                            "Io\\Poll\\Handle",
                            php_scylladb_register_async_poll_handle)
