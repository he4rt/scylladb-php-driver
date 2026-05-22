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

#include "php_driver.h"
#include "php_driver_types.h"
#include "Type/TypeFactory.h"

#include "src/Type/Tuple.h"
#include "src/Type/UserType.h"
BEGIN_EXTERN_C()
#include "Type/Type_arginfo.h"
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

zend_class_entry *php_driver_type_ce = NULL;

#define XX_SCALAR_METHOD(name, value) ZEND_METHOD(Cassandra_Type, name) \
{ \
  zval ztype; \
  ZEND_PARSE_PARAMETERS_NONE(); \
  ztype = php_driver_type_scalar(value ); \
  RETURN_ZVAL(&ztype, 1, 1); \
}

PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR_METHOD)
#undef XX_SCALAR_METHOD

ZEND_METHOD(Cassandra_Type, collection)
{
  zval ztype;
  zval *value_type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(value_type, php_driver_type_ce)
  ZEND_PARSE_PARAMETERS_END();

  if (!php_driver_type_validate(value_type, "type" )) {
    return;
  }

  ztype  = php_driver_type_collection(value_type );
  Z_ADDREF_P(value_type);
  RETURN_ZVAL(&ztype, 0, 1);
}

ZEND_METHOD(Cassandra_Type, tuple)
{
  zval ztype;
  php_driver_type *type;
  zval* args = NULL;
  int argc = 0, i;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  for (i = 0; i < argc; ++i) {
    zval *sub_type = &args[i];
    if (!php_driver_type_validate(sub_type, "type" )) {
      return;
    }
  }

  ztype = php_driver_type_tuple();
  type = PHP_DRIVER_GET_TYPE(&ztype);

  for (i = 0; i < argc; ++i) {
    zval *sub_type = &args[i];
    if (php_driver_type_tuple_add(type, sub_type )) {
      Z_ADDREF_P(sub_type);
    } else {
      break;
    }
  }

  RETURN_ZVAL(&ztype, 0, 1);
}

ZEND_METHOD(Cassandra_Type, userType)
{
  zval ztype;
  php_driver_type *type;
  zval* args = NULL;
  int argc = 0, i;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  if (argc % 2 == 1) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                            "Not enough name/type pairs, user types can only be created " \
                            "from an even number of name/type pairs, where each odd " \
                            "argument is a name and each even argument is a type, " \
                            "e.g userType(name, type, name, type, name, type)");
    return;
  }

  for (i = 0; i < argc; i += 2) {
    zval *name = &args[i];
    zval *sub_type = &args[i + 1];
    if (Z_TYPE_P(name) != IS_STRING) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0 ,
                              "Argument %d is not a string", i + 1);
      return;
    }
    if (!php_driver_type_validate(sub_type, "type" )) {
      return;
    }
  }

  ztype = php_driver_type_user_type();
  type = PHP_DRIVER_GET_TYPE(&ztype);

  for (i = 0; i < argc; i += 2) {
    zval *name = &args[i];
    zval *sub_type = &args[i + 1];
    if (php_driver_type_user_type_add(type,
                                         Z_STRVAL_P(name), Z_STRLEN_P(name),
                                         sub_type )) {
      Z_ADDREF_P(sub_type);
    } else {
      break;
    }
  }

  RETURN_ZVAL(&ztype, 0, 1);
}

ZEND_METHOD(Cassandra_Type, set)
{
  zval ztype;
  zval *value_type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(value_type, php_driver_type_ce)
  ZEND_PARSE_PARAMETERS_END();

  if (!php_driver_type_validate(value_type, "type" )) {
    return;
  }

  ztype = php_driver_type_set(value_type );
  Z_ADDREF_P(value_type);
  RETURN_ZVAL(&ztype, 0, 1);
}

ZEND_METHOD(Cassandra_Type, map)
{
  zval ztype;
  zval *key_type;
  zval *value_type;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_OBJECT_OF_CLASS(key_type, php_driver_type_ce)
    Z_PARAM_OBJECT_OF_CLASS(value_type, php_driver_type_ce)
  ZEND_PARSE_PARAMETERS_END();

  if (!php_driver_type_validate(key_type, "keyType" )) {
    return;
  }

  if (!php_driver_type_validate(value_type, "valueType" )) {
    return;
  }

  ztype = php_driver_type_map(key_type, value_type );
  Z_ADDREF_P(key_type);
  Z_ADDREF_P(value_type);
  RETURN_ZVAL(&ztype, 0, 1);
}

void php_driver_define_Type()
{
  php_driver_type_ce = register_class_Cassandra_Type();
}
END_EXTERN_C()