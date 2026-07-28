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

#include <php_scylladb_types.h>
#include <Zend/zend_smart_str.h>
#include <string.h>
#include <stdlib.h>
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"

#include "DateTime/Date.h"
#include "DateTimeInternal.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <ext/date/php_date.h>

#include "Timestamp_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_timestamp_handlers;


PHP_SCYLLADB_API php_scylladb_timestamp *php_scylladb_timestamp_instantiate(zval *object) {
  zval val;

  if (object_init_ex(&val, php_scylladb_timestamp_ce) != SUCCESS) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));
  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(object));
}

typedef struct {
  cass_int64_t seconds;
  cass_int64_t microseconds;
} time_now;

static time_now php_scylladb_time_now(void) {
  cass_int64_t seconds;
  cass_int64_t microseconds;
#if defined(__APPLE__) && defined(__MACH__)
  struct timeval ts = {};
  gettimeofday(&ts, nullptr);
  seconds = (cass_int64_t)ts.tv_sec;
  microseconds = (cass_int64_t)ts.tv_usec;
#else
  struct timespec ts = {};
  clock_gettime(CLOCK_REALTIME, &ts);
  seconds = ts.tv_sec;
  microseconds = ts.tv_nsec / 1000;
#endif
  return (time_now){ seconds, microseconds };
}

PHP_SCYLLADB_API zend_result php_scylladb_timestamp_initialize(php_scylladb_timestamp *object,
                                                               cass_int64_t seconds,
                                                               cass_int64_t microseconds) {
  if (seconds == -1 && microseconds == -1) {
    time_now time = php_scylladb_time_now();
    seconds = time.seconds;
    microseconds = time.microseconds;
  }

  // timestamp is in ms (milliseconds)
  object->timestamp = (seconds * 1000) + microseconds / 1000;
  return SUCCESS;
}

ZEND_METHOD(Cassandra_Timestamp, __construct) {
  zend_long seconds = -1;
  zend_long microseconds = -1;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(seconds)
    Z_PARAM_LONG(microseconds)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_scylladb_timestamp *self =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(getThis()));

  if (php_scylladb_timestamp_initialize(self, seconds, microseconds) != SUCCESS) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                            "Failed to create Timestamp: seconds(%ld) microseconds(%ld)",
                            (long)seconds, (long)microseconds);
    RETURN_THROWS();
  }
}

ZEND_METHOD(Cassandra_Timestamp, type) {
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_TIMESTAMP);
  RETURN_ZVAL(&type, 1, 1);
}

ZEND_METHOD(Cassandra_Timestamp, time) {
  php_scylladb_timestamp *self =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(getThis()));

  RETURN_LONG(self->timestamp / 1000);
}

ZEND_METHOD(Cassandra_Timestamp, microtime) {
  zend_bool get_as_float = false;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(get_as_float)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_scylladb_timestamp *self =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(getThis()));

  if (get_as_float) {
    RETURN_DOUBLE((double)self->timestamp / 1000.00);
  }

  long sec = (long)(self->timestamp / 1000);
  double usec = (double)(((double)self->timestamp - (double)(sec * 1000)) / 1000.00);

  char ret[128];
  memset(ret, 0, sizeof(ret));
  size_t len = snprintf(ret, sizeof(ret) - 1, "%.8F %" PRId64, usec, (int64_t)sec);
  RETURN_STRINGL_FAST(ret, len);
}

typedef struct {
  php_scylladb_timestamp *self;
} timestamp_to_datetime_ctx_t;

static zend_string *timestamp_to_datetime_get_timestamp(void *vctx) {
  timestamp_to_datetime_ctx_t *ctx = (timestamp_to_datetime_ctx_t *)vctx;
  int64_t sec = ctx->self->timestamp / 1000;
  int64_t millisec = (ctx->self->timestamp - (sec * 1000));

  smart_str b = {};
  smart_str_append_long(&b, (zend_long)sec);
  smart_str_appendc(&b, '.');
  smart_str_append_long(&b, (zend_long)millisec);
  smart_str_0(&b);
  return b.s;
}

ZEND_METHOD(Cassandra_Timestamp, toDateTime) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_timestamp *self =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(getThis()));

  timestamp_to_datetime_ctx_t ctx = { .self = self };
  zval datetime;
  zend_result status = scylladb_php_to_datetime_internal(
      &datetime, "U.v", timestamp_to_datetime_get_timestamp, &ctx);

  if (status == FAILURE) {
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to create DateTime object", 0);
    RETURN_THROWS();
  }

  RETURN_ZVAL(&datetime, 1, 1);
}

ZEND_METHOD(Cassandra_Timestamp, fromDateTime) {
  zval *datetime;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(datetime, php_date_get_interface_ce())
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  zval getTimeStampResult = {};
  zval format;
  zend_string *val = zend_string_init_existing_interned(ZEND_STRL("Uv"), false);
  ZVAL_STR(&format, val);

  zval *ret = zend_call_method_with_1_params(Z_OBJ_P(datetime), Z_OBJCE_P(datetime), nullptr,
                                             "format", &getTimeStampResult, &format);

  if (ret == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zval_ptr_dtor(&format);
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to get Timestamp from DateTime",
                         0);
    RETURN_THROWS();
  }

  php_scylladb_timestamp *self = php_scylladb_timestamp_instantiate(return_value);

  if (self == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zval_ptr_dtor(&format);
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to create Cassandra\\Timestamp",
                         0);
    RETURN_THROWS();
  }

  self->timestamp = strtoll(Z_STRVAL(getTimeStampResult), nullptr, 10);
  zval_ptr_dtor(&getTimeStampResult);
  zval_ptr_dtor(&format);
}

ZEND_METHOD(Cassandra_Timestamp, __toString) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_timestamp *self =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(getThis()));

  char ret[32];
  memset(ret, 0, sizeof(ret));
  size_t len = snprintf(ret, sizeof(ret), "%" PRId64, (int64_t)self->timestamp);
  RETURN_STRINGL_FAST(ret, len);
}


HashTable *php_scylladb_timestamp_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return nullptr;
}
HashTable *php_scylladb_timestamp_properties(zend_object *object) {
  php_scylladb_timestamp *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(3);
  HashTable *props = object->properties;

  long sec = (long)(self->timestamp / 1000);
  long usec = (long)((self->timestamp - (sec * 1000)) * 1000);

  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_TIMESTAMP);
  zend_hash_str_update(props, ZEND_STRL("type"), &type);

  zval seconds;
  ZVAL_LONG(&seconds, sec);
  zend_hash_str_update(props, ZEND_STRL("seconds"), &seconds);

  zval microseconds;
  ZVAL_LONG(&microseconds, usec);
  zend_hash_str_update(props, ZEND_STRL("microseconds"), &microseconds);

  return props;
}

int php_scylladb_timestamp_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  php_scylladb_timestamp *timestamp1 =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(obj1));
  php_scylladb_timestamp *timestamp2 =
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(obj2));

  return PHP_SCYLLADB_COMPARE(timestamp1->timestamp, timestamp2->timestamp);
}

unsigned php_scylladb_timestamp_hash_value(zval *obj) {
  return php_scylladb_bigint_hash(
      PHP_SCYLLADB_OBJ_FETCH(php_scylladb_timestamp, Z_OBJ_P(obj))->timestamp);
}

zend_object *php_scylladb_timestamp_new(zend_class_entry *ce) {
  php_scylladb_timestamp *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_timestamp, ce, &php_scylladb_timestamp_handlers);
  self->timestamp = -1;
  return &self->zendObject;
}


void php_scylladb_timestamp_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_timestamp_handlers.std.offset = offsetof(php_scylladb_timestamp, zendObject);
}
