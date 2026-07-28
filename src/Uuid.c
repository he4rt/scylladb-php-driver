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

#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include "Type/ValueHash.h"
#include "Type/TypeFactory.h"
#include "UuidGen.h"

#include "Uuid_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_uuid_handlers;


void
php_scylladb_uuid_init(INTERNAL_FUNCTION_PARAMETERS)
{
  char *value = nullptr;
  size_t value_len = 0;
  php_scylladb_uuid *self;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STRING(value, value_len)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_scylladb_uuid_ce)) {
    self = PHP_SCYLLADB_GET_UUID(ZEND_THIS);
  } else {
    object_init_ex(return_value, php_scylladb_uuid_ce);
    self = PHP_SCYLLADB_GET_UUID(return_value);
  }

  if (ZEND_NUM_ARGS() == 0) {
    php_scylladb_uuid_generate_random(&self->uuid );
  } else {
    if (cass_uuid_from_string(value, &self->uuid) != CASS_OK) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
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

  auto self = PHP_SCYLLADB_GET_UUID(ZEND_THIS);

  if (uuid == nullptr) {
    php_scylladb_uuid_generate_random(&self->uuid);
  } else {
    if (cass_uuid_from_string(ZSTR_VAL(uuid), &self->uuid) != CASS_OK) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
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
  auto self = PHP_SCYLLADB_GET_UUID(ZEND_THIS);

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Uuid::type() */
ZEND_METHOD(Cassandra_Uuid, type)
{
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_UUID);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Uuid::uuid() */
ZEND_METHOD(Cassandra_Uuid, uuid)
{
  char string[CASS_UUID_STRING_LENGTH];
  auto self = PHP_SCYLLADB_GET_UUID(ZEND_THIS);

  cass_uuid_string(self->uuid, string);

  RETVAL_STRING(string);
}
/* }}} */

/* {{{ Uuid::version() */
ZEND_METHOD(Cassandra_Uuid, version)
{
  auto self = PHP_SCYLLADB_GET_UUID(ZEND_THIS);

  RETURN_LONG((long) cass_uuid_version(self->uuid));
}
/* }}} */


HashTable *
php_scylladb_uuid_gc(zend_object *object, zval** table, int *n)
{
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *
php_scylladb_uuid_properties(zend_object *object)
{
  char string[CASS_UUID_STRING_LENGTH];
  zval type;
  zval uuid;
  zval version;

  auto self = php_scylladb_uuid_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(3);
  HashTable *props = object->properties;

  cass_uuid_string(self->uuid, string);

  type = php_scylladb_type_scalar(CASS_VALUE_TYPE_UUID);
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);

  ZVAL_STRING(&uuid, string);
  (void)zend_hash_str_update(props, ZEND_STRL("uuid"), &uuid);

  ZVAL_LONG(&version, (long) cass_uuid_version(self->uuid));
  (void)zend_hash_str_update(props, ZEND_STRL("version"), &version);

  return props;
}

int
php_scylladb_uuid_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_scylladb_uuid *uuid1 = nullptr;
  php_scylladb_uuid *uuid2 = nullptr;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  uuid1 = PHP_SCYLLADB_GET_UUID(obj1);
  uuid2 = PHP_SCYLLADB_GET_UUID(obj2);

  if (uuid1->uuid.time_and_version != uuid2->uuid.time_and_version)
    return uuid1->uuid.time_and_version < uuid2->uuid.time_and_version ? -1 : 1;
  if (uuid1->uuid.clock_seq_and_node != uuid2->uuid.clock_seq_and_node)
    return uuid1->uuid.clock_seq_and_node < uuid2->uuid.clock_seq_and_node ? -1 : 1;

  return 0;
}

unsigned
php_scylladb_uuid_hash_value(zval *obj)
{
  auto self = PHP_SCYLLADB_GET_UUID(obj);
  return php_scylladb_combine_hash(php_scylladb_bigint_hash(self->uuid.time_and_version),
                                    php_scylladb_bigint_hash(self->uuid.clock_seq_and_node));
}

void
php_scylladb_uuid_free(zend_object *object)
{
  auto self = php_scylladb_uuid_object_fetch(object);

  zend_object_std_dtor(&self->zendObject);
}

zend_object*
php_scylladb_uuid_new(zend_class_entry *ce)
{
  php_scylladb_uuid *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_uuid, ce, &php_scylladb_uuid_handlers);
  return &self->zendObject;
}


void php_scylladb_uuid_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_uuid_handlers.std.offset = offsetof(php_scylladb_uuid, zendObject);
}
