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

#include <php_driver.h>
#include <php_driver_types.h>
#include <util/hash.h>
#include <util/types.h>
#include <util/uuid_gen.h>
BEGIN_EXTERN_C()

#include "Uuid_arginfo.h"

zend_class_entry *php_driver_uuid_ce = NULL;

void
php_driver_uuid_init(INTERNAL_FUNCTION_PARAMETERS)
{
  char *value;
  size_t value_len;
  php_driver_uuid *self;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STRING(value, value_len)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_uuid_ce)) {
    self = PHP_DRIVER_GET_UUID(ZEND_THIS);
  } else {
    object_init_ex(return_value, php_driver_uuid_ce);
    self = PHP_DRIVER_GET_UUID(return_value);
  }

  if (ZEND_NUM_ARGS() == 0) {
    php_driver_uuid_generate_random(&self->uuid );
  } else {
    if (cass_uuid_from_string(value, &self->uuid) != CASS_OK) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                              "Invalid UUID: '%s'", value);
      return;
    }
  }
}

/* {{{ Uuid::__construct(string) */
ZEND_METHOD(Cassandra_Uuid, __construct)
{
  zend_string *uuid = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR(uuid)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_driver_uuid *self = PHP_DRIVER_GET_UUID(ZEND_THIS);

  if (uuid == nullptr) {
    php_driver_uuid_generate_random(&self->uuid);
  } else {
    if (cass_uuid_from_string(ZSTR_VAL(uuid), &self->uuid) != CASS_OK) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Invalid UUID: '%s'", ZSTR_VAL(uuid));
      return;
    }
  }
}
/* }}} */

/* {{{ Uuid::__toString() */
ZEND_METHOD(Cassandra_Uuid, __toString)
{
  char string[CASS_UUID_STRING_LENGTH];
  php_driver_uuid *self = PHP_DRIVER_GET_UUID(ZEND_THIS);

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Uuid::type() */
ZEND_METHOD(Cassandra_Uuid, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_UUID);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Uuid::uuid() */
ZEND_METHOD(Cassandra_Uuid, uuid)
{
  char string[CASS_UUID_STRING_LENGTH];
  php_driver_uuid *self = PHP_DRIVER_GET_UUID(ZEND_THIS);

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Uuid::version() */
ZEND_METHOD(Cassandra_Uuid, version)
{
  php_driver_uuid *self = PHP_DRIVER_GET_UUID(ZEND_THIS);

  RETURN_LONG((long) cass_uuid_version(self->uuid));
}
/* }}} */

static php_driver_value_handlers php_driver_uuid_handlers;

static HashTable *
php_driver_uuid_gc(zend_object *object, zval** table, int *n)
{
  *table = NULL;
  *n = 0;
  return NULL;
}

static HashTable *
php_driver_uuid_properties(zend_object *object)
{
  char string[CASS_UUID_STRING_LENGTH];
  zval type;
  zval uuid;
  zval version;

  php_driver_uuid *self = PHP5TO7_ZEND_OBJECT_GET(uuid, object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(3);
  HashTable *props = object->properties;

  cass_uuid_string(self->uuid, string);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_UUID);
  (void)zend_hash_str_update(props, "type", sizeof("type") - 1, &type);

  ZVAL_STRING(&uuid, string);
  (void)zend_hash_str_update(props, "uuid", sizeof("uuid") - 1, &uuid);

  ZVAL_LONG(&version, (long) cass_uuid_version(self->uuid));
  (void)zend_hash_str_update(props, "version", sizeof("version") - 1, &version);

  return props;
}

static int
php_driver_uuid_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_driver_uuid *uuid1 = NULL;
  php_driver_uuid *uuid2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  uuid1 = PHP_DRIVER_GET_UUID(obj1);
  uuid2 = PHP_DRIVER_GET_UUID(obj2);

  if (uuid1->uuid.time_and_version != uuid2->uuid.time_and_version)
    return uuid1->uuid.time_and_version < uuid2->uuid.time_and_version ? -1 : 1;
  if (uuid1->uuid.clock_seq_and_node != uuid2->uuid.clock_seq_and_node)
    return uuid1->uuid.clock_seq_and_node < uuid2->uuid.clock_seq_and_node ? -1 : 1;

  return 0;
}

static unsigned
php_driver_uuid_hash_value(zval *obj)
{
  php_driver_uuid *self = PHP_DRIVER_GET_UUID(obj);
  return php_driver_combine_hash(php_driver_bigint_hash(self->uuid.time_and_version),
                                    php_driver_bigint_hash(self->uuid.clock_seq_and_node));
}

static void
php_driver_uuid_free(zend_object *object)
{
  php_driver_uuid *self = PHP5TO7_ZEND_OBJECT_GET(uuid, object);

  zend_object_std_dtor(&self->zendObject);
}

static zend_object*
php_driver_uuid_new(zend_class_entry *ce)
{
  php_driver_uuid *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(uuid, ce);

  PHP5TO7_ZEND_OBJECT_INIT(uuid, self, ce);
}

void
php_driver_define_Uuid()
{
  php_driver_uuid_ce = register_class_Cassandra_Uuid(php_driver_value_ce, php_driver_uuid_interface_ce);
  php_driver_uuid_ce->create_object = php_driver_uuid_new;

  memcpy(&php_driver_uuid_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_uuid_handlers.std.get_properties  = php_driver_uuid_properties;
  php_driver_uuid_handlers.std.get_gc          = php_driver_uuid_gc;
  php_driver_uuid_handlers.std.compare = php_driver_uuid_compare;
  php_driver_uuid_handlers.hash_value = php_driver_uuid_hash_value;
  php_driver_uuid_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()
