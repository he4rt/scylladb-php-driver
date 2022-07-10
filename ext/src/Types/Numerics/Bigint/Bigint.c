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

#include <CassandraDriverAPI.h>
#include <Exception/Exceptions.h>
#include <Types/Numerics/Numerics.h>

#include "Bigint.h"
#include "Bigint_arginfo.h"

#include "util/math.h"
#include "util/types.h"

PHP_DRIVER_API zend_class_entry* phpDriverBigintCe = NULL;

static ZEND_RESULT_CODE
ToDouble(zval* result, PhpDriverNumeric* bigint)
{
  ZVAL_DOUBLE(result, (double) bigint->data.bigint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
ToLong(zval* result, PhpDriverNumeric* bigint)
{
  if (bigint->data.bigint.value < (cass_int64_t) ZEND_LONG_MIN) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value is too small");
    return FAILURE;
  }

  if (bigint->data.bigint.value > (cass_int64_t) ZEND_LONG_MAX) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value is too big");
    return FAILURE;
  }

  ZVAL_LONG(result, (zend_long) bigint->data.bigint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
ToString(zval* result, PhpDriverNumeric* bigint)
{
  smart_str string = { 0 };
  smart_str_append_long(&string, bigint->data.bigint.value);
  smart_str_0(&string);

  ZVAL_STR(result, string.s);
  return SUCCESS;
}

void
PhpDriverBigintInit(PhpDriverNumeric* self, zval* value)
{
  switch (Z_TYPE_P(value)) {
  case IS_LONG: {
    self->data.bigint.value = (cass_int64_t) Z_LVAL_P(value);
    break;
  }
  case IS_STRING: {
    int parsed = php_driver_parse_bigint(
      Z_STRVAL_P(value),
      Z_STRLEN_P(value),
      &self->data.bigint.value);

    if (!parsed) {
      return;
    }

    break;
  }
  case IS_DOUBLE: {
    double double_value = Z_DVAL_P(value);

    // TODO: Fix overflow
    if (double_value > INT64_MAX || double_value < INT64_MIN) {
      zend_throw_exception_ex(
        spl_ce_RangeException, 0,
        "value must be between " LL_FORMAT " and " LL_FORMAT ", %g given",
        INT64_MIN, INT64_MAX, double_value);
      return;
    }

    self->data.bigint.value = (cass_int64_t) double_value;
    break;
  }
  case IS_OBJECT: {
    if (instanceof_function(Z_OBJCE_P(value), phpDriverBigintCe)) {
      PhpDriverNumeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(value);
      self->data.bigint.value  = bigint->data.bigint.value;
    }

    break;
  }
  default: {
    INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }
  }
}

/* {{{ Bigint::__construct(string) */
ZEND_METHOD(Cassandra_Bigint, __construct)
{
  zval* value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  PhpDriverBigintInit(self, value);
}
/* }}} */

/* {{{ Bigint::__toString() */
ZEND_METHOD(Cassandra_Bigint, __toString)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToString(return_value, self);
}
/* }}} */

/* {{{ Bigint::type() */
ZEND_METHOD(Cassandra_Bigint, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_BIGINT);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Bigint::value() */
ZEND_METHOD(Cassandra_Bigint, value)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToString(return_value, self);
}
/* }}} */

/* {{{ Bigint::add() */
ZEND_METHOD(Cassandra_Bigint, add)
{
  zval* num;
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* bigint;
  PhpDriverNumeric* result;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverBigintCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverBigintCe);
  result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.bigint.value = self->data.bigint.value + bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::sub() */
ZEND_METHOD(Cassandra_Bigint, sub)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverBigintCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.bigint.value = self->data.bigint.value - bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::mul() */
ZEND_METHOD(Cassandra_Bigint, mul)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverBigintCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.bigint.value = self->data.bigint.value * bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::div() */
ZEND_METHOD(Cassandra_Bigint, div)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverBigintCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  if (bigint->data.bigint.value == 0) {
    zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot divide by zero");
    return;
  }

  result->data.bigint.value = self->data.bigint.value / bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::mod() */
ZEND_METHOD(Cassandra_Bigint, mod)
{
  zval* num;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverBigintCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverBigintCe);
  result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  if (bigint->data.bigint.value == 0) {
    zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot modulo by zero");
    return;
  }

  result->data.bigint.value = self->data.bigint.value % bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::abs() */
ZEND_METHOD(Cassandra_Bigint, abs)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.bigint.value == INT64_MIN) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* result  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.bigint.value = self->data.bigint.value < 0 ? -self->data.bigint.value : self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::neg() */
ZEND_METHOD(Cassandra_Bigint, neg)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.bigint.value = -self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::sqrt() */
ZEND_METHOD(Cassandra_Bigint, sqrt)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.bigint.value < 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0,
                            "Cannot take a square root of a negative number");
    return;
  }

  object_init_ex(return_value, phpDriverBigintCe);

  PhpDriverNumeric* result  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.bigint.value = (cass_int64_t) sqrt((double) self->data.bigint.value);
}
/* }}} */

/* {{{ Bigint::toInt() */
ZEND_METHOD(Cassandra_Bigint, toInt)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToLong(return_value, self);
}
/* }}} */

/* {{{ Bigint::toDouble() */
ZEND_METHOD(Cassandra_Bigint, toDouble)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToDouble(return_value, self);
}
/* }}} */

/* {{{ Bigint::min() */
ZEND_METHOD(Cassandra_Bigint, min)
{
  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* bigint  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  bigint->data.bigint.value = INT64_MIN;
}
/* }}} */

/* {{{ Bigint::max() */
ZEND_METHOD(Cassandra_Bigint, max)
{
  object_init_ex(return_value, phpDriverBigintCe);
  PhpDriverNumeric* bigint  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  bigint->data.bigint.value = INT64_MAX;
}
/* }}} */

static php_driver_value_handlers phpDriverBigintHandlers;

static HashTable*
PhpDriverBigintGc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
PhpDriverBigintProperties(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props       = zend_std_get_properties(object);

  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_BIGINT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  zval value;

  ToString(&value, self);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), &value, sizeof(zval));

  return props;
}

static int
PhpDriverBigintCompare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  PhpDriverNumeric* bigint1 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj1);
  PhpDriverNumeric* bigint2 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj2);

  if (bigint1->data.bigint.value == bigint2->data.bigint.value) {
    return 0;
  }

  if (bigint1->data.bigint.value < bigint2->data.bigint.value) {
    return -1;
  }

  return 1;
}

static uint32_t
PhpDriverBigintHashValue(zval* obj)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj);
  return (uint32_t) (self->data.bigint.value ^ (self->data.bigint.value >> 32));
}

static int
PhpDriverBigintCast(
  zend_object* object,
  zval* retval,
  int type)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  switch (type) {
  case IS_LONG:
    return ToLong(retval, self);
  case IS_DOUBLE:
    return ToDouble(retval, self);
  case IS_STRING:
    return ToString(retval, self);
  default:
    return FAILURE;
  }
}

void
PhpDriverBigintFree(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
PhpDriverBigintNew(zend_class_entry* ce)
{
  PhpDriverNumeric* self = make(PhpDriverNumeric);

  self->type = PHP_DRIVER_BIGINT;
  zend_object_std_init(&self->zval, ce);

  self->zval.handlers = &phpDriverBigintHandlers.std;

  return &self->zval;
}

void
PhpDriverDefineBigint(zend_class_entry* valueInterface, zend_class_entry* numericInterface)
{
  phpDriverBigintCe                = register_class_Cassandra_Bigint(valueInterface, numericInterface);
  phpDriverBigintCe->create_object = PhpDriverBigintNew;

  memcpy(&phpDriverBigintHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  phpDriverBigintHandlers.std.get_properties = PhpDriverBigintProperties;
  phpDriverBigintHandlers.std.get_gc         = PhpDriverBigintGc;
  phpDriverBigintHandlers.std.compare        = PhpDriverBigintCompare;
  phpDriverBigintHandlers.std.cast_object    = PhpDriverBigintCast;
  phpDriverBigintHandlers.hash_value         = PhpDriverBigintHashValue;
  phpDriverBigintHandlers.std.free_obj       = PhpDriverBigintFree;
  phpDriverBigintHandlers.std.offset         = XtOffsetOf(PhpDriverNumeric, zval);

  phpDriverBigintHandlers.std.clone_obj = NULL;
}
