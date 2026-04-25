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
#include <php_driver_globals.h>
#include <php_driver_types.h>
#include <util/types.h>
#include <zend_smart_str.h>

#include <ZendCPP/ZendCPP.hpp>

#include "DateTime/Date.h"
#include "src/Blob.h"
#include "src/DateTime/Duration.h"
#include "src/DateTime/Timeuuid.h"
#include "src/Inet.h"
#include "src/Numbers/Bigint.h"
#include "src/Numbers/Decimal.h"
#include "src/Numbers/Float.h"
#include "src/Numbers/Smallint.h"
#include "src/Numbers/Tinyint.h"
#include "src/Numbers/Varint.h"
#include "src/Type/Tuple.h"
#include "src/Type/UserType.h"
#include "src/Uuid.h"

#define PHP_DRIVER_SCALAR_TYPES_MAP(XX)    \
  XX(ascii, CASS_VALUE_TYPE_ASCII)         \
  XX(bigint, CASS_VALUE_TYPE_BIGINT)       \
  XX(smallint, CASS_VALUE_TYPE_SMALL_INT)  \
  XX(tinyint, CASS_VALUE_TYPE_TINY_INT)    \
  XX(blob, CASS_VALUE_TYPE_BLOB)           \
  XX(boolean, CASS_VALUE_TYPE_BOOLEAN)     \
  XX(counter, CASS_VALUE_TYPE_COUNTER)     \
  XX(decimal, CASS_VALUE_TYPE_DECIMAL)     \
  XX(double, CASS_VALUE_TYPE_DOUBLE)       \
  XX(duration, CASS_VALUE_TYPE_DURATION)   \
  XX(float, CASS_VALUE_TYPE_FLOAT)         \
  XX(int, CASS_VALUE_TYPE_INT)             \
  XX(text, CASS_VALUE_TYPE_TEXT)           \
  XX(timestamp, CASS_VALUE_TYPE_TIMESTAMP) \
  XX(date, CASS_VALUE_TYPE_DATE)           \
  XX(time, CASS_VALUE_TYPE_TIME)           \
  XX(uuid, CASS_VALUE_TYPE_UUID)           \
  XX(varchar, CASS_VALUE_TYPE_VARCHAR)     \
  XX(varint, CASS_VALUE_TYPE_VARINT)       \
  XX(timeuuid, CASS_VALUE_TYPE_TIMEUUID)   \
  XX(inet, CASS_VALUE_TYPE_INET)

struct node_s {
  struct node_s* parent;
  const char* name;
  size_t name_length;
  struct node_s* first_child;
  struct node_s* last_child;
  struct node_s* next_sibling;
  struct node_s* prev_sibling;
};
static int hex_value(int c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return -1;
}

static char* php_driver_from_hex(const char* hex, size_t hex_length) {
  size_t i, c = 0;
  size_t size = hex_length / 2;
  char* result;
  if ((hex_length & 1) == 1) { /* Invalid if not divisible by 2 */
    return NULL;
  }
  result = static_cast<char*>(emalloc(size + 1));
  for (i = 0; i < size; ++i) {
    int half0 = hex_value(hex[i * 2]);
    int half1 = hex_value(hex[i * 2 + 1]);
    if (half0 < 0 || half1 < 0) {
      efree(result);
      return NULL;
    }
    result[c++] = (char)(((uint8_t)half0 << 4) | (uint8_t)half1);
  }
  result[size] = '\0';
  return result;
}

static zval php_driver_create_type(struct node_s* node);

static zval php_driver_tuple_from_data_type(const CassDataType* data_type) {
  zval ztype;
  php_driver_type* type;
  size_t i, count;

  count = cass_data_type_sub_type_count(data_type);
  ztype = php_driver_type_tuple();
  type = PHP_DRIVER_GET_TYPE(&ztype);
  for (i = 0; i < count; ++i) {
    zval sub_type = php_driver_type_from_data_type(cass_data_type_sub_data_type(data_type, i));
    php_driver_type_tuple_add(type, &sub_type);
  }

  return ztype;
}

static zval php_driver_tuple_from_node(struct node_s* node) {
  zval ztype;
  php_driver_type* type;
  struct node_s* current;

  ztype = php_driver_type_tuple();
  type = PHP_DRIVER_GET_TYPE(&ztype);

  for (current = node->first_child; current != NULL; current = current->next_sibling) {
    zval sub_type = php_driver_create_type(current);
    php_driver_type_tuple_add(type, &sub_type);
  }

  return ztype;
}

static zval php_driver_user_type_from_data_type(const CassDataType* data_type) {
  zval ztype;
  php_driver_type* type;
  const char *type_name, *keyspace;
  size_t type_name_len, keyspace_len;
  size_t i, count;

  count = cass_data_type_sub_type_count(data_type);
  ztype = php_driver_type_user_type();
  type = PHP_DRIVER_GET_TYPE(&ztype);

  cass_data_type_type_name(data_type, &type_name, &type_name_len);
  type->data.udt.type_name = estrndup(type_name, type_name_len);
  cass_data_type_keyspace(data_type, &keyspace, &keyspace_len);
  type->data.udt.keyspace = estrndup(keyspace, keyspace_len);

  for (i = 0; i < count; ++i) {
    const char* name;
    size_t name_length;
    zval sub_type = php_driver_type_from_data_type(cass_data_type_sub_data_type(data_type, i));
    cass_data_type_sub_type_name(data_type, i, &name, &name_length);
    php_driver_type_user_type_add(type, name, name_length, &sub_type);
  }

  return ztype;
}

static zval php_driver_user_type_from_node(struct node_s* node) {
  zval ztype;
  php_driver_type* type;
  struct node_s* current = node->first_child;

  ztype = php_driver_type_user_type();
  type = PHP_DRIVER_GET_TYPE(&ztype);

  if (current) {
    type->data.udt.keyspace = estrndup(current->name, current->name_length);
    current = current->next_sibling;
  }

  if (current) {
    type->data.udt.type_name = php_driver_from_hex(current->name, current->name_length);
    current = current->next_sibling;
  }

  for (; current; current = current->next_sibling) {
    zval sub_type;
    char* name = php_driver_from_hex(current->name, current->name_length);
    current = current->next_sibling;
    if (!current) {
      efree(name);
      break;
    }
    sub_type = php_driver_create_type(current);
    php_driver_type_user_type_add(type, name, strlen(name), &sub_type);
    efree(name);
  }

  return ztype;
}

static inline int php5to7_string_compare(zend_string* s1, zend_string* s2) {
  if (s1->len != s2->len) {
    return s1->len < s2->len ? -1 : 1;
  }
  return memcmp(s1->val, s2->val, s1->len);
}

zval php_driver_type_from_data_type(const CassDataType* data_type) {
  zval ztype;
  zval key_type;
  zval value_type;
  const char* class_name;
  size_t class_name_length;
  CassValueType type = cass_data_type_type(data_type);

  ZVAL_UNDEF(&ztype);

  switch (type) {
#define XX_SCALAR(name, value)             \
  case value:                              \
    ztype = php_driver_type_scalar(value); \
    break;
    PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR

    case CASS_VALUE_TYPE_CUSTOM:
      cass_data_type_class_name(data_type, &class_name, &class_name_length);
      ztype = php_driver_type_custom(class_name, class_name_length);
      break;

    case CASS_VALUE_TYPE_LIST:
      value_type = php_driver_type_from_data_type(cass_data_type_sub_data_type(data_type, 0));
      ztype = php_driver_type_collection(&value_type);
      break;

    case CASS_VALUE_TYPE_MAP:
      key_type = php_driver_type_from_data_type(cass_data_type_sub_data_type(data_type, 0));
      value_type = php_driver_type_from_data_type(cass_data_type_sub_data_type(data_type, 1));
      ztype = php_driver_type_map(&key_type, &value_type);
      break;

    case CASS_VALUE_TYPE_SET:
      value_type = php_driver_type_from_data_type(cass_data_type_sub_data_type(data_type, 0));
      ztype = php_driver_type_set(&value_type);
      break;

    case CASS_VALUE_TYPE_TUPLE:
      ztype = php_driver_tuple_from_data_type(data_type);
      break;

    case CASS_VALUE_TYPE_UDT:
      ztype = php_driver_user_type_from_data_type(data_type);
      break;

    default:
      break;
  }

  return ztype;
}

int php_driver_type_validate(zval* object, const char* object_name) {
  if (!instanceof_function(Z_OBJCE_P(object), php_driver_type_scalar_ce) &&
      !instanceof_function(Z_OBJCE_P(object), php_driver_type_collection_ce) &&
      !instanceof_function(Z_OBJCE_P(object), php_driver_type_map_ce) &&
      !instanceof_function(Z_OBJCE_P(object), php_driver_type_set_ce) &&
      !instanceof_function(Z_OBJCE_P(object), php_driver_type_tuple_ce) &&
      !instanceof_function(Z_OBJCE_P(object), php_driver_type_user_type_ce)) {
    throw_invalid_argument(object, object_name, "a valid " PHP_DRIVER_NAMESPACE "\\Type");
    return 0;
  }
  return 1;
}

static inline int collection_compare(php_driver_type* type1, php_driver_type* type2) {
  return php_driver_type_compare(PHP_DRIVER_GET_TYPE(&type1->data.collection.value_type),
                                 PHP_DRIVER_GET_TYPE(&type2->data.collection.value_type));
}

static inline int map_compare(php_driver_type* type1, php_driver_type* type2) {
  int result;
  result = php_driver_type_compare(PHP_DRIVER_GET_TYPE(&type1->data.map.key_type),
                                   PHP_DRIVER_GET_TYPE(&type2->data.map.key_type));
  if (result != 0) return result;
  result = php_driver_type_compare(PHP_DRIVER_GET_TYPE(&type1->data.map.value_type),
                                   PHP_DRIVER_GET_TYPE(&type2->data.map.value_type));
  if (result != 0) return result;
  return 0;
}

static inline int set_compare(php_driver_type* type1, php_driver_type* type2) {
  return php_driver_type_compare(PHP_DRIVER_GET_TYPE(&type1->data.set.value_type),
                                 PHP_DRIVER_GET_TYPE(&type2->data.set.value_type));
}

static inline int tuple_compare(php_driver_type* type1, php_driver_type* type2) {
  HashPosition pos1;
  HashPosition pos2;
  zval* current1;
  zval* current2;

  if (zend_hash_num_elements(&type1->data.tuple.types) !=
      zend_hash_num_elements(&type2->data.tuple.types)) {
    return zend_hash_num_elements(&type1->data.tuple.types) <
                   zend_hash_num_elements(&type2->data.tuple.types)
               ? -1
               : 1;
  }

  zend_hash_internal_pointer_reset_ex(&type1->data.tuple.types, &pos1);
  zend_hash_internal_pointer_reset_ex(&type2->data.tuple.types, &pos2);

  while (PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&type1->data.tuple.types, current1, &pos1) &&
         PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&type2->data.tuple.types, current2, &pos2)) {
    php_driver_type* sub_type1 = PHP_DRIVER_GET_TYPE(current1);
    php_driver_type* sub_type2 = PHP_DRIVER_GET_TYPE(current2);
    int result = php_driver_type_compare(sub_type1, sub_type2);
    if (result != 0) return result;
    zend_hash_move_forward_ex(&type1->data.tuple.types, &pos1);
    zend_hash_move_forward_ex(&type2->data.tuple.types, &pos2);
  }

  return 0;
}

static inline int user_type_compare(php_driver_type* type1, php_driver_type* type2) {
  HashPosition pos1;
  HashPosition pos2;
  zend_string* key1;
  zend_string* key2;
  zval* current1;
  zval* current2;

  if (zend_hash_num_elements(&type1->data.udt.types) !=
      zend_hash_num_elements(&type2->data.udt.types)) {
    return zend_hash_num_elements(&type1->data.udt.types) <
                   zend_hash_num_elements(&type2->data.udt.types)
               ? -1
               : 1;
  }

  zend_hash_internal_pointer_reset_ex(&type1->data.udt.types, &pos1);
  zend_hash_internal_pointer_reset_ex(&type2->data.udt.types, &pos2);

  while (PHP5TO7_ZEND_HASH_GET_CURRENT_KEY_EX(&type1->data.udt.types, &key1, NULL, &pos1) ==
             HASH_KEY_IS_STRING &&
         PHP5TO7_ZEND_HASH_GET_CURRENT_KEY_EX(&type2->data.udt.types, &key2, NULL, &pos2) ==
             HASH_KEY_IS_STRING &&
         PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&type1->data.udt.types, current1, &pos1) &&
         PHP5TO7_ZEND_HASH_GET_CURRENT_DATA_EX(&type2->data.udt.types, current2, &pos2)) {
    int result;
    php_driver_type* sub_type1 = PHP_DRIVER_GET_TYPE(current1);
    php_driver_type* sub_type2 = PHP_DRIVER_GET_TYPE(current2);
    result = php5to7_string_compare(key1, key2);
    if (result != 0) return result;
    result = php_driver_type_compare(sub_type1, sub_type2);
    if (result != 0) return result;
    zend_hash_move_forward_ex(&type1->data.udt.types, &pos1);
    zend_hash_move_forward_ex(&type2->data.udt.types, &pos2);
  }

  return 0;
}

static inline int is_string_type(CassValueType type) {
  return type == CASS_VALUE_TYPE_VARCHAR || type == CASS_VALUE_TYPE_TEXT;
}

int php_driver_type_compare(php_driver_type* type1, php_driver_type* type2) {
  if (type1->type != type2->type) {
    if (is_string_type(type1->type) &&
        is_string_type(type2->type)) { /* varchar and text are aliases */
      return 0;
    }
    return type1->type < type2->type ? -1 : 1;
  } else {
    switch (type1->type) {
      case CASS_VALUE_TYPE_LIST:
        return collection_compare(type1, type2);

      case CASS_VALUE_TYPE_MAP:
        return map_compare(type1, type2);

      case CASS_VALUE_TYPE_SET:
        return set_compare(type1, type2);

      case CASS_VALUE_TYPE_TUPLE:
        return tuple_compare(type1, type2);

      case CASS_VALUE_TYPE_UDT:
        return user_type_compare(type1, type2);

      default:
        break;
    }
    return 0;
  }
}

static inline void collection_string(php_driver_type* type, smart_str* string) {
  smart_str_appendl(string, "list<", 5);
  php_driver_type_string(PHP_DRIVER_GET_TYPE(&type->data.collection.value_type), string);
  smart_str_appendl(string, ">", 1);
}

static inline void map_string(php_driver_type* type, smart_str* string) {
  smart_str_appendl(string, "map<", 4);
  php_driver_type_string(PHP_DRIVER_GET_TYPE(&type->data.map.key_type), string);
  smart_str_appendl(string, ", ", 2);
  php_driver_type_string(PHP_DRIVER_GET_TYPE(&type->data.map.value_type), string);
  smart_str_appendl(string, ">", 1);
}

static inline void set_string(php_driver_type* type, smart_str* string) {
  smart_str_appendl(string, "set<", 4);
  php_driver_type_string(PHP_DRIVER_GET_TYPE(&type->data.set.value_type), string);
  smart_str_appendl(string, ">", 1);
}

static inline void tuple_string(php_driver_type* type, smart_str* string) {
  zval* current;
  int first = 1;

  smart_str_appendl(string, "tuple<", 6);
  PHP5TO7_ZEND_HASH_FOREACH_VAL(&type->data.tuple.types, current) {
    php_driver_type* sub_type = PHP_DRIVER_GET_TYPE(current);
    if (!first) smart_str_appendl(string, ", ", 2);
    first = 0;
    php_driver_type_string(sub_type, string);
  }
  PHP5TO7_ZEND_HASH_FOREACH_END(&type->data.tuple.types);
  smart_str_appendl(string, ">", 1);
}

static inline void user_type_string(php_driver_type* type, smart_str* string) {
  char* name;
  zval* current;
  int first = 1;

  if (type->data.udt.type_name) {
    if (type->data.udt.keyspace) {
      smart_str_appendl(string, type->data.udt.keyspace, strlen(type->data.udt.keyspace));
      smart_str_appendl(string, ".", 1);
    }
    smart_str_appendl(string, type->data.udt.type_name, strlen(type->data.udt.type_name));
  } else {
    smart_str_appendl(string, "userType<", 9);
    PHP5TO7_ZEND_HASH_FOREACH_STR_KEY_VAL(&type->data.udt.types, name, current) {
      php_driver_type* sub_type = PHP_DRIVER_GET_TYPE(current);
      if (!first) smart_str_appendl(string, ", ", 2);
      first = 0;
      smart_str_appendl(string, name, strlen(name));
      smart_str_appendl(string, ":", 1);
      php_driver_type_string(sub_type, string);
    }
    PHP5TO7_ZEND_HASH_FOREACH_END(&type->data.udt.types);
    smart_str_appendl(string, ">", 1);
  }
}

void php_driver_type_string(php_driver_type* type, smart_str* string) {
  switch (type->type) {
#define XX_SCALAR(name, value)                       \
  case value:                                        \
    smart_str_appendl(string, #name, strlen(#name)); \
    break;
    PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR

    case CASS_VALUE_TYPE_LIST:
      collection_string(type, string);
      break;

    case CASS_VALUE_TYPE_MAP:
      map_string(type, string);
      break;

    case CASS_VALUE_TYPE_SET:
      set_string(type, string);
      break;

    case CASS_VALUE_TYPE_TUPLE:
      tuple_string(type, string);
      break;

    case CASS_VALUE_TYPE_UDT:
      user_type_string(type, string);
      break;

    default:
      smart_str_appendl(string, "invalid", 7);
      break;
  }
}

static zval php_driver_type_scalar_new(CassValueType type) {
  zval ztype;
  object_init_ex(&ztype, php_driver_type_scalar_ce);
  php_driver_type* scalar = PHP_DRIVER_GET_TYPE(&ztype);
  scalar->type = type;
  scalar->data_type = cass_data_type_new(type);

  return ztype;
}

const char* php_driver_scalar_type_name(CassValueType type) {
  switch (type) {
#define XX_SCALAR(name, value) \
  case value:                  \
    return #name;
    PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR
    default:
      return "invalid";
  }
}

static void php_driver_varchar_init(INTERNAL_FUNCTION_PARAMETERS) {
  zend_string* string;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(string)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  RETURN_STR(string);
}

static void php_driver_ascii_init(INTERNAL_FUNCTION_PARAMETERS) {
  php_driver_varchar_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void php_driver_boolean_init(INTERNAL_FUNCTION_PARAMETERS) {
  zend_bool value;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_BOOL(value)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  RETURN_BOOL(value);
}

static void php_driver_counter_init(INTERNAL_FUNCTION_PARAMETERS) {
  php_driver_bigint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void php_driver_double_init(INTERNAL_FUNCTION_PARAMETERS) {
  double value;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_DOUBLE(value)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  RETURN_DOUBLE(value);
}

static void php_driver_int_init(INTERNAL_FUNCTION_PARAMETERS) {
  zend_long value;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(value)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  RETURN_LONG(value);
}

static void php_driver_text_init(INTERNAL_FUNCTION_PARAMETERS) {
  php_driver_varchar_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

#define TYPE_INIT_METHOD(t) php_driver_##t##_init

#define TYPES_MAP(XX)                     \
  XX(ascii, CASS_VALUE_TYPE_ASCII)        \
  XX(bigint, CASS_VALUE_TYPE_BIGINT)      \
  XX(smallint, CASS_VALUE_TYPE_SMALL_INT) \
  XX(tinyint, CASS_VALUE_TYPE_TINY_INT)   \
  XX(blob, CASS_VALUE_TYPE_BLOB)          \
  XX(boolean, CASS_VALUE_TYPE_BOOLEAN)    \
  XX(counter, CASS_VALUE_TYPE_COUNTER)    \
  XX(decimal, CASS_VALUE_TYPE_DECIMAL)    \
  XX(double, CASS_VALUE_TYPE_DOUBLE)      \
  XX(duration, CASS_VALUE_TYPE_DURATION)  \
  XX(float, CASS_VALUE_TYPE_FLOAT)        \
  XX(int, CASS_VALUE_TYPE_INT)            \
  XX(text, CASS_VALUE_TYPE_TEXT)          \
  XX(uuid, CASS_VALUE_TYPE_UUID)          \
  XX(varchar, CASS_VALUE_TYPE_VARCHAR)    \
  XX(varint, CASS_VALUE_TYPE_VARINT)      \
  XX(inet, CASS_VALUE_TYPE_INET)

void php_driver_scalar_init(INTERNAL_FUNCTION_PARAMETERS) {
  php_driver_type* self = PHP_DRIVER_GET_TYPE(getThis());

#define XX_SCALAR(name, value)          \
  if (self->type == value) {            \
    TYPE_INIT_METHOD(name)              \
    (INTERNAL_FUNCTION_PARAM_PASSTHRU); \
  }
  TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR
#undef TYPES_MAP
#undef TYPE_INIT_METHOD

  if (self->type == CASS_VALUE_TYPE_TIMESTAMP) {
    zend_long seconds = -1;
    zend_long microseconds = -1;

    // clang-format off
  ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(seconds)
    Z_PARAM_LONG(microseconds)
  ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    auto timestamp = php_scylladb_timestamp_instantiate(return_value);

    if (timestamp == nullptr) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Failed to create Cassandra\\Timestamp");
      return;
    }

    if (php_scylladb_timestamp_initialize(timestamp, seconds, microseconds) != SUCCESS) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Failed to create Timestamp: seconds(%ld) microseconds(%ld)", seconds,
                              microseconds);
    }

    return;
  }

  if (self->type == CASS_VALUE_TYPE_TIME) {
    zend_string* nanosecondsStr = nullptr;
    zend_long nanoseconds = -1;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(0, 1)
      Z_PARAM_OPTIONAL
      Z_PARAM_STR_OR_LONG(nanosecondsStr, nanoseconds)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    auto time = php_scylladb_time_instantiate(return_value);

    if (time == nullptr) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Failed to create Cassandra\\Time");
      return;
    }

    if (php_scylladb_time_initialize(time, nanosecondsStr, nanoseconds) == FAILURE) {
      zend_throw_exception_ex(php_driver_runtime_exception_ce, 0,
                              "Cannot create Cassandra\\Time from invalid value");
    }

    return;
  }

  if (self->type == CASS_VALUE_TYPE_DATE) {
    zend_string* secondsStr = nullptr;
    zend_long seconds = -1;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(0, 1)
      Z_PARAM_OPTIONAL
      Z_PARAM_STR_OR_LONG(secondsStr, seconds)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    auto date = php_scylladb_date_instantiate(return_value);

    if (date == nullptr) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Cannot allocate Cassandra\\Date");
      return;
    }

    if (php_scylladb_date_initialize(date, secondsStr, seconds) == FAILURE) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Cannot create Cassandra\\Date from invalid value");
    }
    return;
  }

  if (self->type == CASS_VALUE_TYPE_TIMEUUID) {
    zend_string* str = nullptr;
    zend_long timestamp = 0;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(0, 1)
      Z_PARAM_OPTIONAL
      Z_PARAM_STR_OR_LONG(str, timestamp)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (php_driver_timeuuid_init(return_value, str, timestamp) != SUCCESS) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Cannot create Timeuuid from invalid value");
    }
  }
}

#define TYPE_CODE(m) type_##m

zval php_driver_type_scalar(CassValueType type) {
  zval result;
  ZVAL_UNDEF(&result);

#define XX_SCALAR(name, value)                                          \
  if (value == type) {                                                  \
    if (Z_ISUNDEF(PHP_DRIVER_G(TYPE_CODE(name)))) {                     \
      PHP_DRIVER_G(TYPE_CODE(name)) = php_driver_type_scalar_new(type); \
    }                                                                   \
    Z_ADDREF_P(&PHP_DRIVER_G(TYPE_CODE(name)));                         \
    return PHP_DRIVER_G(TYPE_CODE(name));                               \
  }
  PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR
  zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "Invalid type");
  return result;
}
#undef TYPE_CODE

zval php_driver_type_map(zval* key_type, zval* value_type) {
  zval ztype;
  php_driver_type* map;
  php_driver_type* sub_type;

  object_init_ex(&ztype, php_driver_type_map_ce);
  map = PHP_DRIVER_GET_TYPE(&ztype);

  if (!Z_ISUNDEF_P(key_type)) {
    sub_type = PHP_DRIVER_GET_TYPE(key_type);
    cass_data_type_add_sub_type(map->data_type, sub_type->data_type);
  }

  if (!Z_ISUNDEF_P(value_type)) {
    sub_type = PHP_DRIVER_GET_TYPE(value_type);
    cass_data_type_add_sub_type(map->data_type, sub_type->data_type);
  }

  map->data.map.key_type = *key_type;
  map->data.map.value_type = *value_type;

  return ztype;
}

zval php_driver_type_map_from_value_types(CassValueType key_type, CassValueType value_type) {
  zval ztype;
  php_driver_type* map;
  php_driver_type* sub_type;

  object_init_ex(&ztype, php_driver_type_map_ce);
  map = PHP_DRIVER_GET_TYPE(&ztype);
  map->data.map.key_type = php_driver_type_scalar(key_type);
  map->data.map.value_type = php_driver_type_scalar(value_type);

  sub_type = PHP_DRIVER_GET_TYPE(&map->data.map.key_type);
  cass_data_type_add_sub_type(map->data_type, sub_type->data_type);
  sub_type = PHP_DRIVER_GET_TYPE(&map->data.map.value_type);
  cass_data_type_add_sub_type(map->data_type, sub_type->data_type);

  return ztype;
}

zval php_driver_type_set(zval* value_type) {
  zval ztype;
  php_driver_type* set;
  php_driver_type* sub_type;

  object_init_ex(&(ztype), php_driver_type_set_ce);
  set = PHP_DRIVER_GET_TYPE(&(ztype));

  if (!Z_ISUNDEF_P(value_type)) {
    sub_type = PHP_DRIVER_GET_TYPE(value_type);
    cass_data_type_add_sub_type(set->data_type, sub_type->data_type);
  }

  set->data.set.value_type = *value_type;

  return ztype;
}

zval php_driver_type_set_from_value_type(CassValueType type) {
  zval ztype;
  php_driver_type* set;
  php_driver_type* sub_type;

  object_init_ex(&(ztype), php_driver_type_set_ce);
  set = PHP_DRIVER_GET_TYPE(&ztype);
  set->data.set.value_type = php_driver_type_scalar(type);

  sub_type = PHP_DRIVER_GET_TYPE(&set->data.set.value_type);
  cass_data_type_add_sub_type(set->data_type, sub_type->data_type);

  return ztype;
}

zval php_driver_type_collection(zval* value_type) {
  zval ztype;
  php_driver_type* collection;
  php_driver_type* sub_type;

  object_init_ex(&ztype, php_driver_type_collection_ce);
  collection = PHP_DRIVER_GET_TYPE(&ztype);

  if (!Z_ISUNDEF_P(value_type)) {
    sub_type = PHP_DRIVER_GET_TYPE(value_type);
    cass_data_type_add_sub_type(collection->data_type, sub_type->data_type);
  }

  collection->data.collection.value_type = *value_type;

  return ztype;
}

zval php_driver_type_collection_from_value_type(CassValueType type) {
  zval ztype;
  php_driver_type* collection;
  php_driver_type* sub_type;

  object_init_ex(&ztype, php_driver_type_collection_ce);
  collection = PHP_DRIVER_GET_TYPE(&ztype);
  collection->data.collection.value_type = php_driver_type_scalar(type);

  sub_type = PHP_DRIVER_GET_TYPE(&collection->data.collection.value_type);
  cass_data_type_add_sub_type(collection->data_type, sub_type->data_type);

  return ztype;
}

zval php_driver_type_tuple() {
  zval ztype;

  object_init_ex(&ztype, php_driver_type_tuple_ce);

  return ztype;
}

zval php_driver_type_user_type() {
  zval ztype;
  php_driver_type* user_type;

  object_init_ex(&ztype, php_driver_type_user_type_ce);
  user_type = PHP_DRIVER_GET_TYPE(&ztype);
  user_type->data_type = cass_data_type_new(CASS_VALUE_TYPE_UDT);

  return ztype;
}

zval php_driver_type_custom(const char* name, size_t name_length) {
  zval ztype;
  php_driver_type* custom;

  object_init_ex(&ztype, php_driver_type_custom_ce);
  custom = PHP_DRIVER_GET_TYPE(&ztype);
  custom->data.custom.class_name = estrndup(name, name_length);

  return ztype;
}

#define EXPECTING_TOKEN(expected)                                                              \
  zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,                         \
                          "Unexpected %s at position %d in string \"%s\", expected " expected, \
                          describe_token(token), ((int)(str - validator) - 1), validator);     \
  return FAILURE;

enum describe_token_type {
  TOKEN_ILLEGAL = 0,
  TOKEN_PAREN_OPEN,
  TOKEN_PAREN_CLOSE,
  TOKEN_COMMA,
  TOKEN_COLON,
  TOKEN_NAME,
  TOKEN_END
};

enum types_parser_state { STATE_CLASS = 0, STATE_AFTER_CLASS, STATE_AFTER_PARENS, STATE_END };

static const char* describe_token(enum describe_token_type token) {
  switch (token) {
    case TOKEN_ILLEGAL:
      return "illegal character";
    case TOKEN_PAREN_OPEN:
      return "opening parenthesis";
    case TOKEN_PAREN_CLOSE:
      return "closing parenthesis";
    case TOKEN_COMMA:
      return "comma";
    case TOKEN_COLON:
      return "colon";
    case TOKEN_NAME:
      return "alphanumeric character";
    case TOKEN_END:
      return "end of string";
    default:
      return "unknown token";
  }
}

static int isletter(char ch) { return isalnum(ch) || ch == '.'; }

static enum describe_token_type next_token(const char* str, size_t len, const char** token_str,
                                  size_t* token_len, const char** str_out, size_t* len_out) {
  enum describe_token_type type;
  unsigned int i = 0;
  char c = str[i];

  if (len == 0) {
    return TOKEN_END;
  }

  if (isalnum(c)) {
    type = TOKEN_NAME;
    while (i < len) {
      if (!isletter(str[i])) {
        break;
      }
      i++;
    }
  } else {
    switch (c) {
      case '\0':
        type = TOKEN_END;
        break;
      case '(':
        type = TOKEN_PAREN_OPEN;
        i++;
        break;
      case ')':
        type = TOKEN_PAREN_CLOSE;
        i++;
        break;
      case ',':
        type = TOKEN_COMMA;
        i++;
        break;
      case ':':
        type = TOKEN_COLON;
        i++;
        break;
      default:
        type = TOKEN_ILLEGAL;
    }
  }

  *token_str = &(str[0]);
  *token_len = i;
  *str_out = &(str[i]);
  *len_out = len - i;

  return type;
}

static struct node_s* php_driver_parse_node_new() {
  struct node_s* node;
  node = static_cast<node_s*>(emalloc(sizeof(struct node_s)));
  node->parent = nullptr;
  node->name = nullptr;
  node->name_length = 0;
  node->first_child = nullptr;
  node->last_child = nullptr;
  node->next_sibling = nullptr;
  node->prev_sibling = nullptr;

  return node;
}

static void php_driver_parse_node_free(struct node_s* node) {
  if (node->first_child) {
    php_driver_parse_node_free(node->first_child);
    node->first_child = nullptr;
  }
  node->last_child = nullptr;

  if (node->next_sibling) {
    php_driver_parse_node_free(node->next_sibling);
    node->next_sibling = nullptr;
  }

  efree(node);
}

static int php_driver_parse_class_name(const char* validator, size_t validator_len,
                                       struct node_s** result) {
  const char* str;
  size_t len;
  const char* token_str;
  size_t token_len;
  enum types_parser_state state;
  enum describe_token_type token;
  struct node_s* root;
  struct node_s* node;
  struct node_s* child;

  token_str = NULL;
  token_len = 0;
  state = STATE_CLASS;
  str = validator;
  len = validator_len;
  root = php_driver_parse_node_new();
  node = root;

  while (1) {
    token = next_token(str, len, &token_str, &token_len, &str, &len);

    if (token == TOKEN_ILLEGAL) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Illegal character \"%c\" at position %d in \"%s\"", *token_str,
                              ((int)(str - validator) - 1), validator);
      php_driver_parse_node_free(root);
      return FAILURE;
    }

    if (state == STATE_AFTER_PARENS) {
      if (token == TOKEN_COMMA) {
        if (node->parent == NULL) {
          EXPECTING_TOKEN("end of string");
        }
        state = STATE_CLASS;

        child = php_driver_parse_node_new();
        child->parent = node->parent;
        child->prev_sibling = node;
        node->next_sibling = child;
        node->parent->last_child = child;

        node = child;
        continue;
      } else if (token == TOKEN_PAREN_CLOSE) {
        if (node->parent == NULL) {
          EXPECTING_TOKEN("end of string");
        }

        node = node->parent;
        continue;
      } else if (token == TOKEN_END) {
        break;
      } else {
        EXPECTING_TOKEN("a comma, a closing parenthesis or an end of string");
      }
    }

    if (state == STATE_AFTER_CLASS) {
      if (token == TOKEN_PAREN_OPEN) {
        state = STATE_CLASS;

        child = php_driver_parse_node_new();
        child->parent = node;

        if (node->first_child == NULL) {
          node->first_child = child;
        }

        if (node->last_child) {
          node->last_child->next_sibling = child;
        }

        child->prev_sibling = node->last_child;
        node->last_child = child;

        node = child;
        continue;
      } else if (token == TOKEN_COMMA || token == TOKEN_COLON) {
        state = STATE_CLASS;

        child = php_driver_parse_node_new();
        child->parent = node->parent;
        child->prev_sibling = node;
        node->next_sibling = child;
        node->parent->last_child = child;

        node = child;
        continue;
      } else if (token == TOKEN_PAREN_CLOSE) {
        state = STATE_AFTER_PARENS;

        node = node->parent;
        continue;
      } else if (token == TOKEN_END) {
        break;
      } else {
        php_driver_parse_node_free(root);
        EXPECTING_TOKEN("opening/closing parenthesis or comma");
      }
    }

    if (state == STATE_CLASS) {
      if (token != TOKEN_NAME) {
        php_driver_parse_node_free(root);
        EXPECTING_TOKEN("fully qualified class name");
      }
      state = STATE_AFTER_CLASS;

      node->name = token_str;
      node->name_length = token_len;
    }
  }

  *result = root;
  return SUCCESS;
}

static CassValueType php_driver_lookup_type(struct node_s* node) {
  if (strncmp("org.apache.cassandra.db.marshal.AsciiType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_ASCII;
  }

  if (strncmp("org.apache.cassandra.db.marshal.LongType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_BIGINT;
  }

  if (strncmp("org.apache.cassandra.db.marshal.ShortType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_SMALL_INT;
  }

  if (strncmp("org.apache.cassandra.db.marshal.ByteType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_TINY_INT;
  }

  if (strncmp("org.apache.cassandra.db.marshal.BytesType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_BLOB;
  }

  if (strncmp("org.apache.cassandra.db.marshal.BooleanType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_BOOLEAN;
  }

  if (strncmp("org.apache.cassandra.db.marshal.CounterColumnType", node->name, node->name_length) ==
      0) {
    return CASS_VALUE_TYPE_COUNTER;
  }

  if (strncmp("org.apache.cassandra.db.marshal.DecimalType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_DECIMAL;
  }

  if (strncmp("org.apache.cassandra.db.marshal.DurationType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_DURATION;
  }

  if (strncmp("org.apache.cassandra.db.marshal.DoubleType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_DOUBLE;
  }

  if (strncmp("org.apache.cassandra.db.marshal.FloatType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_FLOAT;
  }

  if (strncmp("org.apache.cassandra.db.marshal.InetAddressType", node->name, node->name_length) ==
      0) {
    return CASS_VALUE_TYPE_INET;
  }

  if (strncmp("org.apache.cassandra.db.marshal.Int32Type", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_INT;
  }

  if (strncmp("org.apache.cassandra.db.marshal.UTF8Type", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_VARCHAR;
  }

  if (strncmp("org.apache.cassandra.db.marshal.TimestampType", node->name, node->name_length) ==
          0 ||
      strncmp("org.apache.cassandra.db.marshal.DateType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_TIMESTAMP;
  }

  if (strncmp("org.apache.cassandra.db.marshal.SimpleDateType", node->name, node->name_length) ==
      0) {
    return CASS_VALUE_TYPE_DATE;
  }

  if (strncmp("org.apache.cassandra.db.marshal.TimeType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_TIME;
  }

  if (strncmp("org.apache.cassandra.db.marshal.UUIDType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_UUID;
  }

  if (strncmp("org.apache.cassandra.db.marshal.IntegerType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_VARINT;
  }

  if (strncmp("org.apache.cassandra.db.marshal.TimeUUIDType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_TIMEUUID;
  }

  if (strncmp("org.apache.cassandra.db.marshal.MapType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_MAP;
  }

  if (strncmp("org.apache.cassandra.db.marshal.SetType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_SET;
  }

  if (strncmp("org.apache.cassandra.db.marshal.ListType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_LIST;
  }

  if (strncmp("org.apache.cassandra.db.marshal.TupleType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_TUPLE;
  }

  if (strncmp("org.apache.cassandra.db.marshal.UserType", node->name, node->name_length) == 0) {
    return CASS_VALUE_TYPE_UDT;
  }

  return CASS_VALUE_TYPE_CUSTOM;
}

static void php_driver_node_dump_to(struct node_s* node, smart_str* text) {
  smart_str_appendl(text, node->name, node->name_length);

  if (node->first_child) {
    smart_str_appendl(text, "(", 1);
    php_driver_node_dump_to(node->first_child, text);
    smart_str_appendl(text, ")", 1);
  }

  if (node->next_sibling) {
    smart_str_appendl(text, ", ", 2);
    php_driver_node_dump_to(node->next_sibling, text);
  }
}

static zval php_driver_create_type(struct node_s* node) {
  CassValueType type = CASS_VALUE_TYPE_UNKNOWN;

  /* Skip wrapper types */
  while (node && (strncmp("org.apache.cassandra.db.marshal.FrozenType", node->name,
                          node->name_length) == 0 ||
                  strncmp("org.apache.cassandra.db.marshal.ReversedType", node->name,
                          node->name_length) == 0 ||
                  strncmp("org.apache.cassandra.db.marshal.CompositeType", node->name,
                          node->name_length) == 0)) {
    node = node->first_child;
  }

  if (node) {
    type = php_driver_lookup_type(node);
  }

  if (type == CASS_VALUE_TYPE_UNKNOWN) {
    zval undef;
    ZVAL_UNDEF(&undef);
    return undef;
  }

  if (type == CASS_VALUE_TYPE_CUSTOM) {
    zval ztype;
    smart_str class_name = {NULL, 0};
    php_driver_node_dump_to(node, &class_name);
    ztype = php_driver_type_custom(PHP5TO7_SMART_STR_VAL(class_name),
                                   PHP5TO7_SMART_STR_LEN(class_name));
    smart_str_free(&class_name);
    return ztype;
  } else if (type == CASS_VALUE_TYPE_MAP) {
    zval key_type;
    zval value_type;

    if (node->first_child) {
      key_type = php_driver_create_type(node->first_child);
      value_type = php_driver_create_type(node->first_child->next_sibling);
    } else {
      ZVAL_UNDEF(&key_type);
      ZVAL_UNDEF(&value_type);
    }
    return php_driver_type_map(&key_type, &value_type);
  } else if (type == CASS_VALUE_TYPE_LIST) {
    zval value_type;
    if (node->first_child) {
      value_type = php_driver_create_type(node->first_child);
    } else {
      ZVAL_UNDEF(&value_type);
    }
    return php_driver_type_collection(&value_type);
  } else if (type == CASS_VALUE_TYPE_SET) {
    zval value_type;
    if (node->first_child) {
      value_type = php_driver_create_type(node->first_child);
    } else {
      ZVAL_UNDEF(&value_type);
    }
    return php_driver_type_set(&value_type);
  } else if (type == CASS_VALUE_TYPE_TUPLE) {
    return php_driver_tuple_from_node(node);
  } else if (type == CASS_VALUE_TYPE_UDT) {
    return php_driver_user_type_from_node(node);
  }

  return php_driver_type_scalar(type);
}

int php_driver_parse_column_type(const char* validator, size_t validator_len, int* reversed_out,
                                 int* frozen_out, zval* type_out) {
  struct node_s* root;
  struct node_s* node = nullptr;
  cass_bool_t reversed = cass_false;
  cass_bool_t frozen = cass_false;

  if (php_driver_parse_class_name(validator, validator_len, &root) == FAILURE) {
    return FAILURE;
  }

  node = root;

  while (node) {
    if (strncmp("org.apache.cassandra.db.marshal.ReversedType", node->name, node->name_length) ==
        0) {
      reversed = cass_true;
      node = node->first_child;
      continue;
    }

    if (strncmp("org.apache.cassandra.db.marshal.FrozenType", node->name, node->name_length) == 0) {
      frozen = cass_true;
      node = node->first_child;
      continue;
    }

    if (strncmp("org.apache.cassandra.db.marshal.CompositeType", node->name, node->name_length) ==
        0) {
      node = node->first_child;
      continue;
    }

    break;
  }

  if (node == nullptr) {
    php_driver_parse_node_free(root);
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "Invalid type");
    return FAILURE;
  }

  *reversed_out = reversed;
  *frozen_out = frozen;
  *type_out = php_driver_create_type(node);

  php_driver_parse_node_free(root);

  return SUCCESS;
}

