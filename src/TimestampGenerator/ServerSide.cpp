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

BEGIN_EXTERN_C()
#include "ServerSide_arginfo.h"

zend_class_entry *php_driver_timestamp_gen_server_side_ce = NULL;

static zend_object_handlers php_driver_timestamp_gen_server_side_handlers;

static void
php_driver_timestamp_gen_server_side_free(zend_object *object)
{
    php_driver_timestamp_gen *self = php_driver_timestamp_gen_object_fetch(object);

    cass_timestamp_gen_free(self->gen);

    zend_object_std_dtor(&self->zendObject);
}

static zend_object *
php_driver_timestamp_gen_server_side_new(zend_class_entry *ce)
{
    php_driver_timestamp_gen *self =
        (php_driver_timestamp_gen *)ecalloc(1, sizeof(php_driver_timestamp_gen) + zend_object_properties_size(ce));

    self->gen = cass_timestamp_gen_server_side_new();

    zend_object_std_init(&self->zendObject, ce);
    self->zendObject.handlers = &php_driver_timestamp_gen_server_side_handlers;

    return &self->zendObject;
}

void php_driver_define_TimestampGeneratorServerSide()
{
    php_driver_timestamp_gen_server_side_ce =
        register_class_Cassandra_TimestampGenerator_ServerSide(php_driver_timestamp_gen_ce);
    php_driver_timestamp_gen_server_side_ce->create_object = php_driver_timestamp_gen_server_side_new;

    memcpy(&php_driver_timestamp_gen_server_side_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_timestamp_gen_server_side_handlers.offset   = XtOffsetOf(php_driver_timestamp_gen, zendObject);
    php_driver_timestamp_gen_server_side_handlers.free_obj = php_driver_timestamp_gen_server_side_free;
}
END_EXTERN_C()
