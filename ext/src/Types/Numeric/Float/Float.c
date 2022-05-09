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

#include "util/math.h"
#include "util/types.h"
#include <float.h>

#include <Types/Numeric/Numeric.h>

#include "Float_arginfo.h"

zend_class_entry* php_driver_float_ce = NULL;

static zend_always_inline ZEND_RESULT_CODE
to_string(zval* result, php_driver_numeric* flt)
{
  smart_str string = { 0 };
  smart_str_append_double(&string, (double) flt->data.floating.value, EG(precision), false);
  smart_str_0(&string);

  ZVAL_STR(result, string.s);
  return SUCCESS;
}

void zend_always_inline
php_driver_float_init(php_driver_numeric* self, zval* value)
{
  switch (Z_TYPE_P(value)) {
  case IS_LONG: {
    self->data.floating.value = (cass_float_t) Z_LVAL_P(value);
    return;
  }
  case IS_DOUBLE: {
    self->data.floating.value = (cass_float_t) Z_DVAL_P(value);
    return;
  }
  case IS_STRING: {
    php_driver_parse_float(
      Z_STRVAL_P(value),
      Z_STRLEN_P(value),
      &self->data.floating.value);
    return;
  }
  case IS_OBJECT: {
    if (instanceof_function(Z_OBJCE_P(value), php_driver_float_ce)) {
      php_driver_numeric* other = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(value);
      self->data.floating.value = other->data.floating.value;
      return;
    }
  }
  }

  INVALID_ARGUMENT(value, "a long, double, numeric string or a " PHP_DRIVER_NAMESPACE "\\Float instance");
}

/* {{{ Float::__construct(string) */
ZEND_METHOD(Cassandra_Float, __construct)
{
  zval* value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  php_driver_float_init(self, value);
}
/* }}} */

/* {{{ Float::__toString() */
ZEND_METHOD(Cassandra_Float, __toString)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  to_string(return_value, self);
}
/* }}} */

/* {{{ Float::type() */
ZEND_METHOD(Cassandra_Float, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Float::value() */
ZEND_METHOD(Cassandra_Float, value)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::isInfinite() */
ZEND_METHOD(Cassandra_Float, isInfinite)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_BOOL(zend_isinf(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isFinite() */
ZEND_METHOD(Cassandra_Float, isFinite)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_BOOL(zend_finite(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isNaN() */
ZEND_METHOD(Cassandra_Float, isNaN)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_BOOL(zend_isnan(self->data.floating.value));
}
/* }}} */

/* {{{ Float::add() */
ZEND_METHOD(Cassandra_Float, add)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = self->data.floating.value + flt->data.floating.value;
}
/* }}} */

/* {{{ Float::sub() */
ZEND_METHOD(Cassandra_Float, sub)
{
  zval* num;
  php_driver_numeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_float_ce);
  result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = self->data.floating.value - flt->data.floating.value;
}
/* }}} */

/* {{{ Float::mul() */
ZEND_METHOD(Cassandra_Float, mul)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = self->data.floating.value * flt->data.floating.value;
}
/* }}} */

/* {{{ Float::div() */
ZEND_METHOD(Cassandra_Float, div)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  if (flt->data.floating.value == 0) {
    zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
    return;
  }

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = self->data.floating.value / flt->data.floating.value;
}
/* }}} */

/* {{{ Float::mod() */
ZEND_METHOD(Cassandra_Float, mod)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  if (flt->data.floating.value == 0) {
    zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
    return;
  }

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.floating.value = (cass_float_t) fmod(self->data.floating.value, flt->data.floating.value);
}

/* {{{ Float::abs() */
ZEND_METHOD(Cassandra_Float, abs)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = fabsf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::neg() */
ZEND_METHOD(Cassandra_Float, neg)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = -self->data.floating.value;
}
/* }}} */

/* {{{ Float::sqrt() */
ZEND_METHOD(Cassandra_Float, sqrt)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.floating.value < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0,
                            "Cannot take a square root of a negative number");
    return;
  }

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = sqrtf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::toInt() */
ZEND_METHOD(Cassandra_Float, toInt)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  RETURN_LONG((zend_long) self->data.floating.value);
}
/* }}} */

/* {{{ Float::toDouble() */
ZEND_METHOD(Cassandra_Float, toDouble)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_THIS();

  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::min() */
ZEND_METHOD(Cassandra_Float, min)
{
  object_init_ex(return_value, php_driver_float_ce);

  php_driver_numeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  flt->data.floating.value = FLT_MIN;
}
/* }}} */

/* {{{ Float::max() */
ZEND_METHOD(Cassandra_Float, max)
{
  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* flt = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  flt->data.floating.value = FLT_MAX;
}
/* }}} */

static php_driver_value_handlers php_driver_float_handlers;

static HashTable*
php_driver_float_gc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
php_driver_float_properties(zend_object* object)
{
  zval type;
  zval value;

  php_driver_numeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props         = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  to_string(&value, self);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), &value, sizeof(zval));

  return props;
}

static zend_always_inline cass_int32_t
float_to_bits(cass_float_t value)
{
  cass_int32_t bits;
  if (zend_isnan(value)) {
    return 0x7fc00000; /* A canonical NaN value */
  }

  memcpy(&bits, &value, sizeof(cass_int32_t));
  return bits;
}

static int
php_driver_float_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  php_driver_numeric* flt1 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj1);
  php_driver_numeric* flt2 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj2);

  if (flt1->data.floating.value < flt2->data.floating.value) {
    return -1;
  }
  if (flt1->data.floating.value > flt2->data.floating.value) {
    return 1;
  }

  cass_int32_t bits1 = float_to_bits(flt1->data.floating.value);
  cass_int32_t bits2 = float_to_bits(flt2->data.floating.value);

  /* Handle NaNs and negative and positive 0.0 */
  return bits1 < bits2 ? -1 : bits1 > bits2;
}

static unsigned
php_driver_float_hash_value(zval* obj)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj);
  return float_to_bits(self->data.floating.value);
}

static int
php_driver_float_cast(
  zend_object* object,
  zval* retval,
  int type)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  switch (type) {
  case IS_LONG: {
    ZVAL_LONG(retval, (int64_t) self->data.floating.value);
    return SUCCESS;
  }
  case IS_DOUBLE: {
    ZVAL_DOUBLE(retval, (double) self->data.floating.value);
    return SUCCESS;
  }
  case IS_STRING: {
    return to_string(retval, self);
  }
  default: {
    return FAILURE;
  }
  }
}

void
php_driver_float_free(zend_object* object)
{
  php_driver_numeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  zend_object_std_dtor(&self->zval);
}

zend_object*
php_driver_float_new(zend_class_entry* ce)
{
  php_driver_numeric* self = make(php_driver_numeric);

  self->type = PHP_DRIVER_FLOAT;

  zend_object_std_init(&self->zval, ce);
  self->zval.handlers = &php_driver_float_handlers.std;

  return &self->zval;
}

void
php_driver_define_Float(zend_class_entry* value_interface, zend_class_entry* numeric_interface)
{
  php_driver_float_ce                = register_class_Cassandra_Float(value_interface, numeric_interface);
  php_driver_float_ce->create_object = php_driver_float_new;

  memcpy(&php_driver_float_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_float_handlers.std.get_properties = php_driver_float_properties;
  php_driver_float_handlers.std.get_gc         = php_driver_float_gc;
  php_driver_float_handlers.std.compare        = php_driver_float_compare;
  php_driver_float_handlers.std.cast_object    = php_driver_float_cast;
  php_driver_float_handlers.std.free_obj       = php_driver_float_free;
  php_driver_float_handlers.hash_value         = php_driver_float_hash_value;
  php_driver_float_handlers.std.offset         = XtOffsetOf(php_driver_numeric, zval);
  php_driver_float_handlers.std.clone_obj      = NULL;
}
