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

#include <DateTime/Date.h>
#include <ZendCPP/String/Builder.h>
#include <php.h>
#include <util/hash.h>
#include <util/types.h>

#include <ZendCPP/ZendCPP.hpp>
#include <ctime>

#include "DateTimeInternal.h"

BEGIN_EXTERN_C()

#include <ext/date/php_date.h>

#include "Date_arginfo.h"

zend_class_entry *php_scylladb_date_ce = nullptr;

PHP_SCYLLADB_API php_scylladb_date *php_scylladb_date_instantiate(zval *object) {
  zval val;

  if (object_init_ex(&val, php_scylladb_date_ce) != SUCCESS) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));
  return ZendCPP::ObjectFetch<php_scylladb_date>(object);
}

PHP_SCYLLADB_API zend_result php_scylladb_date_initialize(php_scylladb_date *self,
                                                          zend_string *secondsStr,
                                                          zend_long seconds) {
  cass_int64_t secs = -1;

  if (secondsStr != nullptr) {
    errno = 0;
    char *end = nullptr;
    secs = std::strtol(ZSTR_VAL(secondsStr), &end, 10);

    if ((secs == 0 || secs == LONG_MIN || secs == LONG_MAX) &&
        (end == ZSTR_VAL(secondsStr) || *end != '\0' || errno != 0)) {
      return FAILURE;
    }

  } else if (seconds != -1) {
    secs = seconds;
  }

  self->date = cass_date_from_epoch(secs == -1 ? time(nullptr) : secs);

  return SUCCESS;
}

ZEND_METHOD(Cassandra_Date, __construct) {
  zend_string *secondsStr = nullptr;
  zend_long seconds = -1;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR_OR_LONG(secondsStr, seconds)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (php_scylladb_date_initialize(ZendCPP::ObjectFetch<php_scylladb_date>(getThis()), secondsStr,
                                   seconds) == FAILURE) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "Invalid seconds value: '%s'", ZSTR_VAL(secondsStr));
    RETURN_THROWS();
  }
}
ZEND_METHOD(Cassandra_Date, type) {
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_DATE);
  RETURN_ZVAL(&type, 1, 1);
}

ZEND_METHOD(Cassandra_Date, seconds) {
  auto *self = ZendCPP::ObjectFetch<php_scylladb_date>(getThis());
  RETURN_LONG(cass_date_time_to_epoch(self->date, 0));
}

ZEND_METHOD(Cassandra_Date, toDateTime) {
  zval *ztime = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_OBJECT_OF_CLASS(ztime, php_scylladb_time_ce)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_scylladb_time *time_obj = nullptr;

  if (ztime != nullptr) [[likely]] {
    time_obj = ZendCPP::ObjectFetch<php_scylladb_time>(ztime);
  }

  auto *self = ZendCPP::ObjectFetch<php_scylladb_date>(getThis());

  zval datetime;

  zend_result status = scylladb_php_to_datetime_internal(&datetime, "U", [self, time_obj]() {
    ZendCPP::StringBuilder builder;
    auto str =
        builder
            .Append(cass_date_time_to_epoch(self->date, time_obj != nullptr ? time_obj->time : 0))
            .Build();
    return zend_string_copy(str);
  });

  if (status == FAILURE) [[unlikely]] {
    zend_throw_exception(php_driver_runtime_exception_ce, "Failed to create DateTime object", 0);
    RETURN_THROWS();
  }

  RETURN_ZVAL(&datetime, 0, 1);
}

ZEND_METHOD(Cassandra_Date, fromDateTime) {
  zval *datetime;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(datetime, php_date_get_interface_ce())
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  zval getTimeStampResult = {};
  zend_call_method_with_0_params(Z_OBJ_P(datetime), Z_OBJCE_P(datetime), nullptr, "gettimestamp",
                                 &getTimeStampResult);

  auto self = php_scylladb_date_instantiate(return_value);

  if (self == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zend_throw_exception(php_driver_runtime_exception_ce, "Failed to create Cassandra\\Date object",
                         0);
    RETURN_THROWS();
  }

  self->date = cass_date_from_epoch(Z_LVAL(getTimeStampResult));
  zval_ptr_dtor(&getTimeStampResult);
}

ZEND_METHOD(Cassandra_Date, __toString) {
  ZEND_PARSE_PARAMETERS_NONE();

  auto *self = ZendCPP::ObjectFetch<php_scylladb_date>(getThis());

  char *ret = nullptr;
  spprintf(&ret, 0, PHP_DRIVER_NAMESPACE "\\Date(seconds=%ld)",
           cass_date_time_to_epoch(self->date, 0));
  RETVAL_STRING(ret);
  efree(ret);
}

static php_driver_value_handlers php_scylladb_date_handlers;

static HashTable *php_scylladb_date_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *php_scylladb_date_properties(zend_object *object) {
  HashTable *props = zend_std_get_properties(object);

  auto type = php_driver_type_scalar(CASS_VALUE_TYPE_DATE);
  zend_hash_str_update(props, ZEND_STRL("type"), &type);

  zval seconds;
  auto self = ZendCPP::ObjectFetch<php_scylladb_date>(object);
  ZVAL_LONG(&seconds, cass_date_time_to_epoch(self->date, 0));
  zend_hash_str_update(props, ZEND_STRL("seconds"), &seconds);

  return props;
}

static int php_scylladb_date_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  auto date1 = ZendCPP::ObjectFetch<php_scylladb_date>(obj1);
  auto date2 = ZendCPP::ObjectFetch<php_scylladb_date>(obj2);

  return PHP_DRIVER_COMPARE(date1->date, date2->date);
}

static unsigned php_scylladb_date_hash_value(zval *obj) {
  auto self = ZendCPP::ObjectFetch<php_scylladb_date>(obj);
  return 31 * 17 + self->date;
}

static zend_object *php_scylladb_date_new(zend_class_entry *ce) {
  auto *self = ZendCPP::Allocate<php_scylladb_date>(ce, &php_scylladb_date_handlers);
  self->date = 0;
  return &self->zendObject;
}

void php_driver_define_Date() {
  php_scylladb_date_ce = register_class_Cassandra_Date(php_driver_value_ce);
  php_scylladb_date_ce->create_object = php_scylladb_date_new;

  ZendCPP::InitHandlers<php_scylladb_date>(&php_scylladb_date_handlers);
  php_scylladb_date_handlers.std.get_properties = php_scylladb_date_properties;
  php_scylladb_date_handlers.std.get_gc = php_scylladb_date_gc;
  php_scylladb_date_handlers.std.compare = php_scylladb_date_compare;
  php_scylladb_date_handlers.hash_value = php_scylladb_date_hash_value;
}

END_EXTERN_C()
