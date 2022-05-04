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
#include "util/hash.h"
#include "util/math.h"
#include "util/types.h"

#include <ext/spl/spl_exceptions.h>
#include <float.h>
#include <gmp.h>
#include <math.h>

#include <Numeric/Numeric.h>

zend_class_entry* php_driver_decimal_ce = NULL;
/*
 * IEEE 754 double precision floating point representation:
 *
 *  S   EEEEEEEEEEE  MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * [63][  62 - 52  ][                    51 - 0                          ]
 *
 * S = sign bit
 * E = exponent
 * M = mantissa
 */
#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_MANTISSA_MASK (cass_int64_t)((1LL << DOUBLE_MANTISSA_BITS) - 1)
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_EXPONENT_MASK (cass_int64_t)((1LL << DOUBLE_EXPONENT_BITS) - 1)

static zend_always_inline void
to_mpf(mpf_t result, php_driver_numeric* decimal)
{
  mpf_t scale_factor;
  long scale;
  /* result = unscaled * pow(10, -scale) */
  mpf_set_z(result, decimal->data.decimal.value);

  scale = decimal->data.decimal.scale;
  mpf_init_set_si(scale_factor, 10);
  mpf_pow_ui(scale_factor, scale_factor, scale < 0 ? -scale : scale);

  if (scale > 0) {
    mpf_ui_div(scale_factor, 1, scale_factor);
  }

  mpf_mul(result, result, scale_factor);

  mpf_clear(scale_factor);
}

static zend_always_inline void
from_double(php_driver_numeric* result, double value)
{
  int denormal;
  char mantissa_str[32];
  cass_int64_t raw, mantissa, exponent;

  // Copy the bits of value into an int64 so that we can do bit manipulations on it.
  memcpy(&raw, &value, 8);

  mantissa = raw & DOUBLE_MANTISSA_MASK;
  exponent = (raw >> DOUBLE_MANTISSA_BITS) & DOUBLE_EXPONENT_MASK;

  /* This exponent is offset using 1023 unless it's a denormal value then its value
   * is the minimum value -1022
   */
  if (exponent == 0) {
    /* If the exponent is a zero then we have a denormal (subnormal) number. These are numbers
     * that represent small values around 0.0. The mantissa has the form of 0.xxxxxxxx...
     *
     * http://en.wikipedia.org/wiki/Denormal_number
     */
    denormal = 1;
    exponent = -1022;
  } else {
    /* Normal number The mantissa has the form of 1.xxxxxxx... */
    denormal = 0;
    exponent -= 1023;
  }

  /* Move the factional parts in the mantissa to the exponent. The significand
   * represents fractional parts:
   *
   * S = 1 + B51 * 2^-51 + B50 * 2^-52 ... + B0
   *
   */
  exponent -= DOUBLE_MANTISSA_BITS;

  if (!denormal) {
    /* Normal numbers have an implied one i.e. 1.xxxxxx... */
    mantissa |= (1LL << DOUBLE_MANTISSA_BITS);
  }

  /* Remove trailing zeros and move them to the exponent */
  while (exponent < 0 && (mantissa & 1) == 0) {
    ++exponent;
    mantissa >>= 1;
  }

  /* There isn't any "long long" setter method  */
  sprintf(mantissa_str, "%ld", mantissa);
  mpz_set_str(result->data.decimal.value, mantissa_str, 10);

  /* Change the sign if negative */
  if (raw < 0) {
    mpz_neg(result->data.decimal.value, result->data.decimal.value);
  }

  if (exponent < 0) {
    /* Convert from pow(2, exponent) to pow(10, exponent):
     *
     * mantissa * pow(2, exponent) equals
     * mantissa * (pow(10, exponent) / pow(5, exponent))
     */
    mpz_t pow_5;
    mpz_init(pow_5);
    mpz_ui_pow_ui(pow_5, 5, -exponent);
    mpz_mul(result->data.decimal.value, result->data.decimal.value, pow_5);
    mpz_clear(pow_5);
    result->data.decimal.scale = -exponent;
  } else {
    mpz_mul_2exp(result->data.decimal.value, result->data.decimal.value, exponent);
    result->data.decimal.scale = 0;
  }
}

static zend_always_inline ZEND_RESULT_CODE
to_double(zval* result, php_driver_numeric* decimal)
{
  mpf_t value;
  mpf_init(value);
  to_mpf(value, decimal);

  if (mpf_cmp_d(value, -DBL_MAX) < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too small");
    mpf_clear(value);
    return FAILURE;
  }

  if (mpf_cmp_d(value, DBL_MAX) > 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too big");
    mpf_clear(value);
    return FAILURE;
  }

  ZVAL_DOUBLE(result, mpf_get_d(value));
  mpf_clear(value);
  return SUCCESS;
}

static zend_always_inline ZEND_RESULT_CODE
to_long(zval* result, php_driver_numeric* decimal)
{
  mpf_t value;
  mpf_init(value);
  to_mpf(value, decimal);

  if (mpf_cmp_si(value, LONG_MIN) < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too small");
    mpf_clear(value);
    return FAILURE;
  }

  if (mpf_cmp_si(value, LONG_MAX) > 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too big");
    mpf_clear(value);
    return FAILURE;
  }

  ZVAL_LONG(result, mpf_get_si(value));

  mpf_clear(value);
  return SUCCESS;
}

static zend_always_inline ZEND_RESULT_CODE
to_string(zval* result, php_driver_numeric* decimal)
{
  char* string;
  int string_len;
  php_driver_format_decimal(decimal->data.decimal.value, decimal->data.decimal.scale, &string, &string_len);

  PHP5TO7_ZVAL_STRINGL(result, string, string_len);
  efree(string);

  return SUCCESS;
}

static zend_always_inline void
align_decimals(php_driver_numeric* lhs, php_driver_numeric* rhs)
{
  mpz_t pow_10;
  mpz_init(pow_10);
  if (lhs->data.decimal.scale < rhs->data.decimal.scale) {
    mpz_ui_pow_ui(pow_10, 10, rhs->data.decimal.scale - lhs->data.decimal.scale);
    mpz_mul(lhs->data.decimal.value, lhs->data.decimal.value, pow_10);
  } else if (lhs->data.decimal.scale > rhs->data.decimal.scale) {
    mpz_ui_pow_ui(pow_10, 10, lhs->data.decimal.scale - rhs->data.decimal.scale);
    mpz_mul(rhs->data.decimal.value, rhs->data.decimal.value, pow_10);
  }
  mpz_clear(pow_10);
}

zend_always_inline void
php_driver_decimal_init(php_driver_numeric* self, zval* value)
{
  switch (Z_TYPE_P(value)) {
  case IS_LONG: {
    mpz_set_si(self->data.decimal.value, Z_LVAL_P(value));
    self->data.decimal.scale = 0;
  }
  case IS_DOUBLE: {
    double val = Z_DVAL_P(value);
    if (zend_isnan(val) || zend_isinf(val)) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Value of NaN or +/- infinity is not supported");
      return;
    }

    from_double(self, val);
  }
  case IS_STRING: {
    php_driver_parse_decimal(
      Z_STRVAL_P(value),
      Z_STRLEN_P(value),
      &self->data.decimal.value,
      &self->data.decimal.scale TSRMLS_CC);
  }
  case IS_OBJECT: {
    if (instanceof_function(Z_OBJCE_P(value), php_driver_decimal_ce)) {
      php_driver_numeric* decimal = PHP_DRIVER_GET_NUMERIC_ZVAL(value);
      mpz_set(self->data.decimal.value, decimal->data.decimal.value);
      self->data.decimal.scale = decimal->data.decimal.scale;
    }
  }
  default: {
    INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }
  }
}

/* {{{ Decimal::__construct(string) */
ZEND_METHOD(Cassandra_Decimal, __construct)
{
  zval* value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  php_driver_decimal_init(self, value);
}
/* }}} */

/* {{{ Decimal::__toString() */
ZEND_METHOD(Cassandra_Decimal, __toString)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  to_string(return_value, self);
}
/* }}} */

/* {{{ Decimal::type() */
ZEND_METHOD(Cassandra_Decimal, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_DECIMAL);

  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Decimal::value() */
ZEND_METHOD(Cassandra_Decimal, value)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  char* string;
  int string_len;

  php_driver_format_integer(self->data.decimal.value, &string, &string_len);

  PHP5TO7_RETVAL_STRINGL(string, string_len);
  efree(string);
}
/* }}} */

ZEND_METHOD(Cassandra_Decimal, scale)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  RETURN_LONG(self->data.decimal.scale);
}

/* {{{ Decimal::add() */
ZEND_METHOD(Cassandra_Decimal, add)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_decimal_ce TSRMLS_CC)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }

  php_driver_numeric* self    = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* decimal = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  object_init_ex(return_value, php_driver_decimal_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);
  align_decimals(self, decimal);
  mpz_add(result->data.decimal.value, self->data.decimal.value, decimal->data.decimal.value);
  result->data.decimal.scale = MAX(self->data.decimal.scale, decimal->data.decimal.scale);
}
/* }}} */

/* {{{ Decimal::sub() */
ZEND_METHOD(Cassandra_Decimal, sub)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_decimal_ce)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }

  php_driver_numeric* self    = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* decimal = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  object_init_ex(return_value, php_driver_decimal_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  align_decimals(self, decimal);
  mpz_sub(result->data.decimal.value, self->data.decimal.value, decimal->data.decimal.value);
  result->data.decimal.scale = MAX(self->data.decimal.scale, decimal->data.decimal.scale);
}
/* }}} */

/* {{{ Decimal::mul() */
ZEND_METHOD(Cassandra_Decimal, mul)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), php_driver_decimal_ce TSRMLS_CC)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }

  php_driver_numeric* self    = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* decimal = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  object_init_ex(return_value, php_driver_decimal_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  mpz_mul(result->data.decimal.value, self->data.decimal.value, decimal->data.decimal.value);
  result->data.decimal.scale = self->data.decimal.scale + decimal->data.decimal.scale;
}
/* }}} */

/* {{{ Decimal::div() */
ZEND_METHOD(Cassandra_Decimal, div)
{
  /* TODO: Implementation of this a bit more difficult than anticipated. */
  zend_throw_exception_ex(php_driver_runtime_exception_ce, 0 TSRMLS_CC, "Not implemented");
}
/* }}} */

/* {{{ Decimal::mod() */
ZEND_METHOD(Cassandra_Decimal, mod)
{
  /* TODO: We could implement a remainder method */
  zend_throw_exception_ex(php_driver_runtime_exception_ce, 0 TSRMLS_CC, "Not implemented");
}

/* {{{ Decimal::abs() */
ZEND_METHOD(Cassandra_Decimal, abs)
{
  php_driver_numeric* result = NULL;
  php_driver_numeric* self   = PHP_DRIVER_GET_NUMERIC(getThis());

  object_init_ex(return_value, php_driver_decimal_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);

  mpz_abs(result->data.decimal.value, self->data.decimal.value);
  result->data.decimal.scale = self->data.decimal.scale;
}
/* }}} */

/* {{{ Decimal::neg() */
ZEND_METHOD(Cassandra_Decimal, neg)
{
  php_driver_numeric* result = NULL;
  php_driver_numeric* self   = PHP_DRIVER_GET_NUMERIC_THIS();

  object_init_ex(return_value, php_driver_decimal_ce);
  result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  mpz_neg(result->data.decimal.value, self->data.decimal.value);
  result->data.decimal.scale = self->data.decimal.scale;
}
/* }}} */

/* {{{ Decimal::sqrt() */
ZEND_METHOD(Cassandra_Decimal, sqrt)
{
  zend_throw_exception_ex(php_driver_runtime_exception_ce, 0 TSRMLS_CC, "Not implemented");
#if 0
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  mpf_t value;
  mpf_init(value);
  to_mpf(value, self);

  mpf_sqrt(value, value);

  mp_exp_t exponent;
  char* mantissa = mpf_get_str(NULL, &exponent, 10, 0, value);

  object_init_ex(return_value, php_driver_decimal_ce);
  php_driver_numeric *result = PHP_DRIVER_GET_NUMERIC(return_value);

  mpz_set_str(result->value.decimal_value, mantissa, 10);
  mp_bitcnt_t prec = mpf_get_prec(value);
  exponent -= prec;
  result->value.decimal_scale = -exponent;

  free(mantissa);
  mpf_clear(value);
#endif
}
/* }}} */

/* {{{ Decimal::toInt() */
ZEND_METHOD(Cassandra_Decimal, toInt)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  to_long(return_value, self);
}
/* }}} */

/* {{{ Decimal::toDouble() */
ZEND_METHOD(Cassandra_Decimal, toDouble)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  to_double(return_value, self);
}
/* }}} */
#include "Decimal_arginfo.h"

static php_driver_value_handlers php_driver_decimal_handlers;

static HashTable*
php_driver_decimal_gc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
php_driver_decimal_properties(
  zend_object* object)
{
  char* string;
  int string_len;
  php5to7_zval type;
  php5to7_zval value;
  php5to7_zval scale;

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC(object);
  HashTable* props         = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_DECIMAL);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  php_driver_format_integer(self->data.decimal.value, &string, &string_len);
  PHP5TO7_ZVAL_MAYBE_MAKE(PHP5TO7_ZVAL_MAYBE_P(value));
  PHP5TO7_ZVAL_STRINGL(PHP5TO7_ZVAL_MAYBE_P(value), string, string_len);
  efree(string);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), PHP5TO7_ZVAL_MAYBE_P(value), sizeof(zval));

  PHP5TO7_ZVAL_MAYBE_MAKE(scale);
  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(scale), self->data.decimal.scale);
  PHP5TO7_ZEND_HASH_UPDATE(props, "scale", sizeof("scale"), PHP5TO7_ZVAL_MAYBE_P(scale), sizeof(zval));

  return props;
}

static int
php_driver_decimal_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_driver_numeric* decimal1 = NULL;
  php_driver_numeric* decimal2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  decimal1 = PHP_DRIVER_GET_NUMERIC_ZVAL(obj1);
  decimal2 = PHP_DRIVER_GET_NUMERIC_ZVAL(obj2);

  if (decimal1->data.decimal.scale == decimal2->data.decimal.scale) {
    return mpz_cmp(decimal1->data.decimal.value, decimal2->data.decimal.value);
  }

  if (decimal1->data.decimal.scale < decimal2->data.decimal.scale) {
    return -1;
  }

  return 1;
}

static uint32_t
php_driver_decimal_hash_value(zval* obj)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_ZVAL(obj);

  return php_driver_mpz_hash((uint32_t) self->data.decimal.scale, self->data.decimal.value);
}

static ZEND_RESULT_CODE
php_driver_decimal_cast(
  zend_object* object,
  zval* retval,
  int type)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC(object);

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

static void
php_driver_decimal_free(zend_object* object)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC(object);

  mpz_clear(self->data.decimal.value);

  zend_object_std_dtor(&self->zval);
}

static php5to7_zend_object
php_driver_decimal_new(zend_class_entry* ce)
{
  php_driver_numeric* self = emalloc(sizeof(php_driver_numeric));

  self->type               = PHP_DRIVER_DECIMAL;
  self->data.decimal.scale = 0;
  mpz_init(self->data.decimal.value);

  zend_object_std_init(&self->zval, ce);

  self->zval.handlers = &php_driver_decimal_handlers.std;
  return &self->zval;
}

void
php_driver_define_Decimal(zend_class_entry* value_interface, zend_class_entry* numeric_interface)
{
  php_driver_decimal_ce                = register_class_Cassandra_Decimal(value_interface, numeric_interface);
  php_driver_decimal_ce->create_object = php_driver_decimal_new;

  memcpy(&php_driver_decimal_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_decimal_handlers.std.get_properties = php_driver_decimal_properties;
  php_driver_decimal_handlers.std.get_gc         = php_driver_decimal_gc;
  php_driver_decimal_handlers.std.compare        = php_driver_decimal_compare;
  php_driver_decimal_handlers.std.cast_object    = php_driver_decimal_cast;
  php_driver_decimal_handlers.std.offset         = XtOffsetOf(php_driver_numeric, zval);
  php_driver_decimal_handlers.std.free_obj       = php_driver_decimal_free;
  php_driver_decimal_handlers.hash_value         = php_driver_decimal_hash_value;
  php_driver_decimal_handlers.std.clone_obj      = NULL;
}
