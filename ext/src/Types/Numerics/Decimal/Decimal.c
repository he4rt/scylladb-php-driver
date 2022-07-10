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

#include <float.h>
#include <gmp.h>
#include <math.h>
#include <php.h>
#include "php_driver.h"

#include <ext/spl/spl_exceptions.h>

#include <CassandraDriverAPI.h>
#include <Types/Numerics/Numerics.h>

#include "Decimal.h"
#include "Decimal_arginfo.h"

#include "util/hash.h"
#include "util/math.h"
#include "util/types.h"

PHP_DRIVER_API zend_class_entry* phpDriverDecimalCe = NULL;

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
ToMpf(mpf_t result, PhpDriverNumeric* decimal)
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
FromDouble(PhpDriverNumeric* result, double value)
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
ToDouble(zval* result, PhpDriverNumeric* decimal)
{
  mpf_t value;
  mpf_init(value);
  ToMpf(value, decimal);

  if (mpf_cmp_d(value, -DBL_MAX) < 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value is too small");
    mpf_clear(value);
    return FAILURE;
  }

  if (mpf_cmp_d(value, DBL_MAX) > 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value is too big");
    mpf_clear(value);
    return FAILURE;
  }

  ZVAL_DOUBLE(result, mpf_get_d(value));
  mpf_clear(value);
  return SUCCESS;
}

static zend_always_inline ZEND_RESULT_CODE
ToLong(zval* result, PhpDriverNumeric* decimal)
{
  mpf_t value;
  mpf_init(value);
  ToMpf(value, decimal);

  if (mpf_cmp_si(value, LONG_MIN) < 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value is too small");
    mpf_clear(value);
    return FAILURE;
  }

  if (mpf_cmp_si(value, LONG_MAX) > 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value is too big");
    mpf_clear(value);
    return FAILURE;
  }

  ZVAL_LONG(result, mpf_get_si(value));

  mpf_clear(value);
  return SUCCESS;
}

static zend_always_inline ZEND_RESULT_CODE
ToString(zval* result, PhpDriverNumeric* decimal)
{
  char* string;
  int string_len;
  php_driver_format_decimal(decimal->data.decimal.value, decimal->data.decimal.scale, &string, &string_len);

  ZVAL_STRINGL(result, string, string_len);
  efree(string);

  return SUCCESS;
}

static zend_always_inline void
AlignDecimals(PhpDriverNumeric* lhs, PhpDriverNumeric* rhs)
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
PhpDriverDecimalInit(PhpDriverNumeric* self, zval* value)
{
  switch (Z_TYPE_P(value)) {
  case IS_LONG: {
    mpz_set_si(self->data.decimal.value, Z_LVAL_P(value));
    self->data.decimal.scale = 0;
  }
  case IS_DOUBLE: {
    double val = Z_DVAL_P(value);
    if (zend_isnan(val) || zend_isinf(val)) {
      zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0,
                              "Value of NaN or +/- infinity is not supported");
      return;
    }

    FromDouble(self, val);
  }
  case IS_STRING: {
    php_driver_parse_decimal(
      Z_STRVAL_P(value),
      Z_STRLEN_P(value),
      &self->data.decimal.value,
      &self->data.decimal.scale);
  }
  case IS_OBJECT: {
    if (instanceof_function(Z_OBJCE_P(value), phpDriverDecimalCe)) {
      PhpDriverNumeric* decimal = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(value);
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

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  PhpDriverDecimalInit(self, value);
}
/* }}} */

/* {{{ Decimal::__toString() */
ZEND_METHOD(Cassandra_Decimal, __toString)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToString(return_value, self);
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
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  char* string;
  int string_len;

  php_driver_format_integer(self->data.decimal.value, &string, &string_len);

  RETVAL_STRINGL(string, string_len);
  efree(string);
}
/* }}} */

ZEND_METHOD(Cassandra_Decimal, scale)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  RETURN_LONG(self->data.decimal.scale);
}

/* {{{ Decimal::add() */
ZEND_METHOD(Cassandra_Decimal, add)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverDecimalCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }

  PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* decimal = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverDecimalCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  AlignDecimals(self, decimal);
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

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverDecimalCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }

  PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* decimal = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverDecimalCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  AlignDecimals(self, decimal);
  mpz_sub(result->data.decimal.value, self->data.decimal.value, decimal->data.decimal.value);
  result->data.decimal.scale = MAX(self->data.decimal.scale, decimal->data.decimal.scale);
}
/* }}} */

/* {{{ Decimal::mul() */
ZEND_METHOD(Cassandra_Decimal, mul)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(num), phpDriverDecimalCe)) {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Decimal");
  }

  PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* decimal = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverDecimalCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  mpz_mul(result->data.decimal.value, self->data.decimal.value, decimal->data.decimal.value);
  result->data.decimal.scale = self->data.decimal.scale + decimal->data.decimal.scale;
}
/* }}} */

/* {{{ Decimal::div() */
ZEND_METHOD(Cassandra_Decimal, div)
{
  /* TODO: Implementation of this a bit more difficult than anticipated. */
  zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Not implemented");
}
/* }}} */

/* {{{ Decimal::mod() */
ZEND_METHOD(Cassandra_Decimal, mod)
{
  /* TODO: We could implement a remainder method */
  zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Not implemented");
}

/* {{{ Decimal::abs() */
ZEND_METHOD(Cassandra_Decimal, abs)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  object_init_ex(return_value, phpDriverDecimalCe);
  result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

  mpz_abs(result->data.decimal.value, self->data.decimal.value);
  result->data.decimal.scale = self->data.decimal.scale;
}
/* }}} */

/* {{{ Decimal::neg() */
ZEND_METHOD(Cassandra_Decimal, neg)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, phpDriverDecimalCe);
  result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  mpz_neg(result->data.decimal.value, self->data.decimal.value);
  result->data.decimal.scale = self->data.decimal.scale;
}
/* }}} */

/* {{{ Decimal::sqrt() */
ZEND_METHOD(Cassandra_Decimal, sqrt)
{
  zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Not implemented");
#if 0
  PhpDriverNumeric *self = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  mpf_t value;
  mpf_init(value);
  to_mpf(value, self);

  mpf_sqrt(value, value);

  mp_exp_t exponent;
  char* mantissa = mpf_get_str(NULL, &exponent, 10, 0, value);

  object_init_ex(return_value, phpDriverDecimalCe);
  PhpDriverNumeric *result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

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
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToLong(return_value, self);
}
/* }}} */

/* {{{ Decimal::toDouble() */
ZEND_METHOD(Cassandra_Decimal, toDouble)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToDouble(return_value, self);
}
/* }}} */

static php_driver_value_handlers phpDriverDecimalHandlers;

static HashTable*
PhpDriverDecimalGc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
PhpDriverDecimalProperties(
  zend_object* object)
{
  char* string;
  int string_len;
  zval type;
  zval value;
  zval scale;

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props       = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_DECIMAL);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  php_driver_format_integer(self->data.decimal.value, &string, &string_len);

  ZVAL_STRINGL(&value, string, string_len);
  efree(string);
  zend_hash_str_update(props, "value", sizeof("value") - 1, &value);

  ZVAL_LONG(&scale, self->data.decimal.scale);
  zend_hash_str_update(props, "scale", sizeof("scale") - 1, &scale);

  return props;
}

static int
PhpDriverDecimalCompare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  PhpDriverNumeric* decimal1 = NULL;
  PhpDriverNumeric* decimal2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  decimal1 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj1);
  decimal2 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj2);

  if (decimal1->data.decimal.scale == decimal2->data.decimal.scale) {
    return mpz_cmp(decimal1->data.decimal.value, decimal2->data.decimal.value);
  }

  if (decimal1->data.decimal.scale < decimal2->data.decimal.scale) {
    return -1;
  }

  return 1;
}

static uint32_t
PhpDriverDecimalHashValue(zval* obj)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj);

  return php_driver_mpz_hash((uint32_t) self->data.decimal.scale, self->data.decimal.value);
}

static int
PhpDriverDecimalCast(
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

static void
PhpDriverDecimalFree(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  mpz_clear(self->data.decimal.value);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
PhpDriverDecimalNew(zend_class_entry* ce)
{
  PhpDriverNumeric* self = make(PhpDriverNumeric);

  self->type               = PHP_DRIVER_DECIMAL;
  self->data.decimal.scale = 0;
  mpz_init(self->data.decimal.value);

  zend_object_std_init(&self->zval, ce);

  self->zval.handlers = &phpDriverDecimalHandlers.std;
  return &self->zval;
}

void
PhpDriverDefineDecimal(zend_class_entry* valueInterface, zend_class_entry* numericInterface)
{
  phpDriverDecimalCe                = register_class_Cassandra_Decimal(valueInterface, numericInterface);
  phpDriverDecimalCe->create_object = PhpDriverDecimalNew;

  memcpy(&phpDriverDecimalHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  phpDriverDecimalHandlers.std.get_properties = PhpDriverDecimalProperties;
  phpDriverDecimalHandlers.std.get_gc         = PhpDriverDecimalGc;
  phpDriverDecimalHandlers.std.compare        = PhpDriverDecimalCompare;
  phpDriverDecimalHandlers.std.cast_object    = PhpDriverDecimalCast;
  phpDriverDecimalHandlers.std.free_obj       = PhpDriverDecimalFree;
  phpDriverDecimalHandlers.hash_value         = PhpDriverDecimalHashValue;
  phpDriverDecimalHandlers.std.offset         = XtOffsetOf(PhpDriverNumeric, zval);
  phpDriverDecimalHandlers.std.clone_obj      = NULL;
}
