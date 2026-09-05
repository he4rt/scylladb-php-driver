/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "php_scylladb.h"
#include "php_scylladb_types.h"

#include "Cassandra_arginfo.h"

ZEND_METHOD(Cassandra, cluster) {
    object_init_ex(return_value, php_scylladb_cluster_builder_ce);
}

ZEND_METHOD(Cassandra, ssl) {
    object_init_ex(return_value, php_scylladb_ssl_options_builder_ce);
}

static void override_class_constant_string(zend_class_entry *ce, const char *name, const char *value) {
    zend_class_constant *c = (zend_class_constant *)zend_hash_str_find_ptr(&ce->constants_table, name, strlen(name));
    if (c == nullptr) return;
    zval_ptr_dtor(&c->value);
    ZVAL_STR(&c->value, zend_string_init(value, strlen(value), 1));
}

void php_scylladb_core_post_register(zend_class_entry *ce)
{
    /* VERSION is now wired in the stub via @cvalue PHP_SCYLLADB_VERSION.
     * CPP_DRIVER_VERSION still needs runtime composition because
     * CASS_VERSION_SUFFIX may be empty and we don't want a trailing dash. */
    char buf[64];
    const char *suffix = CASS_VERSION_SUFFIX;
    snprintf(buf, sizeof(buf), "%d.%d.%d%s%s",
             CASS_VERSION_MAJOR, CASS_VERSION_MINOR, CASS_VERSION_PATCH,
             suffix[0] == '\0' ? "" : "-", suffix);
    override_class_constant_string(ce, "CPP_DRIVER_VERSION", buf);
}
