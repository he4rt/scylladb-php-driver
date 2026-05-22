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

#include "Timeuuid_arginfo.h"

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
    return SUCCESS;
  }

  if (str != nullptr) {
    if (cass_uuid_from_string(ZSTR_VAL(str), &self->uuid) != CASS_OK) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "Invalid UUID: '%s'", ZSTR_VAL(str));
      return FAILURE;
    }

    int version = cass_uuid_version(self->uuid);
    if (version != 1) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "UUID must be of type 1, type %d given", version);
      return FAILURE;
    }
    return SUCCESS;
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
ZEND_METHOD(Cassandra_Timeuuid, __construct) {
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
ZEND_METHOD(Cassandra_Timeuuid, __toString) {
  char string[CASS_UUID_STRING_LENGTH];
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Timeuuid::type() */
ZEND_METHOD(Cassandra_Timeuuid, type) {
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TIMEUUID);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Timeuuid::uuid() */
ZEND_METHOD(Cassandra_Timeuuid, uuid) {
  char string[CASS_UUID_STRING_LENGTH];
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Timeuuid::version() */
ZEND_METHOD(Cassandra_Timeuuid, version) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  RETURN_LONG((long)cass_uuid_version(self->uuid));
}
/* }}} */

/* {{{ Timeuuid::time() */
ZEND_METHOD(Cassandra_Timeuuid, time) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(getThis());

  RETURN_LONG((long)(cass_uuid_timestamp(self->uuid) / 1000));
}
/* }}} */

static php_driver_value_handlers php_driver_timeuuid_handlers;

static HashTable *php_driver_timeuuid_gc(zend_object *object, zval** table, int *n) {
  *table = nullptr;
  *n = 0;
  return NULL;
}

static HashTable *php_driver_timeuuid_properties(zend_object *object) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(3);
  HashTable *props = object->properties;

  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TIMEUUID);
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);

  char string[CASS_UUID_STRING_LENGTH];
  cass_uuid_string(self->uuid, string);

  zval uuid;
  ZVAL_STRING(&uuid, string);
  (void)zend_hash_str_update(props, ZEND_STRL("uuid"), &uuid);

  zval version;
  ZVAL_LONG(&version, (long)cass_uuid_version(self->uuid));
  (void)zend_hash_str_update(props, ZEND_STRL("version"), &version);

  return props;
}

static int php_driver_timeuuid_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  auto *uuid1 = ZendCPP::ObjectFetch<php_driver_uuid>(obj1);
  auto *uuid2 = ZendCPP::ObjectFetch<php_driver_uuid>(obj2);

  if (uuid1->uuid.time_and_version != uuid2->uuid.time_and_version)
    return uuid1->uuid.time_and_version < uuid2->uuid.time_and_version ? -1 : 1;
  if (uuid1->uuid.clock_seq_and_node != uuid2->uuid.clock_seq_and_node)
    return uuid1->uuid.clock_seq_and_node < uuid2->uuid.clock_seq_and_node ? -1 : 1;
  return 0;
}

static unsigned php_driver_timeuuid_hash_value(zval *obj) {
  auto *self = ZendCPP::ObjectFetch<php_driver_uuid>(obj);

  return php_driver_combine_hash(
      (self->uuid.time_and_version ^ (self->uuid.time_and_version >> 32)),
      (self->uuid.clock_seq_and_node ^ (self->uuid.clock_seq_and_node >> 32)));
}

static void php_driver_timeuuid_free(zend_object *object) {
  php_driver_uuid *self = php_driver_uuid_object_fetch(object);
  zend_object_std_dtor(&self->zendObject);
}

static zend_object* php_driver_timeuuid_new(zend_class_entry *ce) {
  auto *self = (php_driver_uuid *)ecalloc(1, sizeof(php_driver_uuid) + zend_object_properties_size(ce));

  zend_object_std_init(&self->zendObject, ce);
  php_driver_timeuuid_handlers.std.offset = XtOffsetOf(php_driver_uuid, zendObject);
  php_driver_timeuuid_handlers.std.free_obj = php_driver_timeuuid_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_timeuuid_handlers;
  return &self->zendObject;
}

void php_driver_define_Timeuuid() {
  php_driver_timeuuid_ce = register_class_Cassandra_Timeuuid(php_driver_value_ce,
                                                              php_driver_uuid_interface_ce);
  php_driver_timeuuid_ce->create_object = php_driver_timeuuid_new;

  memcpy(&php_driver_timeuuid_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_driver_timeuuid_handlers.std.get_properties = php_driver_timeuuid_properties;
  php_driver_timeuuid_handlers.std.get_gc = php_driver_timeuuid_gc;
  php_driver_timeuuid_handlers.std.compare = php_driver_timeuuid_compare;
  php_driver_timeuuid_handlers.hash_value = php_driver_timeuuid_hash_value;
  php_driver_timeuuid_handlers.std.clone_obj = nullptr;
}
END_EXTERN_C()
