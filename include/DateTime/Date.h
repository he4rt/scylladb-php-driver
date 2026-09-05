#pragma once

#include <api.h>
#include <cassandra.h>
#include <php.h>
#include <php_scylladb_object.h>

BEGIN_EXTERN_C()
typedef struct {
  cass_uint32_t date;
  zend_object zendObject;
} php_scylladb_date;

typedef struct {
  cass_int64_t time;
  zend_object zendObject;
} php_scylladb_time;

typedef struct {
  cass_int64_t timestamp;
  zend_object zendObject;
} php_scylladb_timestamp;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_date_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_time_ce;

[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API php_scylladb_date *php_scylladb_date_instantiate(zval *object);
/* secondsStr is optional (use the long alternative); only `object` is required. */
[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API zend_result php_scylladb_date_initialize(php_scylladb_date *object,
                                                                        zend_string *secondsStr,
                                                                        zend_long seconds,
                                                                        bool provided);

[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API php_scylladb_time *php_scylladb_time_instantiate(zval *object);
[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API zend_result php_scylladb_time_initialize(php_scylladb_time *object,
                                                                        zend_string *nanosecondsStr,
                                                                        zend_long nanoseconds,
                                                                        bool provided);

[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API php_scylladb_timestamp *php_scylladb_timestamp_instantiate(zval *object);
[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API zend_result php_scylladb_timestamp_initialize(php_scylladb_timestamp *object,
                                                                             cass_int64_t seconds,
                                                                             cass_int64_t microseconds);

static zend_always_inline php_scylladb_date *php_scylladb_date_from_obj(zend_object *obj) {
  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, obj);
}

static zend_always_inline php_scylladb_time *php_scylladb_time_from_obj(zend_object *obj) {
  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, obj);
}

static zend_always_inline php_scylladb_timestamp *php_scylladb_timestamp_from_obj(zend_object *obj) {
  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, obj);
}

#define Z_SCYLLADB_DATE_P(zv) php_scylladb_date_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_DATE(zv) php_scylladb_date_from_obj(Z_OBJ((zv)))

#define Z_SCYLLADB_TIME_P(zv) php_scylladb_time_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_TIME(zv) php_scylladb_time_from_obj(Z_OBJ((zv)))

#define Z_SCYLLADB_TIMESTAMP_P(zv) php_scylladb_timestamp_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_TIMESTAMP(zv) php_scylladb_timestamp_from_obj(Z_OBJ((zv)))

END_EXTERN_C()
