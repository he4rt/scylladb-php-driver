/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "php_driver.h"
#include "php_driver_types.h"

BEGIN_EXTERN_C()
#include "Cassandra_arginfo.h"

zend_class_entry *php_driver_core_ce = NULL;

ZEND_METHOD(Cassandra, cluster) {
    object_init_ex(return_value, php_driver_cluster_builder_ce);
}

ZEND_METHOD(Cassandra, ssl) {
    object_init_ex(return_value, php_scylladb_ssl_builder_ce);
}

static void override_class_constant_string(zend_class_entry *ce, const char *name, const char *value) {
    zend_class_constant *c = (zend_class_constant *)zend_hash_str_find_ptr(&ce->constants_table, name, strlen(name));
    if (c == NULL) return;
    zval_ptr_dtor(&c->value);
    ZVAL_STR(&c->value, zend_string_init(value, strlen(value), 1));
}

void php_driver_define_Core() {
    php_driver_core_ce = register_class_Cassandra();

    /* CPP_DRIVER_VERSION is runtime-computed from the linked driver. The stub
       has a placeholder; override it here with the real value before any
       user code can observe it. */
    char buf[64];
    snprintf(buf, sizeof(buf), "%d.%d.%d%s",
             CASS_VERSION_MAJOR, CASS_VERSION_MINOR, CASS_VERSION_PATCH,
             strlen(CASS_VERSION_SUFFIX) > 0 ? "-" CASS_VERSION_SUFFIX : "");
    override_class_constant_string(php_driver_core_ce, "CPP_DRIVER_VERSION", buf);

    /* VERSION constant set to PHP_DRIVER_VERSION (overrides stub placeholder). */
    override_class_constant_string(php_driver_core_ce, "VERSION", PHP_DRIVER_VERSION);
}
END_EXTERN_C()
