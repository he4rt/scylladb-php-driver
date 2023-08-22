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

#include <ZendCPP/ZendCPP.hpp>

#include "php_driver.h"
#include "php_driver_types.h"
#include "util/hash.h"
#include "util/types.h"
#include "util/uuid_gen.h"

BEGIN_EXTERN_C()

#include <ext/date/php_date.h>

zend_class_entry *php_driver_timeuuid_ce = nullptr;

zend_result php_driver_timeuuid_init(zval *returnValue, zend_string *str = nullptr,
                                     zend_long timestamp = -1) {
  if (returnValue == nullptr) {
    return FAILURE;
  }

  if (Z_TYPE_P(returnValue) == IS_UNDEF) {
    zval val;
    object_init_ex(&val, php_driver_timeuuid_ce);
    ZVAL_OBJ(returnValue, Z_OBJ(val));
  }

  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(returnValue);

  if (str == nullptr && timestamp == -1) {
    php_driver_uuid_generate_time(&self->uuid);
    return FAILURE;
  }

  if (str != nullptr) {
    if (cass_uuid_from_string(ZSTR_VAL(str), &self->uuid) != CASS_OK) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "Invalid UUID: '%s'",
                              ZSTR_VAL(str));
      return FAILURE;
    }

    if (int version = cass_uuid_version(self->uuid) != 1) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "UUID must be of type 1, type %d given", version);
    }
    return FAILURE;
  }

  if (timestamp < 0) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "Timestamp must be a positive integer, %ld given", timestamp);
    return FAILURE;
  }

  php_driver_uuid_generate_from_time(timestamp, &self->uuid);
  return SUCCESS;
}

/* {{{ Timeuuid::__construct(string|int) */
PHP_METHOD(Timeuuid, __construct) {
  zend_string *str = nullptr;
  zend_long timestamp = -1;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR_OR_LONG(str, timestamp)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (php_driver_timeuuid_init(getThis(), str, timestamp) != SUCCESS) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "Cannot create Timeuuid from invalid value");
    return;
  }
}
/* }}} */

/* {{{ Timeuuid::__toString() */
PHP_METHOD(Timeuuid, __toString) {
  char string[CASS_UUID_STRING_LENGTH];
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Timeuuid::type() */
PHP_METHOD(Timeuuid, type) {
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TIMEUUID);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Timeuuid::value() */
PHP_METHOD(Timeuuid, uuid) {
  char string[CASS_UUID_STRING_LENGTH];
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Timeuuid::version() */
PHP_METHOD(Timeuuid, version) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  RETURN_LONG((long)cass_uuid_version(self->uuid));
}
/* }}} */

/* {{{ Timeuuid::time() */
PHP_METHOD(Timeuuid, time) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  RETURN_LONG((long)(cass_uuid_timestamp(self->uuid) / 1000));
}
/* }}} */

/* {{{ Timeuuid::toDateTime() */
PHP_METHOD(Timeuuid, toDateTime) {
  php_driver_uuid *self;
  zval datetime_object;
  zval *datetime = &datetime_object;
  php_date_obj *datetime_obj = nullptr;
  char *str;
  int str_len;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_DRIVER_GET_UUID(getThis());

  php_date_instantiate(php_date_get_date_ce(), datetime);

  datetime_obj = php_date_obj_from_obj(Z_OBJ_P(datetime));
  str_len = spprintf(&str, 0, "@%ld", (long)(cass_uuid_timestamp(self->uuid) / 1000));
  php_date_initialize(datetime_obj, str, str_len, nullptr, nullptr, 0);
  efree(str);

  RETVAL_ZVAL(datetime, 0, 0);
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 0)
ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 80200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tostring, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()
#else
#define arginfo_tostring arginfo_none
#endif

static zend_function_entry php_driver_timeuuid_methods[] = {
    PHP_ME(Timeuuid, __construct, arginfo__construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
        PHP_ME(Timeuuid, __toString, arginfo_tostring, ZEND_ACC_PUBLIC)
            PHP_ME(Timeuuid, type, arginfo_none, ZEND_ACC_PUBLIC)
                PHP_ME(Timeuuid, uuid, arginfo_none, ZEND_ACC_PUBLIC)
                    PHP_ME(Timeuuid, version, arginfo_none, ZEND_ACC_PUBLIC)
                        PHP_ME(Timeuuid, time, arginfo_none, ZEND_ACC_PUBLIC)
                            PHP_ME(Timeuuid, toDateTime, arginfo_none, ZEND_ACC_PUBLIC) PHP_FE_END};

static zend_object_handlers php_driver_timeuuid_handlers;

static HashTable *php_driver_timeuuid_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *php_driver_timeuuid_properties(zend_object *object) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(object);
  HashTable *props = zend_std_get_properties(object);

  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TIMEUUID);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  char string[CASS_UUID_STRING_LENGTH];
  cass_uuid_string(self->uuid, string);

  zval uuid;
  ZVAL_STRING(&uuid, string);
  PHP5TO7_ZEND_HASH_UPDATE(props, "uuid", sizeof("uuid"), &uuid, sizeof(zval));

  zval version;
  ZVAL_LONG(&version, (long)cass_uuid_version(self->uuid));
  PHP5TO7_ZEND_HASH_UPDATE(props, "version", sizeof("version"), &version, sizeof(zval));

  return props;
}

static int php_driver_timeuuid_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  auto *uuid1 = ZendCPP::ObjectFetch<php_driver_uuid>(obj1);
  auto *uuid2 = ZendCPP::ObjectFetch<php_driver_uuid>(obj2);

  if (uuid1->uuid.time_and_version != uuid2->uuid.time_and_version)
    return uuid1->uuid.time_and_version < uuid2->uuid.time_and_version ? -1 : 1;
  if (uuid1->uuid.clock_seq_and_node != uuid2->uuid.clock_seq_and_node)
    return uuid1->uuid.clock_seq_and_node < uuid2->uuid.clock_seq_and_node ? -1 : 1;
  return 0;
}

static void php_driver_timeuuid_free(zend_object *object) {
  php_driver_uuid *self = PHP5TO7_ZEND_OBJECT_GET(uuid, object);
  zend_object_std_dtor(&self->zendObject);
}

static zend_object *php_driver_timeuuid_new(zend_class_entry *ce) {
  auto *self = PHP5TO7_ZEND_OBJECT_ECALLOC(uuid, ce);

  PHP5TO7_ZEND_OBJECT_INIT_EX(uuid, timeuuid, self, ce);
}

void php_driver_define_Timeuuid() {
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Timeuuid", php_driver_timeuuid_methods);
  php_driver_timeuuid_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_timeuuid_ce, 2, php_driver_value_ce,
                        php_driver_uuid_interface_ce);
  memcpy(&php_driver_timeuuid_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_driver_timeuuid_handlers.get_properties = php_driver_timeuuid_properties;
  php_driver_timeuuid_handlers.get_gc = php_driver_timeuuid_gc;
  php_driver_timeuuid_handlers.compare = php_driver_timeuuid_compare;
  php_driver_timeuuid_ce->ce_flags |= ZEND_ACC_FINAL;
  php_driver_timeuuid_ce->create_object = php_driver_timeuuid_new;
  php_driver_timeuuid_handlers.clone_obj = nullptr;
}
END_EXTERN_C()