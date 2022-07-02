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

#include "include/TimestampGenerators/TimestampGenerators.h"
#include <CassandraDriverAPI.h>
#include <cassandra.h>
#include <php.h>

#include "ServerSideGenerator.h"
#include "ServerSideGenerator_arginfo.h"

PHP_DRIVER_API zend_class_entry* phpDriverTimestampGenServerSideCe = NULL;

static zend_object_handlers phpDriverTimestampGenServerSideHandlers;

static void
PhpDriverTimestampGenServerSideFree(zend_object* object)
{
  php_driver_timestamp_gen* self = PHP_DRIVER_TIMESTAMP_GENERATOR_OBJECT(object);

  if (self->gen != NULL) {
    cass_timestamp_gen_free(self->gen);
  }

  zend_object_std_dtor(&self->zval);
}

static zend_object*
PhpDriverTimestampGenServerSideNew(zend_class_entry* ce)
{
  php_driver_timestamp_gen* self = make(php_driver_timestamp_gen);

  self->gen = cass_timestamp_gen_server_side_new();

  zend_object_std_init(&self->zval, ce);

  self->zval.handlers = &phpDriverTimestampGenServerSideHandlers;

  return &self->zval;
}

void
PhpDriverDefineServerSideTimestampGenerator(zend_class_entry* timestampGeneratorInterface)
{
  phpDriverTimestampGenServerSideCe                = register_class_Cassandra_TimestampGenerators_ServerSideGenerator(timestampGeneratorInterface);
  phpDriverTimestampGenServerSideCe->create_object = PhpDriverTimestampGenServerSideNew;

  memcpy(&phpDriverTimestampGenServerSideHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  phpDriverTimestampGenServerSideHandlers.free_obj = PhpDriverTimestampGenServerSideFree;
  phpDriverTimestampGenServerSideHandlers.offset   = XtOffsetOf(php_driver_timestamp_gen, zval);
}
