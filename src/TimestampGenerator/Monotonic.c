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
#include "php_scylladb_types.h"

#include "Monotonic_arginfo.h"

extern zend_object_handlers php_scylladb_timestamp_generator_monotonic_handlers;

void
php_scylladb_timestamp_generator_monotonic_free(zend_object *object)
{
    php_scylladb_timestamp_gen *self = php_scylladb_timestamp_gen_object_fetch(object);

    cass_timestamp_gen_free(self->gen);

    zend_object_std_dtor(&self->zendObject);
}

zend_object *
php_scylladb_timestamp_generator_monotonic_new(zend_class_entry *ce)
{
    php_scylladb_timestamp_gen *self =
        (php_scylladb_timestamp_gen *)ecalloc(1, sizeof(php_scylladb_timestamp_gen) + zend_object_properties_size(ce));

    self->gen = cass_timestamp_gen_monotonic_new();

    zend_object_std_init(&self->zendObject, ce);
    self->zendObject.handlers = &php_scylladb_timestamp_generator_monotonic_handlers;

    return &self->zendObject;
}

void php_scylladb_timestamp_generator_monotonic_post_register(zend_class_entry *ce)
{
    (void)ce;
    php_scylladb_timestamp_generator_monotonic_handlers.offset = XtOffsetOf(php_scylladb_timestamp_gen, zendObject);
}
