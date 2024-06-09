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

#include <ZendCPP/String/Builder.h>
#include <php_driver_types.h>
#include <util/hash.h>
#include <util/types.h>

#include <ZendCPP/ZendCPP.hpp>

#include "DateTime/Date.h"
#include "DateTimeInternal.h"

BEGIN_EXTERN_C()
#include <ext/date/php_date.h>

#include "Timestamp_arginfo.h"

zend_class_entry *php_scylladb_timestamp_ce = nullptr;

PHP_SCYLLADB_API php_scylladb_timestamp *php_scylladb_timestamp_instantiate(zval *object) {
  zval val;

  if (object_init_ex(&val, php_scylladb_timestamp_ce) != SUCCESS) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));
  return ZendCPP::ObjectFetch<php_scylladb_timestamp>(object);
}

typedef struct {
  cass_int64_t seconds;
  cass_int64_t microseconds;
} time_now;

static time_now php_driver_time_now() {
  cass_int64_t seconds;
  cass_int64_t microseconds;
#if defined(__APPLE__) && defined(__MACH__)
  struct timeval ts {};
  gettimeofday(&ts, NULL);
  seconds = (cass_int64_t)ts.tv_sec;
  microseconds = (cass_int64_t)ts.tv_usec;
#else
  timespec ts{};
  clock_gettime(CLOCK_REALTIME, &ts);
  seconds = ts.tv_sec;
  microseconds = ts.tv_nsec / 1000;
#endif
  return time_now{seconds, microseconds};
}

PHP_SCYLLADB_API zend_result php_scylladb_timestamp_initialize(php_scylladb_timestamp *object,
                                                               cass_int64_t seconds,
                                                               cass_int64_t microseconds) {
  if (seconds == -1 && microseconds == -1) {
    auto time = php_driver_time_now();
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

  auto self = ZendCPP::ObjectFetch<php_scylladb_timestamp>(getThis());

  if (php_scylladb_timestamp_initialize(self, seconds, microseconds) != SUCCESS) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "Failed to create Timestamp: seconds(%ld) microseconds(%ld)", seconds,
                            microseconds);
    RETURN_THROWS();
  }
}

ZEND_METHOD(Cassandra_Timestamp, type) {
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TIMESTAMP);
  RETURN_ZVAL(&type, 1, 1);
}

ZEND_METHOD(Cassandra_Timestamp, time) {
  auto *self = ZendCPP::ObjectFetch<php_scylladb_timestamp>(getThis());

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

  auto self = ZendCPP::ObjectFetch<php_scylladb_timestamp>(getThis());

  if (get_as_float) {
    RETURN_DOUBLE((double)self->timestamp / 1000.00);
  }

  auto sec = (long)(self->timestamp / 1000);
  auto usec = (double)(((double)self->timestamp - (double)(sec * 1000)) / 1000.00);

  char ret[128];
  memset(ret, 0, sizeof(ret));
  size_t len = snprintf(ret, sizeof(ret) - 1, "%.8F %" PRId64, usec, sec);
  RETURN_STRINGL_FAST(ret, len);
}

ZEND_METHOD(Cassandra_Timestamp, toDateTime) {
  ZEND_PARSE_PARAMETERS_NONE();

  auto self = ZendCPP::ObjectFetch<php_scylladb_timestamp>(getThis());

  zval datetime;
  zend_result status = scylladb_php_to_datetime_internal(&datetime, "U.v", [self]() {
    int64_t sec = self->timestamp / 1000;
    int64_t millisec = (self->timestamp - (sec * 1000));

    ZendCPP::StringBuilder builder(32);

    auto str = builder.Append(sec).Append('.').Append(millisec).Build();
    return zend_string_copy(str);
  });

  if (status == FAILURE) [[unlikely]] {
    zend_throw_exception(php_driver_runtime_exception_ce, "Failed to create DateTime object", 0);
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

  zval getTimeStampResult{};
  zval format;
  auto val = zend_string_init_existing_interned(ZEND_STRL("Uv"), false);
  ZVAL_STR(&format, val);

  auto ret = zend_call_method_with_1_params(Z_OBJ_P(datetime), Z_OBJCE_P(datetime), nullptr,
                                            "format", &getTimeStampResult, &format);

  if (ret == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zval_ptr_dtor(&format);
    zend_throw_exception(php_driver_runtime_exception_ce, "Failed to get Timestamp from DateTime",
                         0);
    RETURN_THROWS();
  };

  auto self = php_scylladb_timestamp_instantiate(return_value);

  if (self == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zval_ptr_dtor(&format);
    zend_throw_exception(php_driver_runtime_exception_ce, "Failed to create Cassandra\\Timestamp",
                         0);
    RETURN_THROWS();
  }

  self->timestamp = std::strtoll(Z_STRVAL(getTimeStampResult), nullptr, 10);
  zval_ptr_dtor(&getTimeStampResult);
  zval_ptr_dtor(&format);
}

ZEND_METHOD(Cassandra_Timestamp, __toString) {
  ZEND_PARSE_PARAMETERS_NONE();

  auto *self = ZendCPP::ObjectFetch<php_scylladb_timestamp>(getThis());

  char ret[32];
  memset(ret, 0, sizeof(ret));
  size_t len = snprintf(ret, sizeof(ret), "%" PRId64, (int64_t)self->timestamp);
  RETURN_STRINGL_FAST(ret, len);
}

static php_driver_value_handlers php_driver_timestamp_handlers;

static HashTable *php_driver_timestamp_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return zend_std_get_properties(object);
}
static HashTable *php_driver_timestamp_properties(zend_object *object) {
  auto self = ZendCPP::ObjectFetch<php_scylladb_timestamp>(object);
  HashTable *props = zend_std_get_properties(object);

  long sec = (long)(self->timestamp / 1000);
  long usec = (long)((self->timestamp - (sec * 1000)) * 1000);

  auto type = php_driver_type_scalar(CASS_VALUE_TYPE_TIMESTAMP);
  zend_hash_str_update(props, ZEND_STRL("type"), &type);

  zval seconds;
  ZVAL_LONG(&seconds, sec);
  zend_hash_str_update(props, ZEND_STRL("seconds"), &seconds);

  zval microseconds;
  ZVAL_LONG(&microseconds, usec);
  zend_hash_str_update(props, ZEND_STRL("microseconds"), &microseconds);

  return props;
}

static int php_driver_timestamp_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  auto timestamp1 = ZendCPP::ObjectFetch<php_scylladb_timestamp>(obj1);
  auto timestamp2 = ZendCPP::ObjectFetch<php_scylladb_timestamp>(obj2);

  return PHP_DRIVER_COMPARE(timestamp1->timestamp, timestamp2->timestamp);
}

static unsigned php_driver_timestamp_hash_value(zval *obj) {
  return php_driver_bigint_hash(ZendCPP::ObjectFetch<php_scylladb_timestamp>(obj)->timestamp);
}

static zend_object *php_driver_timestamp_new(zend_class_entry *ce) {
  auto self = ZendCPP::Allocate<php_scylladb_timestamp>(ce, &php_driver_timestamp_handlers);
  self->timestamp = -1;
  return &self->zendObject;
}

void php_driver_define_Timestamp() {
  php_scylladb_timestamp_ce = register_class_Cassandra_Timestamp(php_driver_value_ce);
  php_scylladb_timestamp_ce->create_object = php_driver_timestamp_new;

  ZendCPP::InitHandlers<php_scylladb_timestamp>(&php_driver_timestamp_handlers);
  php_driver_timestamp_handlers.std.get_properties = php_driver_timestamp_properties;
  php_driver_timestamp_handlers.std.get_gc = php_driver_timestamp_gc;
  php_driver_timestamp_handlers.std.compare = php_driver_timestamp_compare;
  php_driver_timestamp_handlers.hash_value = php_driver_timestamp_hash_value;
}
END_EXTERN_C()