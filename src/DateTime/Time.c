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

#include "Type/ValueHash.h"
#include "Numbers/NumberParser.h"
#include "Type/TypeFactory.h"

#include "DateTime/Date.h"
#include "php_scylladb.h"
#include "php_scylladb_types.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/time.h>
#else
#include <time.h>
#endif

#define NUM_NANOSECONDS_PER_DAY 86399999999999LL
#define NANOSECONDS_PER_SECOND 1000000000LL

static cass_int64_t php_scylladb_time_now_ns(void) {
  cass_int64_t seconds;
  cass_int64_t nanoseconds;
#if defined(__APPLE__) && defined(__MACH__)
  struct timeval ts = {};
  gettimeofday(&ts, nullptr);
  seconds = (cass_int64_t)ts.tv_sec;
  nanoseconds = (cass_int64_t)ts.tv_usec * 1000;
#else
  struct timespec ts = {};
  clock_gettime(CLOCK_REALTIME, &ts);
  seconds = (cass_int64_t)ts.tv_sec;
  nanoseconds = (cass_int64_t)ts.tv_nsec;
#endif
  return cass_time_from_epoch(seconds) + nanoseconds;
}


#include <ext/date/lib/timelib.h>
#include <ext/date/php_date.h>

#include "Time_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_time_handlers;


static int to_string(zval *result, php_scylladb_time *time) {
  ZVAL_STR(result, zend_strpprintf(0, "%" PRId64, (int64_t)time->time));
  return SUCCESS;
}

PHP_SCYLLADB_API php_scylladb_time *php_scylladb_time_instantiate(zval *object) {
  zval val;

  if (object_init_ex(&val, php_scylladb_time_ce) == FAILURE) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));

  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(object));
}

PHP_SCYLLADB_API zend_result php_scylladb_time_initialize(php_scylladb_time *self,
                                                          zend_string *nanosecondsStr,
                                                          zend_long nanoseconds, bool provided) {
  if (!provided) {
    self->time = php_scylladb_time_now_ns();
    return SUCCESS;
  }

  cass_int64_t value = (cass_int64_t)nanoseconds;

  if (nanosecondsStr != nullptr) {
    if (!php_scylladb_parse_bigint(nanosecondsStr, &value)) {
      zval zNanoseconds;
      ZVAL_STR(&zNanoseconds, nanosecondsStr);
      throw_invalid_argument(&zNanoseconds, "nanoseconds",
                             "invalid string representation of a number of nanoseconds");
      return FAILURE;
    }

    if (value < 0 || value > NUM_NANOSECONDS_PER_DAY) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "nanoseconds must be nanoseconds since midnight, '%.*s' given",
                              (int)ZSTR_LEN(nanosecondsStr), ZSTR_VAL(nanosecondsStr));
      return FAILURE;
    }

    self->time = value;
    return SUCCESS;
  }

  if (value < 0 || value > NUM_NANOSECONDS_PER_DAY) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                            "nanoseconds must be nanoseconds since midnight, " ZEND_LONG_FMT
                            " given",
                            nanoseconds);
    return FAILURE;
  }

  self->time = value;
  return SUCCESS;
}

ZEND_METHOD(Cassandra_Time, __construct) {
  zend_string *nanosecondsStr = nullptr;
  zend_long nanoseconds = -1;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR_OR_LONG(nanosecondsStr, nanoseconds)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_scylladb_time *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(ZEND_THIS));

  if (php_scylladb_time_initialize(self, nanosecondsStr, nanoseconds, ZEND_NUM_ARGS() != 0) ==
      FAILURE) {
    RETURN_THROWS();
  }
}

ZEND_METHOD(Cassandra_Time, type) {
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_TIME);
  RETURN_ZVAL(&type, 1, 1);
}

ZEND_METHOD(Cassandra_Time, seconds) {
  php_scylladb_time *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(ZEND_THIS));
  RETURN_LONG(self->time / NANOSECONDS_PER_SECOND);
}

ZEND_METHOD(Cassandra_Time, fromDateTime) {
  zval *datetime = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(datetime, php_date_get_interface_ce())
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  zval getTimeStampResult;
  if (zend_call_method_with_0_params(Z_OBJ_P(datetime), Z_OBJCE_P(datetime), nullptr,
                                     "getTimestamp", &getTimeStampResult) == nullptr) {
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to get timestamp from DateTime",
                         0);
    RETURN_THROWS();
  }

  php_scylladb_time *self = php_scylladb_time_instantiate(return_value);

  if (self == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to create Cassandra\\Time", 0);
    RETURN_THROWS();
  }

  /* Extract nanoseconds since midnight from a unix timestamp.
     getTimestamp() returns whole seconds; multiply modulo-day by 1e9 for ns. */
  zend_long unix_sec = Z_LVAL(getTimeStampResult);
  self->time = (unix_sec % 86400) * 1000000000LL;
  zval_ptr_dtor(&getTimeStampResult);
}

ZEND_METHOD(Cassandra_Time, __toString) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_time *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(ZEND_THIS));
  to_string(return_value, self);
}


HashTable *php_scylladb_time_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *php_scylladb_time_properties(zend_object *object) {
  HashTable *props = php_scylladb_properties_rebuild(object, 2);

  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_TIME);
  zend_hash_str_update(props, ZEND_STRL("type"), &type);

  zval nanoseconds;
  to_string(&nanoseconds, PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, object));
  zend_hash_str_update(props, ZEND_STRL("nanoseconds"), &nanoseconds);

  return props;
}

int php_scylladb_time_compare(zval *obj1, zval *obj2) {
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  php_scylladb_time *time1 = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(obj1));
  php_scylladb_time *time2 = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(obj2));

  return PHP_SCYLLADB_COMPARE(time1->time, time2->time);
}

unsigned php_scylladb_time_hash_value(zval *obj) {
  php_scylladb_time *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(obj));
  return php_scylladb_bigint_hash(self->time);
}

zend_object *php_scylladb_time_new(zend_class_entry *ce) {
  php_scylladb_time *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_time, ce, &php_scylladb_time_handlers);
  self->time = -1;
  return &self->zendObject;
}



void php_scylladb_time_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_time_handlers.std.offset = offsetof(php_scylladb_time, zendObject);
}
