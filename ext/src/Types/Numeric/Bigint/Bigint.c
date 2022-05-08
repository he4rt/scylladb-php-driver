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

#include <Types/Numeric/Numeric.h>

#include "Bigint.h"
#include "Bigint_arginfo.h"

#include "util/math.h"
#include "util/types.h"

zend_class_entry* php_driver_bigint_ce = NULL;

static ZEND_RESULT_CODE
to_double(zval* result, php_driver_numeric* bigint)
{
  ZVAL_DOUBLE(result, (double) bigint->data.bigint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
to_long(zval* result, php_driver_numeric* bigint)
{
  if (bigint->data.bigint.value < (cass_int64_t) ZEND_LONG_MIN) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too small");
    return FAILURE;
  }

  if (bigint->data.bigint.value > (cass_int64_t) ZEND_LONG_MAX) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too big");
    return FAILURE;
  }

  ZVAL_LONG(result, (zend_long) bigint->data.bigint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
to_string(zval* result, php_driver_numeric* bigint)
{
  smart_str string = { 0 };
  smart_str_append_long(&string, bigint->data.bigint.value);
  smart_str_0(&string);

  ZVAL_STR(result, string.s);
  return SUCCESS;
}

void
php_driver_bigint_init(php_driver_numeric* self, zval* value)
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
        php_driver_range_exception_ce, 0,
        "value must be between " LL_FORMAT " and " LL_FORMAT ", %g given",
        INT64_MIN, INT64_MAX, double_value);
      return;
    }

    self->data.bigint.value = (cass_int64_t) double_value;
    break;
  }
  case IS_OBJECT: {
    if (instanceof_function(Z_OBJCE_P(value), php_driver_bigint_ce)) {
      php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(value);
      self->data.bigint.value    = bigint->data.bigint.value;
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

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  php_driver_bigint_init(self, value);
}
/* }}} */

/* {{{ Bigint::__toString() */
ZEND_METHOD(Cassandra_Bigint, __toString)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  to_string(return_value, self);
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
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  to_string(return_value, self);
}
/* }}} */

/* {{{ Bigint::add() */
ZEND_METHOD(Cassandra_Bigint, add)
{
  zval* num;
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* bigint;
  php_driver_numeric* result;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_bigint_ce);
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

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  php_driver_numeric* self   = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

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

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  php_driver_numeric* self   = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

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

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  php_driver_numeric* self   = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  if (bigint->data.bigint.value == 0) {
    zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
    return;
  }

  result->data.bigint.value = self->data.bigint.value / bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::mod() */
ZEND_METHOD(Cassandra_Bigint, mod)
{
  zval* num;
  php_driver_numeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }

  php_driver_numeric* self   = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_bigint_ce);
  result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  if (bigint->data.bigint.value == 0) {
    zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot modulo by zero");
    return;
  }

  result->data.bigint.value = self->data.bigint.value % bigint->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::abs() */
ZEND_METHOD(Cassandra_Bigint, abs)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.bigint.value == INT64_MIN) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.bigint.value  = self->data.bigint.value < 0 ? -self->data.bigint.value : self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::neg() */
ZEND_METHOD(Cassandra_Bigint, neg)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.bigint.value = -self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::sqrt() */
ZEND_METHOD(Cassandra_Bigint, sqrt)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.bigint.value < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0,
                            "Cannot take a square root of a negative number");
    return;
  }

  object_init_ex(return_value, php_driver_bigint_ce);

  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.bigint.value  = (cass_int64_t) sqrt((double) self->data.bigint.value);
}
/* }}} */

/* {{{ Bigint::toInt() */
ZEND_METHOD(Cassandra_Bigint, toInt)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  to_long(return_value, self);
}
/* }}} */

/* {{{ Bigint::toDouble() */
ZEND_METHOD(Cassandra_Bigint, toDouble)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  to_double(return_value, self);
}
/* }}} */

/* {{{ Bigint::min() */
ZEND_METHOD(Cassandra_Bigint, min)
{
  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  bigint->data.bigint.value  = INT64_MIN;
}
/* }}} */

/* {{{ Bigint::max() */
ZEND_METHOD(Cassandra_Bigint, max)
{
  object_init_ex(return_value, php_driver_bigint_ce);
  php_driver_numeric* bigint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  bigint->data.bigint.value  = INT64_MAX;
}
/* }}} */

static php_driver_value_handlers php_driver_bigint_handlers;

static HashTable*
php_driver_bigint_gc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
php_driver_bigint_properties(zend_object* object)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props         = zend_std_get_properties(object);

  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_BIGINT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  zval value;

  to_string(&value, self);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), &value, sizeof(zval));

  return props;
}

static int
php_driver_bigint_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  php_driver_numeric* bigint1 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj1);
  php_driver_numeric* bigint2 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj2);

  if (bigint1->data.bigint.value == bigint2->data.bigint.value) {
    return 0;
  }

  if (bigint1->data.bigint.value < bigint2->data.bigint.value) {
    return -1;
  }

  return 1;
}

static uint32_t
php_driver_bigint_hash_value(zval* obj)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj);
  return (uint32_t) (self->data.bigint.value ^ (self->data.bigint.value >> 32));
}

static int
php_driver_bigint_cast(
  zend_object* object,
  zval* retval,
  int type)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  switch (type) {
  case IS_LONG:
    return to_long(retval, self);
  case IS_DOUBLE:
    return to_double(retval, self);
  case IS_STRING:
    return to_string(retval, self);
  default:
    return FAILURE;
  }
}

void
php_driver_bigint_free(zend_object* object)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_bigint_new(zend_class_entry* ce)
{
  php_driver_numeric* self = make(php_driver_numeric);

  self->type = PHP_DRIVER_BIGINT;
  zend_object_std_init(&self->zval, ce);

  self->zval.handlers = &php_driver_bigint_handlers.std;

  return &self->zval;
}

void
php_driver_define_Bigint(zend_class_entry* value_interface, zend_class_entry* numeric_interface)
{
  php_driver_bigint_ce                = register_class_Cassandra_Bigint(value_interface, numeric_interface);
  php_driver_bigint_ce->create_object = php_driver_bigint_new;

  memcpy(&php_driver_bigint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_bigint_handlers.std.get_properties = php_driver_bigint_properties;
  php_driver_bigint_handlers.std.get_gc         = php_driver_bigint_gc;
  php_driver_bigint_handlers.std.compare        = php_driver_bigint_compare;
  php_driver_bigint_handlers.std.cast_object    = php_driver_bigint_cast;
  php_driver_bigint_handlers.hash_value         = php_driver_bigint_hash_value;
  php_driver_bigint_handlers.std.free_obj       = php_driver_bigint_free;
  php_driver_bigint_handlers.std.offset         = XtOffsetOf(php_driver_numeric, zval);

  php_driver_bigint_handlers.std.clone_obj = NULL;
}
