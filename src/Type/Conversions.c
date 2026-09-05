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
#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include "Type/Conversions.h"
#include "Type/ValueHash.h"
#include "Numbers/NumberParser.h"
#include "Type/TypeFactory.h"

#include <math.h>

#define EXPECTING_VALUE(expected)                         \
  do {                                                    \
    throw_invalid_argument(object, "argument", expected); \
    return 0;                                             \
  } while (0)

#define INSTANCE_OF(cls) \
  (Z_TYPE_P(object) == IS_OBJECT && instanceof_function(Z_OBJCE_P(object), cls))

#define CHECK_ERROR(rc) ASSERT_SUCCESS_BLOCK(rc, result = 0;)

bool php_scylladb_validate_object(zval* object, zval* ztype) {
  php_scylladb_type* type;

  if (Z_TYPE_P(object) == IS_NULL) return 1;

  type = PHP_SCYLLADB_GET_TYPE(ztype);

  switch (type->type) {
    case CASS_VALUE_TYPE_VARCHAR:
    case CASS_VALUE_TYPE_TEXT:
    case CASS_VALUE_TYPE_ASCII:
      if (Z_TYPE_P(object) != IS_STRING) {
        EXPECTING_VALUE("a string");
      }

      return 1;
    case CASS_VALUE_TYPE_DOUBLE:
      if (Z_TYPE_P(object) != IS_DOUBLE) {
        EXPECTING_VALUE("a float");
      }

      return 1;
    case CASS_VALUE_TYPE_INT:
      if (Z_TYPE_P(object) != IS_LONG) {
        EXPECTING_VALUE("an int");
      }

      return 1;
    case CASS_VALUE_TYPE_BOOLEAN:
      if (!(Z_TYPE_P(object) == IS_TRUE || Z_TYPE_P(object) == IS_FALSE)) {
        EXPECTING_VALUE("a boolean");
      }

      return 1;
    case CASS_VALUE_TYPE_FLOAT:
      if (!INSTANCE_OF(php_scylladb_float_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Float");
      }

      return 1;
    case CASS_VALUE_TYPE_COUNTER:
    case CASS_VALUE_TYPE_BIGINT:
      if (!INSTANCE_OF(php_scylladb_bigint_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Bigint");
      }

      return 1;
    case CASS_VALUE_TYPE_SMALL_INT:
      if (!INSTANCE_OF(php_scylladb_smallint_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Smallint");
      }

      return 1;
    case CASS_VALUE_TYPE_TINY_INT:
      if (!INSTANCE_OF(php_scylladb_tinyint_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
      }

      return 1;
    case CASS_VALUE_TYPE_BLOB:
      if (!INSTANCE_OF(php_scylladb_blob_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Blob");
      }

      return 1;
    case CASS_VALUE_TYPE_DECIMAL:
      if (!INSTANCE_OF(php_scylladb_decimal_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Decimal");
      }

      return 1;
    case CASS_VALUE_TYPE_DURATION:
      if (!INSTANCE_OF(php_scylladb_duration_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Duration");
      }

      return 1;
    case CASS_VALUE_TYPE_TIMESTAMP:
      if (!INSTANCE_OF(php_scylladb_timestamp_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Timestamp");
      }

      return 1;
    case CASS_VALUE_TYPE_DATE:
      if (!INSTANCE_OF(php_scylladb_date_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Date");
      }

      return 1;
    case CASS_VALUE_TYPE_TIME:
      if (!INSTANCE_OF(php_scylladb_time_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Time");
      }

      return 1;
    case CASS_VALUE_TYPE_UUID:
      if (!INSTANCE_OF(php_scylladb_uuid_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Uuid");
      }

      return 1;
    case CASS_VALUE_TYPE_VARINT:
      if (!INSTANCE_OF(php_scylladb_varint_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Varint");
      }

      return 1;
    case CASS_VALUE_TYPE_TIMEUUID:
      if (!INSTANCE_OF(php_scylladb_timeuuid_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Timeuuid");
      }

      return 1;
    case CASS_VALUE_TYPE_INET:
      if (!INSTANCE_OF(php_scylladb_inet_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Inet");
      }

      return 1;
    case CASS_VALUE_TYPE_MAP:
      if (!INSTANCE_OF(php_scylladb_map_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Map");
      } else {
        auto map = PHP_SCYLLADB_GET_MAP(object);
        auto map_type = PHP_SCYLLADB_GET_TYPE(&(map->type));
        if (php_scylladb_type_compare(map_type, type) != 0) {
          return 0;
        }
      }

      return 1;
    case CASS_VALUE_TYPE_SET:
      if (!INSTANCE_OF(php_scylladb_set_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Set");
      } else {
        auto set = PHP_SCYLLADB_GET_SET(object);
        auto set_type = PHP_SCYLLADB_GET_TYPE(&(set->type));
        if (php_scylladb_type_compare(set_type, type) != 0) {
          return 0;
        }
      }

      return 1;
    case CASS_VALUE_TYPE_LIST:
      if (!INSTANCE_OF(php_scylladb_collection_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Collection");
      } else {
        auto collection = PHP_SCYLLADB_GET_COLLECTION(object);
        auto collection_type = PHP_SCYLLADB_GET_TYPE(&(collection->type));
        if (php_scylladb_type_compare(collection_type, type) != 0) {
          return 0;
        }
      }

      return 1;
    case CASS_VALUE_TYPE_TUPLE:
      if (!INSTANCE_OF(php_scylladb_tuple_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\Tuple");
      } else {
        auto tuple = PHP_SCYLLADB_GET_TUPLE(object);
        auto tuple_type = PHP_SCYLLADB_GET_TYPE(&(tuple->type));
        if (php_scylladb_type_compare(tuple_type, type) != 0) {
          return 0;
        }
      }

      return 1;
    case CASS_VALUE_TYPE_UDT:
      if (!INSTANCE_OF(php_scylladb_user_type_value_ce)) {
        EXPECTING_VALUE("an instance of " PHP_SCYLLADB_NAMESPACE "\\UserTypeValue");
      } else {
        auto user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(object);
        auto user_type = PHP_SCYLLADB_GET_TYPE(&(user_type_value->type));
        if (php_scylladb_type_compare(user_type, type) != 0) {
          return 0;
        }
      }

      return 1;
    case CASS_VALUE_TYPE_CUSTOM:
    case CASS_VALUE_TYPE_LAST_ENTRY:
    case CASS_VALUE_TYPE_UNKNOWN:
    default:
      EXPECTING_VALUE("a simple " PHP_SCYLLADB_NAMESPACE " value");

      return 0;
  }
}

bool php_scylladb_value_type(const zend_string* type, CassValueType* value_type) {
  if (zend_string_equals_literal(type, "ascii")) {
    *value_type = CASS_VALUE_TYPE_ASCII;
  } else if (zend_string_equals_literal(type, "bigint")) {
    *value_type = CASS_VALUE_TYPE_BIGINT;
  } else if (zend_string_equals_literal(type, "smallint")) {
    *value_type = CASS_VALUE_TYPE_SMALL_INT;
  } else if (zend_string_equals_literal(type, "tinyint")) {
    *value_type = CASS_VALUE_TYPE_TINY_INT;
  } else if (zend_string_equals_literal(type, "blob")) {
    *value_type = CASS_VALUE_TYPE_BLOB;
  } else if (zend_string_equals_literal(type, "boolean")) {
    *value_type = CASS_VALUE_TYPE_BOOLEAN;
  } else if (zend_string_equals_literal(type, "counter")) {
    *value_type = CASS_VALUE_TYPE_COUNTER;
  } else if (zend_string_equals_literal(type, "decimal")) {
    *value_type = CASS_VALUE_TYPE_DECIMAL;
  } else if (zend_string_equals_literal(type, "duration")) {
    *value_type = CASS_VALUE_TYPE_DURATION;
  } else if (zend_string_equals_literal(type, "double")) {
    *value_type = CASS_VALUE_TYPE_DOUBLE;
  } else if (zend_string_equals_literal(type, "float")) {
    *value_type = CASS_VALUE_TYPE_FLOAT;
  } else if (zend_string_equals_literal(type, "int")) {
    *value_type = CASS_VALUE_TYPE_INT;
  } else if (zend_string_equals_literal(type, "text")) {
    *value_type = CASS_VALUE_TYPE_TEXT;
  } else if (zend_string_equals_literal(type, "timestamp")) {
    *value_type = CASS_VALUE_TYPE_TIMESTAMP;
  } else if (zend_string_equals_literal(type, "date")) {
    *value_type = CASS_VALUE_TYPE_DATE;
  } else if (zend_string_equals_literal(type, "time")) {
    *value_type = CASS_VALUE_TYPE_TIME;
  } else if (zend_string_equals_literal(type, "uuid")) {
    *value_type = CASS_VALUE_TYPE_UUID;
  } else if (zend_string_equals_literal(type, "varchar")) {
    *value_type = CASS_VALUE_TYPE_VARCHAR;
  } else if (zend_string_equals_literal(type, "varint")) {
    *value_type = CASS_VALUE_TYPE_VARINT;
  } else if (zend_string_equals_literal(type, "timeuuid")) {
    *value_type = CASS_VALUE_TYPE_TIMEUUID;
  } else if (zend_string_equals_literal(type, "inet")) {
    *value_type = CASS_VALUE_TYPE_INET;
  } else {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0, "Unsupported type '%s'",
                            ZSTR_VAL(type));
    return false;
  }

  return true;
}

static int php_scylladb_collection_append(CassCollection* collection, zval* value,
                                        CassValueType type) {
  int result = 1;
  php_scylladb_blob* blob;
  php_scylladb_numeric* numeric;
  php_scylladb_timestamp* timestamp;
  php_scylladb_date* date;
  php_scylladb_time* time;
  php_scylladb_uuid* uuid;
  php_scylladb_inet* inet;
  php_scylladb_duration* duration;
  size_t size;
  cass_byte_t* data;
  php_scylladb_collection* coll;
  php_scylladb_map* map;
  php_scylladb_set* set;
  php_scylladb_tuple* tup;
  php_scylladb_user_type_value* user_type_value;
  CassCollection* sub_collection;
  CassTuple* sub_tuple;
  CassUserType* sub_ut;

  switch (type) {
    case CASS_VALUE_TYPE_TEXT:
    case CASS_VALUE_TYPE_ASCII:
    case CASS_VALUE_TYPE_VARCHAR:
      CHECK_ERROR(
          cass_collection_append_string_n(collection, Z_STRVAL_P(value), Z_STRLEN_P(value)));
      break;
    case CASS_VALUE_TYPE_BIGINT:
    case CASS_VALUE_TYPE_COUNTER:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_collection_append_int64(collection, numeric->data.bigint.value));
      break;
    case CASS_VALUE_TYPE_SMALL_INT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_collection_append_int16(collection, numeric->data.smallint.value));
      break;
    case CASS_VALUE_TYPE_TINY_INT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_collection_append_int8(collection, numeric->data.tinyint.value));
      break;
    case CASS_VALUE_TYPE_BLOB:
      blob = PHP_SCYLLADB_GET_BLOB(value);
      CHECK_ERROR(cass_collection_append_bytes(collection, blob->data, blob->size));
      break;
    case CASS_VALUE_TYPE_BOOLEAN:
      CHECK_ERROR(cass_collection_append_bool(collection,
                                              Z_TYPE_P(value) == IS_TRUE ? cass_true : cass_false));
      break;
    case CASS_VALUE_TYPE_DOUBLE:
      CHECK_ERROR(cass_collection_append_double(collection, Z_DVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_FLOAT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_collection_append_float(collection, numeric->data.floating.value));
      break;
    case CASS_VALUE_TYPE_INT:
      CHECK_ERROR(cass_collection_append_int32(collection, (cass_int32_t)Z_LVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_TIMESTAMP:
      timestamp = Z_SCYLLADB_TIMESTAMP_P(value);
      CHECK_ERROR(cass_collection_append_int64(collection, timestamp->timestamp));
      break;
    case CASS_VALUE_TYPE_DATE:
      date = Z_SCYLLADB_DATE_P(value);
      CHECK_ERROR(cass_collection_append_uint32(collection, date->date));
      break;
    case CASS_VALUE_TYPE_TIME:
      time = Z_SCYLLADB_TIME_P(value);
      CHECK_ERROR(cass_collection_append_int64(collection, time->time));
      break;
    case CASS_VALUE_TYPE_UUID:
    case CASS_VALUE_TYPE_TIMEUUID:
      uuid = PHP_SCYLLADB_GET_UUID(value);
      CHECK_ERROR(cass_collection_append_uuid(collection, uuid->uuid));
      break;
    case CASS_VALUE_TYPE_VARINT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      data = export_twos_complement(numeric->data.varint.value, &size);
      CHECK_ERROR(cass_collection_append_bytes(collection, data, size));
      free(data);
      break;
    case CASS_VALUE_TYPE_DECIMAL:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      data = export_twos_complement(numeric->data.decimal.value, &size);
      CHECK_ERROR(
          cass_collection_append_decimal(collection, data, size, (cass_int32_t)numeric->data.decimal.scale));
      free(data);
      break;
    case CASS_VALUE_TYPE_DURATION:
      duration = PHP_SCYLLADB_GET_DURATION(value);
      CHECK_ERROR(cass_collection_append_duration(collection, duration->months, duration->days,
                                                  duration->nanos));
      break;
    case CASS_VALUE_TYPE_INET:
      inet = PHP_SCYLLADB_GET_INET(value);
      CHECK_ERROR(cass_collection_append_inet(collection, inet->inet));
      break;
    case CASS_VALUE_TYPE_LIST:
      coll = PHP_SCYLLADB_GET_COLLECTION(value);
      if (!php_scylladb_collection_from_collection(coll, &sub_collection)) return 0;
      CHECK_ERROR(cass_collection_append_collection(collection, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_MAP:
      map = PHP_SCYLLADB_GET_MAP(value);
      if (!php_scylladb_collection_from_map(map, &sub_collection)) return 0;
      CHECK_ERROR(cass_collection_append_collection(collection, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_SET:
      set = PHP_SCYLLADB_GET_SET(value);
      if (!php_scylladb_collection_from_set(set, &sub_collection)) return 0;
      CHECK_ERROR(cass_collection_append_collection(collection, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_TUPLE:
      tup = PHP_SCYLLADB_GET_TUPLE(value);
      if (!php_scylladb_tuple_from_tuple(tup, &sub_tuple)) return 0;
      CHECK_ERROR(cass_collection_append_tuple(collection, sub_tuple));
      cass_tuple_free(sub_tuple);
      break;
    case CASS_VALUE_TYPE_UDT:
      user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(value);
      if (!php_scylladb_user_type_from_user_type_value(user_type_value, &sub_ut)) return 0;
      CHECK_ERROR(cass_collection_append_user_type(collection, sub_ut));
      cass_user_type_free(sub_ut);
      break;
    case CASS_VALUE_TYPE_CUSTOM:
    case CASS_VALUE_TYPE_LAST_ENTRY:
    case CASS_VALUE_TYPE_UNKNOWN:
    default:
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0, "Unsupported collection type");
      return 0;
  }

  return result;
}

static int php_scylladb_tuple_set(CassTuple* tuple, zend_ulong index, zval* value,
                                CassValueType type) {
  int result = 1;
  php_scylladb_blob* blob;
  php_scylladb_numeric* numeric;
  php_scylladb_timestamp* timestamp;
  php_scylladb_date* date;
  php_scylladb_time* time;
  php_scylladb_uuid* uuid;
  php_scylladb_inet* inet;
  php_scylladb_duration* duration;
  size_t size;
  cass_byte_t* data;
  php_scylladb_collection* coll;
  php_scylladb_map* map;
  php_scylladb_set* set;
  php_scylladb_tuple* tup;
  php_scylladb_user_type_value* user_type_value;
  CassCollection* sub_collection;
  CassTuple* sub_tuple;
  CassUserType* sub_ut;

  if (Z_TYPE_P(value) == IS_NULL) {
    CHECK_ERROR(cass_tuple_set_null(tuple, index));
    return result;
  }

  switch (type) {
    case CASS_VALUE_TYPE_TEXT:
    case CASS_VALUE_TYPE_ASCII:
    case CASS_VALUE_TYPE_VARCHAR:
      CHECK_ERROR(cass_tuple_set_string_n(tuple, index, Z_STRVAL_P(value), Z_STRLEN_P(value)));
      break;
    case CASS_VALUE_TYPE_BIGINT:
    case CASS_VALUE_TYPE_COUNTER:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_tuple_set_int64(tuple, index, numeric->data.bigint.value));
      break;
    case CASS_VALUE_TYPE_SMALL_INT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_tuple_set_int16(tuple, index, numeric->data.smallint.value));
      break;
    case CASS_VALUE_TYPE_TINY_INT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_tuple_set_int8(tuple, index, numeric->data.tinyint.value));
      break;
    case CASS_VALUE_TYPE_BLOB:
      blob = PHP_SCYLLADB_GET_BLOB(value);
      CHECK_ERROR(cass_tuple_set_bytes(tuple, index, blob->data, blob->size));
      break;
    case CASS_VALUE_TYPE_BOOLEAN:
      CHECK_ERROR(
          cass_tuple_set_bool(tuple, index, Z_TYPE_P(value) == IS_TRUE ? cass_true : cass_false));
      break;
    case CASS_VALUE_TYPE_DOUBLE:
      CHECK_ERROR(cass_tuple_set_double(tuple, index, Z_DVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_FLOAT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_tuple_set_float(tuple, index, numeric->data.floating.value));
      break;
    case CASS_VALUE_TYPE_INT:
      CHECK_ERROR(cass_tuple_set_int32(tuple, index, (cass_int32_t)Z_LVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_TIMESTAMP:
      timestamp = Z_SCYLLADB_TIMESTAMP_P(value);
      CHECK_ERROR(cass_tuple_set_int64(tuple, index, timestamp->timestamp));
      break;
    case CASS_VALUE_TYPE_DATE:
      date = Z_SCYLLADB_DATE_P(value);
      CHECK_ERROR(cass_tuple_set_uint32(tuple, index, date->date));
      break;
    case CASS_VALUE_TYPE_TIME:
      time = Z_SCYLLADB_TIME_P(value);
      CHECK_ERROR(cass_tuple_set_int64(tuple, index, time->time));
      break;
    case CASS_VALUE_TYPE_UUID:
    case CASS_VALUE_TYPE_TIMEUUID:
      uuid = PHP_SCYLLADB_GET_UUID(value);
      CHECK_ERROR(cass_tuple_set_uuid(tuple, index, uuid->uuid));
      break;
    case CASS_VALUE_TYPE_VARINT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      data = export_twos_complement(numeric->data.varint.value, &size);
      CHECK_ERROR(cass_tuple_set_bytes(tuple, index, data, size));
      free(data);
      break;
    case CASS_VALUE_TYPE_DECIMAL:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      data = export_twos_complement(numeric->data.decimal.value, &size);
      CHECK_ERROR(cass_tuple_set_decimal(tuple, index, data, size, (cass_int32_t)numeric->data.decimal.scale));
      free(data);
      break;
    case CASS_VALUE_TYPE_DURATION:
      duration = PHP_SCYLLADB_GET_DURATION(value);
      CHECK_ERROR(
          cass_tuple_set_duration(tuple, index, duration->months, duration->days, duration->nanos));
      break;
    case CASS_VALUE_TYPE_INET:
      inet = PHP_SCYLLADB_GET_INET(value);
      CHECK_ERROR(cass_tuple_set_inet(tuple, index, inet->inet));
      break;
    case CASS_VALUE_TYPE_LIST:
      coll = PHP_SCYLLADB_GET_COLLECTION(value);
      if (!php_scylladb_collection_from_collection(coll, &sub_collection)) return 0;
      CHECK_ERROR(cass_tuple_set_collection(tuple, index, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_MAP:
      map = PHP_SCYLLADB_GET_MAP(value);
      if (!php_scylladb_collection_from_map(map, &sub_collection)) return 0;
      CHECK_ERROR(cass_tuple_set_collection(tuple, index, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_SET:
      set = PHP_SCYLLADB_GET_SET(value);
      if (!php_scylladb_collection_from_set(set, &sub_collection)) return 0;
      CHECK_ERROR(cass_tuple_set_collection(tuple, index, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_TUPLE:
      tup = PHP_SCYLLADB_GET_TUPLE(value);
      if (!php_scylladb_tuple_from_tuple(tup, &sub_tuple)) return 0;
      CHECK_ERROR(cass_tuple_set_tuple(tuple, index, sub_tuple));
      cass_tuple_free(sub_tuple);
      break;
    case CASS_VALUE_TYPE_UDT:
      user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(value);
      if (!php_scylladb_user_type_from_user_type_value(user_type_value, &sub_ut)) return 0;
      CHECK_ERROR(cass_tuple_set_user_type(tuple, index, sub_ut));
      cass_user_type_free(sub_ut);
      break;
    case CASS_VALUE_TYPE_CUSTOM:
    case CASS_VALUE_TYPE_LAST_ENTRY:
    case CASS_VALUE_TYPE_UNKNOWN:
    default:
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0, "Unsupported collection type");
      return 0;
  }

  return result;
}

static int php_scylladb_user_type_set(CassUserType* ut, const char* name, zval* value,
                                    CassValueType type) {
  int result = 1;
  php_scylladb_blob* blob;
  php_scylladb_numeric* numeric;
  php_scylladb_timestamp* timestamp;
  php_scylladb_date* date;
  php_scylladb_time* time;
  php_scylladb_uuid* uuid;
  php_scylladb_inet* inet;
  php_scylladb_duration* duration;
  size_t size;
  cass_byte_t* data;
  php_scylladb_collection* coll;
  php_scylladb_map* map;
  php_scylladb_set* set;
  php_scylladb_tuple* tuple;
  php_scylladb_user_type_value* user_type_value;
  CassCollection* sub_collection;
  CassTuple* sub_tup;
  CassUserType* sub_ut;

  if (Z_TYPE_P(value) == IS_NULL) {
    CHECK_ERROR(cass_user_type_set_null_by_name(ut, name));
    return result;
  }

  switch (type) {
    case CASS_VALUE_TYPE_TEXT:
    case CASS_VALUE_TYPE_ASCII:
    case CASS_VALUE_TYPE_VARCHAR:
      CHECK_ERROR(cass_user_type_set_string_by_name(ut, name, Z_STRVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_BIGINT:
    case CASS_VALUE_TYPE_COUNTER:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_user_type_set_int64_by_name(ut, name, numeric->data.bigint.value));
      break;
    case CASS_VALUE_TYPE_SMALL_INT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_user_type_set_int16_by_name(ut, name, numeric->data.smallint.value));
      break;
    case CASS_VALUE_TYPE_TINY_INT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_user_type_set_int8_by_name(ut, name, numeric->data.tinyint.value));
      break;
    case CASS_VALUE_TYPE_BLOB:
      blob = PHP_SCYLLADB_GET_BLOB(value);
      CHECK_ERROR(cass_user_type_set_bytes_by_name(ut, name, blob->data, blob->size));
      break;
    case CASS_VALUE_TYPE_BOOLEAN:
      CHECK_ERROR(cass_user_type_set_bool_by_name(
          ut, name, Z_TYPE_P(value) == IS_TRUE ? cass_true : cass_false));
      break;
    case CASS_VALUE_TYPE_DOUBLE:
      CHECK_ERROR(cass_user_type_set_double_by_name(ut, name, Z_DVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_FLOAT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_ERROR(cass_user_type_set_float_by_name(ut, name, numeric->data.floating.value));
      break;
    case CASS_VALUE_TYPE_INT:
      CHECK_ERROR(cass_user_type_set_int32_by_name(ut, name, (cass_int32_t)Z_LVAL_P(value)));
      break;
    case CASS_VALUE_TYPE_TIMESTAMP:
      timestamp = Z_SCYLLADB_TIMESTAMP_P(value);
      CHECK_ERROR(cass_user_type_set_int64_by_name(ut, name, timestamp->timestamp));
      break;
    case CASS_VALUE_TYPE_DATE:
      date = Z_SCYLLADB_DATE_P(value);
      CHECK_ERROR(cass_user_type_set_uint32_by_name(ut, name, date->date));
      break;
    case CASS_VALUE_TYPE_TIME:
      time = Z_SCYLLADB_TIME_P(value);
      CHECK_ERROR(cass_user_type_set_int64_by_name(ut, name, time->time));
      break;
    case CASS_VALUE_TYPE_UUID:
    case CASS_VALUE_TYPE_TIMEUUID:
      uuid = PHP_SCYLLADB_GET_UUID(value);
      CHECK_ERROR(cass_user_type_set_uuid_by_name(ut, name, uuid->uuid));
      break;
    case CASS_VALUE_TYPE_VARINT:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      data = export_twos_complement(numeric->data.varint.value, &size);
      CHECK_ERROR(cass_user_type_set_bytes_by_name(ut, name, data, size));
      free(data);
      break;
    case CASS_VALUE_TYPE_DECIMAL:
      numeric = PHP_SCYLLADB_GET_NUMERIC(value);
      data = export_twos_complement(numeric->data.decimal.value, &size);
      CHECK_ERROR(
          cass_user_type_set_decimal_by_name(ut, name, data, size, (cass_int32_t)numeric->data.decimal.scale));
      free(data);
      break;
    case CASS_VALUE_TYPE_DURATION:
      duration = PHP_SCYLLADB_GET_DURATION(value);
      CHECK_ERROR(cass_user_type_set_duration_by_name(ut, name, duration->months, duration->days,
                                                      duration->nanos));
      break;
    case CASS_VALUE_TYPE_INET:
      inet = PHP_SCYLLADB_GET_INET(value);
      CHECK_ERROR(cass_user_type_set_inet_by_name(ut, name, inet->inet));
      break;
    case CASS_VALUE_TYPE_LIST:
      coll = PHP_SCYLLADB_GET_COLLECTION(value);
      if (!php_scylladb_collection_from_collection(coll, &sub_collection)) return 0;
      CHECK_ERROR(cass_user_type_set_collection_by_name(ut, name, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_MAP:
      map = PHP_SCYLLADB_GET_MAP(value);
      if (!php_scylladb_collection_from_map(map, &sub_collection)) return 0;
      CHECK_ERROR(cass_user_type_set_collection_by_name(ut, name, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_SET:
      set = PHP_SCYLLADB_GET_SET(value);
      if (!php_scylladb_collection_from_set(set, &sub_collection)) return 0;
      CHECK_ERROR(cass_user_type_set_collection_by_name(ut, name, sub_collection));
      cass_collection_free(sub_collection);
      break;
    case CASS_VALUE_TYPE_TUPLE:
      tuple = PHP_SCYLLADB_GET_TUPLE(value);
      if (!php_scylladb_tuple_from_tuple(tuple, &sub_tup)) return 0;
      CHECK_ERROR(cass_user_type_set_tuple_by_name(ut, name, sub_tup));
      cass_tuple_free(sub_tup);
      break;
    case CASS_VALUE_TYPE_UDT:
      user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(value);
      if (!php_scylladb_user_type_from_user_type_value(user_type_value, &sub_ut)) return 0;
      CHECK_ERROR(cass_user_type_set_user_type_by_name(ut, name, sub_ut));
      cass_user_type_free(sub_ut);
      break;
    case CASS_VALUE_TYPE_CUSTOM:
    case CASS_VALUE_TYPE_LAST_ENTRY:
    case CASS_VALUE_TYPE_UNKNOWN:
    default:
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0, "Unsupported collection type");
      return 0;
  }

  return result;
}

bool php_scylladb_collection_from_set(php_scylladb_set* set, CassCollection** collection_ptr) {
  int result = 1;
  CassCollection* collection = nullptr;
  php_scylladb_type* type;
  php_scylladb_type* value_type;
  php_scylladb_set_entry *curr, *temp;

  type = PHP_SCYLLADB_GET_TYPE(&set->type);
  value_type = PHP_SCYLLADB_GET_TYPE(&type->data.set.value_type);
  collection = cass_collection_new_from_data_type(type->data_type, HASH_COUNT(set->entries));
  if (collection == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassCollection for set");
    return 0;
  }
  HASH_ITER(hh, set->entries, curr, temp) {
    if (!php_scylladb_collection_append(collection, &curr->value, value_type->type)) {
      result = 0;
      break;
    }
  }

  if (result) {
    *collection_ptr = collection;
  } else {
    cass_collection_free(collection);
  }

  return result;
}

bool php_scylladb_collection_from_collection(php_scylladb_collection* coll,
                                          CassCollection** collection_ptr) {
  int result = 1;
  zval* current;
  php_scylladb_type* type;
  php_scylladb_type* value_type;
  CassCollection* collection;

  type = PHP_SCYLLADB_GET_TYPE(&(coll->type));
  value_type = PHP_SCYLLADB_GET_TYPE(&(type->data.collection.value_type));
  collection =
      cass_collection_new_from_data_type(type->data_type, zend_hash_num_elements(&coll->values));
  if (collection == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassCollection for list");
    return 0;
  }

  ZEND_HASH_FOREACH_VAL(&coll->values, current) {
    if (!php_scylladb_collection_append(collection, current, value_type->type)) {
      result = 0;
      break;
    }
  }
  ZEND_HASH_FOREACH_END();

  if (result)
    *collection_ptr = collection;
  else
    cass_collection_free(collection);

  return result;
}

bool php_scylladb_collection_from_map(php_scylladb_map* map, CassCollection** collection_ptr) {
  int result = 1;
  CassCollection* collection;
  php_scylladb_type* type;
  php_scylladb_type* key_type;
  php_scylladb_type* value_type;
  php_scylladb_map_entry *curr, *temp;

  type = PHP_SCYLLADB_GET_TYPE(&(map->type));
  value_type = PHP_SCYLLADB_GET_TYPE(&(type->data.map.value_type));
  key_type = PHP_SCYLLADB_GET_TYPE(&(type->data.map.key_type));
  collection = cass_collection_new_from_data_type(type->data_type, HASH_COUNT(map->entries));
  if (collection == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassCollection for map");
    return 0;
  }
  HASH_ITER(hh, map->entries, curr, temp) {
    if (!php_scylladb_collection_append(collection, &(curr->key), key_type->type)) {
      result = 0;
      break;
    }
    if (!php_scylladb_collection_append(collection, &(curr->value), value_type->type)) {
      result = 0;
      break;
    }
  }

  if (result)
    *collection_ptr = collection;
  else
    cass_collection_free(collection);

  return result;
}

bool php_scylladb_tuple_from_tuple(php_scylladb_tuple* tuple, CassTuple** output) {
  int result = 1;
  zend_ulong num_key;
  zval* current;
  php_scylladb_type* type;
  CassTuple* tup;

  type = PHP_SCYLLADB_GET_TYPE(&(tuple->type));
  tup = cass_tuple_new_from_data_type(type->data_type);
  if (tup == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassTuple");
    return 0;
  }

  ZEND_HASH_FOREACH_NUM_KEY_VAL(&tuple->values, num_key, current) {
    zval* zsub_type;
    php_scylladb_type* sub_type;
    if ((zsub_type = zend_hash_index_find(&type->data.tuple.types, num_key)) == nullptr ||
        !php_scylladb_validate_object((current), (zsub_type))) {
      result = 0;
      break;
    }
    sub_type = PHP_SCYLLADB_GET_TYPE((zsub_type));
    if (!php_scylladb_tuple_set(tup, num_key, (current), sub_type->type)) {
      result = 0;
      break;
    }
  }
  ZEND_HASH_FOREACH_END();

  if (result)
    *output = tup;
  else
    cass_tuple_free(tup);

  return result;
}

bool php_scylladb_user_type_from_user_type_value(php_scylladb_user_type_value* user_type_value,
                                              CassUserType** output) {
  int result = 1;
  zend_string* name;
  zval* current;
  php_scylladb_type* type;
  CassUserType* ut;

  type = PHP_SCYLLADB_GET_TYPE(&user_type_value->type);
  ut = cass_user_type_new_from_data_type(type->data_type);
  if (ut == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassUserType");
    return 0;
  }

  ZEND_HASH_FOREACH_STR_KEY_VAL(&user_type_value->values, name, current) {
    zval* zsub_type;
    php_scylladb_type* sub_type;
    if ((zsub_type = zend_hash_find(&type->data.udt.types, name)) == nullptr ||
        !php_scylladb_validate_object((current), (zsub_type))) {
      result = 0;
      break;
    }
    sub_type = PHP_SCYLLADB_GET_TYPE((zsub_type));
    if (!php_scylladb_user_type_set(ut, ZSTR_VAL(name), (current), sub_type->type)) {
      result = 0;
      break;
    }
  }
  ZEND_HASH_FOREACH_END();

  if (result) {
    *output = ut;
  } else {
    cass_user_type_free(ut);
  }

  return result;
}
