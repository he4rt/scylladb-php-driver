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

#include <Futures/FutureClose.h>
#include <cassandra_driver.h>

#include "util/FutureInterface.h"

#include "FutureClose_arginfo.h"

zend_class_entry* php_driver_future_close_ce = NULL;

ZEND_METHOD(Cassandra_FutureClose, get)
{
  zval* timeout                 = NULL;
  php_driver_future_close* self = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &timeout) == FAILURE)
    return;

  self = PHP_DRIVER_FUTURE_CLOSE_THIS();

  if (php_driver_future_wait_timed(self->future, timeout) == FAILURE)
    return;

  if (php_driver_future_is_error(self->future) == FAILURE)
    return;
}

static zend_object_handlers php_driver_future_close_handlers;

static HashTable*
php_driver_future_close_properties(zend_object* object)
{
  HashTable* props = zend_std_get_properties(object);

  return props;
}

static int
php_driver_future_close_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_future_close_free(zend_object* object)
{
  php_driver_future_close* self = PHP_DRIVER_FUTURE_CLOSE_OBJECT(object);

  if (self->future) {
    cass_future_free(self->future);
  }

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_future_close_new(zend_class_entry* ce)
{
  php_driver_future_close* self = make(php_driver_future_close);

  self->future = NULL;

  zend_object_std_init(&self->zval, ce);

  self->zval.handlers = &php_driver_future_close_handlers;

  return &self->zval;
}

void
php_driver_define_FutureClose(zend_class_entry* future_interface)
{
  php_driver_future_close_ce                = register_class_Cassandra_FutureClose(future_interface);
  php_driver_future_close_ce->create_object = php_driver_future_close_new;

  memcpy(&php_driver_future_close_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_future_close_handlers.get_properties = php_driver_future_close_properties;
  php_driver_future_close_handlers.compare        = php_driver_future_close_compare;
  php_driver_future_close_handlers.clone_obj      = NULL;
  php_driver_future_close_handlers.free_obj       = php_driver_future_close_free;
  php_driver_future_close_handlers.offset         = XtOffsetOf(php_driver_future_close, zval);
}
