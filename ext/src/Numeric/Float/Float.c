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
#include "util/math.h"
#include "util/types.h"
#include <float.h>

#include <Numeric/Numeric.h>

zend_class_entry* php_driver_float_ce = NULL;

static ZEND_RESULT_CODE
to_string(zval* result, php_driver_numeric* flt)
{
  smart_str string = { 0 };
  smart_str_append_double(&string, (double) flt->data.floating.value, EG(precision), false);
  smart_str_0(&string);

  ZVAL_STR(result, string.s);
  return SUCCESS;
}

void
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
      php_driver_numeric* other = PHP_DRIVER_GET_NUMERIC_ZVAL(value);
      self->data.floating.value = other->data.floating.value;
      return;
    }
  }
  }

  INVALID_ARGUMENT(value, "a long, double, numeric string or a " PHP_DRIVER_NAMESPACE "\\Float instance");
}

/* {{{ Float::__construct(string) */
PHP_METHOD(Float, __construct)
{
  zval* value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  php_driver_float_init(self, value);
}
/* }}} */

/* {{{ Float::__toString() */
PHP_METHOD(Float, __toString)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  to_string(return_value, self);
}
/* }}} */

/* {{{ Float::type() */
PHP_METHOD(Float, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Float::value() */
PHP_METHOD(Float, value)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::isInfinite() */
PHP_METHOD(Float, isInfinite)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  RETURN_BOOL(zend_isinf(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isFinite() */
PHP_METHOD(Float, isFinite)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  RETURN_BOOL(zend_finite(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isNaN() */
PHP_METHOD(Float, isNaN)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  RETURN_BOOL(zend_isnan(self->data.floating.value));
}
/* }}} */

/* {{{ Float::add() */
PHP_METHOD(Float, add)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = self->data.floating.value + flt->data.floating.value;
}
/* }}} */

/* {{{ Float::sub() */
PHP_METHOD(Float, sub)
{
  zval* num;
  php_driver_numeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  object_init_ex(return_value, php_driver_float_ce);
  result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = self->data.floating.value - flt->data.floating.value;
}
/* }}} */

/* {{{ Float::mul() */
PHP_METHOD(Float, mul)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = self->data.floating.value * flt->data.floating.value;
}
/* }}} */

/* {{{ Float::div() */
PHP_METHOD(Float, div)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  if (flt->data.floating.value == 0) {
    zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
    return;
  }

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = self->data.floating.value / flt->data.floating.value;
}
/* }}} */

/* {{{ Float::mod() */
PHP_METHOD(Float, mod)
{
  zval* num;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), php_driver_float_ce))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();
  php_driver_numeric* flt  = PHP_DRIVER_GET_NUMERIC_ZVAL(num);

  if (flt->data.floating.value == 0) {
    zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
    return;
  }

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result  = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);
  result->data.floating.value = (cass_float_t) fmod(self->data.floating.value, flt->data.floating.value);
}

/* {{{ Float::abs() */
PHP_METHOD(Float, abs)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = fabsf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::neg() */
PHP_METHOD(Float, neg)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = -self->data.floating.value;
}
/* }}} */

/* {{{ Float::sqrt() */
PHP_METHOD(Float, sqrt)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  if (self->data.floating.value < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0,
                            "Cannot take a square root of a negative number");
    return;
  }

  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* result = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  result->data.floating.value = sqrtf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::toInt() */
PHP_METHOD(Float, toInt)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  RETURN_LONG((zend_long) self->data.floating.value);
}
/* }}} */

/* {{{ Float::toDouble() */
PHP_METHOD(Float, toDouble)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_THIS();

  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::min() */
PHP_METHOD(Float, min)
{
  object_init_ex(return_value, php_driver_float_ce);

  php_driver_numeric* flt  = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);
  flt->data.floating.value = FLT_MIN;
}
/* }}} */

/* {{{ Float::max() */
PHP_METHOD(Float, max)
{
  object_init_ex(return_value, php_driver_float_ce);
  php_driver_numeric* flt = PHP_DRIVER_GET_NUMERIC_ZVAL(return_value);

  flt->data.floating.value = FLT_MAX;
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_num, 0, ZEND_RETURN_VALUE, 1)
ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_float_methods[] = {
  PHP_ME(Float, __construct, arginfo__construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(Float, __toString, arginfo_none, ZEND_ACC_PUBLIC)
      PHP_ME(Float, type, arginfo_none, ZEND_ACC_PUBLIC)
        PHP_ME(Float, value, arginfo_none, ZEND_ACC_PUBLIC)
          PHP_ME(Float, isInfinite, arginfo_none, ZEND_ACC_PUBLIC)
            PHP_ME(Float, isFinite, arginfo_none, ZEND_ACC_PUBLIC)
              PHP_ME(Float, isNaN, arginfo_none, ZEND_ACC_PUBLIC)
                PHP_ME(Float, add, arginfo_num, ZEND_ACC_PUBLIC)
                  PHP_ME(Float, sub, arginfo_num, ZEND_ACC_PUBLIC)
                    PHP_ME(Float, mul, arginfo_num, ZEND_ACC_PUBLIC)
                      PHP_ME(Float, div, arginfo_num, ZEND_ACC_PUBLIC)
                        PHP_ME(Float, mod, arginfo_num, ZEND_ACC_PUBLIC)
                          PHP_ME(Float, abs, arginfo_none, ZEND_ACC_PUBLIC)
                            PHP_ME(Float, neg, arginfo_none, ZEND_ACC_PUBLIC)
                              PHP_ME(Float, sqrt, arginfo_none, ZEND_ACC_PUBLIC)
                                PHP_ME(Float, toInt, arginfo_none, ZEND_ACC_PUBLIC)
                                  PHP_ME(Float, toDouble, arginfo_none, ZEND_ACC_PUBLIC)
                                    PHP_ME(Float, min, arginfo_none, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                                      PHP_ME(Float, max, arginfo_none, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                                        PHP_FE_END
};

static php_driver_value_handlers php_driver_float_handlers;

static HashTable*
php_driver_float_gc(
  zend_object* object,
  php5to7_zval_gc table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
php_driver_float_properties(zend_object* object)
{
  php5to7_zval type;
  php5to7_zval value;

  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC(object);
  HashTable* props         = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  to_string(&value, self);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), &value, sizeof(zval));

  return props;
}

static inline cass_int32_t
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

  php_driver_numeric* flt1 = PHP_DRIVER_GET_NUMERIC_ZVAL(obj1);
  php_driver_numeric* flt2 = PHP_DRIVER_GET_NUMERIC_ZVAL(obj2);

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
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC_ZVAL(obj);
  return float_to_bits(self->data.floating.value);
}

static ZEND_RESULT_CODE
php_driver_float_cast(
  zend_object* object,
  zval* retval,
  int type)
{
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC(object);

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
  php_driver_numeric* self = PHP_DRIVER_GET_NUMERIC(object);

  zend_object_std_dtor(&self->zval);
}

zend_object*
php_driver_float_new(zend_class_entry* ce)
{
  php_driver_numeric* self = emalloc(sizeof(php_driver_numeric));

  self->type = PHP_DRIVER_FLOAT;

  zend_object_std_init(&self->zval, ce);
  self->zval.handlers = &php_driver_float_handlers.std;

  return &self->zval;
}

void
php_driver_define_Float()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Float", php_driver_float_methods);

  php_driver_float_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_float_ce, 2, php_driver_value_ce, php_driver_numeric_ce);

  php_driver_float_ce->ce_flags |= PHP5TO7_ZEND_ACC_FINAL;
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
