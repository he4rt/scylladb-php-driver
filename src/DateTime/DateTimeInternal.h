#pragma once

#include <php.h>

/* Callback that produces the formatted timestamp string. Receives an opaque
 * context cookie and returns a fresh zend_string the caller owns (the helper
 * releases it). */
typedef zend_string *(*scylladb_get_timestamp_fn)(void *ctx);

zend_result scylladb_php_to_datetime_internal(
    zval *dst, const char *format,
    scylladb_get_timestamp_fn get_timestamp, void *ctx);
