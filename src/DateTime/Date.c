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
#include <php.h>
#include <php_scylladb_types.h>
#include <Zend/zend_smart_str.h>
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"

#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include "Numbers/NumberParser.h"

#include "DateTimeInternal.h"

#include <ext/date/php_date.h>

#include "Date_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_date_handlers;


PHP_SCYLLADB_API php_scylladb_date *php_scylladb_date_instantiate(zval *object) {
  zval val;

  if (object_init_ex(&val, php_scylladb_date_ce) != SUCCESS) {
    return nullptr;
  }

  ZVAL_OBJ(object, Z_OBJ(val));
  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(object));
}

PHP_SCYLLADB_API zend_result php_scylladb_date_initialize(php_scylladb_date *self,
                                                          zend_string *secondsStr,
                                                          zend_long seconds, bool provided) {
  if (!provided) {
    self->date = cass_date_from_epoch(time(nullptr));
    return SUCCESS;
  }

  cass_int64_t secs = (cass_int64_t)seconds;

  if (secondsStr != nullptr && !php_scylladb_parse_bigint(secondsStr, &secs)) {
    return FAILURE;
  }

  self->date = cass_date_from_epoch(secs);

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

  if (php_scylladb_date_initialize(PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(ZEND_THIS)),
                                   secondsStr, seconds, ZEND_NUM_ARGS() != 0) == FAILURE) {
    RETURN_THROWS();
  }
}
ZEND_METHOD(Cassandra_Date, type) {
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_DATE);
  RETURN_ZVAL(&type, 1, 1);
}

ZEND_METHOD(Cassandra_Date, seconds) {
  php_scylladb_date *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(ZEND_THIS));
  RETURN_LONG(cass_date_time_to_epoch(self->date, 0));
}

typedef struct {
  php_scylladb_date *self;
  php_scylladb_time *time_obj;
} date_to_datetime_ctx_t;

static zend_string *date_to_datetime_get_timestamp(void *vctx) {
  date_to_datetime_ctx_t *ctx = (date_to_datetime_ctx_t *)vctx;
  smart_str b = {};
  smart_str_append_long(
      &b,
      (zend_long)cass_date_time_to_epoch(
          ctx->self->date, ctx->time_obj != nullptr ? ctx->time_obj->time : 0));
  smart_str_0(&b);
  return b.s;
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

  if (ztime != nullptr) {
    time_obj = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_time, Z_OBJ_P(ztime));
  }

  php_scylladb_date *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(ZEND_THIS));

  date_to_datetime_ctx_t ctx = { .self = self, .time_obj = time_obj };
  zval datetime;
  zend_result status = scylladb_php_to_datetime_internal(
      &datetime, "U", date_to_datetime_get_timestamp, &ctx);

  if (status == FAILURE) {
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to create DateTime object", 0);
    RETURN_THROWS();
  }

  RETURN_ZVAL(&datetime, 0, 1);
}

ZEND_METHOD(Cassandra_Date, fromDateTime) {
  zval *datetime = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(datetime, php_date_get_interface_ce())
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  zval getTimeStampResult = {};
  zend_call_method_with_0_params(Z_OBJ_P(datetime), Z_OBJCE_P(datetime), nullptr, "gettimestamp",
                                 &getTimeStampResult);

  php_scylladb_date *self = php_scylladb_date_instantiate(return_value);

  if (self == nullptr) {
    zval_ptr_dtor(&getTimeStampResult);
    zend_throw_exception(php_scylladb_runtime_exception_ce, "Failed to create Cassandra\\Date object",
                         0);
    RETURN_THROWS();
  }

  self->date = cass_date_from_epoch(Z_LVAL(getTimeStampResult));
  zval_ptr_dtor(&getTimeStampResult);
}

ZEND_METHOD(Cassandra_Date, __toString) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_scylladb_date *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(ZEND_THIS));

  RETVAL_STR(zend_strpprintf(0, PHP_SCYLLADB_NAMESPACE "\\Date(seconds=%" PRId64 ")",
                             (int64_t)cass_date_time_to_epoch(self->date, 0)));
}


HashTable *php_scylladb_date_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *php_scylladb_date_properties(zend_object *object) {
  HashTable *props = php_scylladb_properties_rebuild(object, 2);

  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_DATE);
  zend_hash_str_update(props, ZEND_STRL("type"), &type);

  zval seconds;
  php_scylladb_date *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, object);
  ZVAL_LONG(&seconds, cass_date_time_to_epoch(self->date, 0));
  zend_hash_str_update(props, ZEND_STRL("seconds"), &seconds);

  return props;
}

int php_scylladb_date_compare(zval *obj1, zval *obj2) {
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  php_scylladb_date *date1 = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(obj1));
  php_scylladb_date *date2 = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(obj2));

  return PHP_SCYLLADB_COMPARE(date1->date, date2->date);
}

unsigned php_scylladb_date_hash_value(zval *obj) {
  php_scylladb_date *self = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_date, Z_OBJ_P(obj));
  return 31 * 17 + self->date;
}

zend_object *php_scylladb_date_new(zend_class_entry *ce) {
  php_scylladb_date *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_date, ce, &php_scylladb_date_handlers);
  self->date = 0;
  return &self->zendObject;
}



void php_scylladb_date_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_date_handlers.std.offset = offsetof(php_scylladb_date, zendObject);
}
