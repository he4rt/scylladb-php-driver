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
BEGIN_EXTERN_C()
#include "Float_arginfo.h"
zend_class_entry *php_driver_float_ce = NULL;

static zend_result
to_string(zval *result, php_driver_numeric *flt )
{
  char *string;
  spprintf(&string, 0, "%.*F", (int) EG(precision), flt->data.floating.value);
  ZVAL_STRING(result, string);
  efree(string);
  return SUCCESS;
}

void
php_driver_float_init(INTERNAL_FUNCTION_PARAMETERS)
{
  php_driver_numeric *self;
  zval *value;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(value)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_float_ce)) {
    self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  } else {
    object_init_ex(return_value, php_driver_float_ce);
    self = PHP_DRIVER_GET_NUMERIC(return_value);
  }

  if (Z_TYPE_P(value) == IS_LONG) {
    self->data.floating.value = (cass_float_t) Z_LVAL_P(value);
  } else if (Z_TYPE_P(value) == IS_DOUBLE) {
    self->data.floating.value = (cass_float_t) Z_DVAL_P(value);
  } else if (Z_TYPE_P(value) == IS_STRING) {
    if (!php_driver_parse_float(Z_STRVAL_P(value), Z_STRLEN_P(value),
                                   &self->data.floating.value )) {
      return;
    }
  } else if (Z_TYPE_P(value) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(value), php_driver_float_ce )) {
    php_driver_numeric *flt = PHP_DRIVER_GET_NUMERIC(return_value);
    self->data.floating.value = flt->data.floating.value;
  } else {
    INVALID_ARGUMENT(value, "a long, double, numeric string or a " \
                            PHP_DRIVER_NAMESPACE "\\Float instance");
  }
}

/* {{{ Float::__construct(string) */
ZEND_METHOD(Cassandra_Float, __construct)
{
  php_driver_float_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Float::__toString() */
ZEND_METHOD(Cassandra_Float, __toString)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

  to_string(return_value, self );
}
/* }}} */

/* {{{ Float::type() */
ZEND_METHOD(Cassandra_Float, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT );
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Float::value() */
ZEND_METHOD(Cassandra_Float, value)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::isInfinite() */
ZEND_METHOD(Cassandra_Float, isInfinite)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  RETURN_BOOL(zend_isinf(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isFinite() */
ZEND_METHOD(Cassandra_Float, isFinite)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  RETURN_BOOL(zend_finite(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isNaN() */
ZEND_METHOD(Cassandra_Float, isNaN)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  RETURN_BOOL(zend_isnan(self->data.floating.value));
}
/* }}} */

/* {{{ Float::add() */
ZEND_METHOD(Cassandra_Float, add)
{
  zval *num;
  php_driver_numeric *result = NULL;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(num)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_float_ce )) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    php_driver_numeric *flt = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_float_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    result->data.floating.value = self->data.floating.value + flt->data.floating.value;
  } else {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }
}
/* }}} */

/* {{{ Float::sub() */
ZEND_METHOD(Cassandra_Float, sub)
{
  zval *num;
  php_driver_numeric *result = NULL;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(num)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_float_ce )) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    php_driver_numeric *flt = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_float_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    result->data.floating.value = self->data.floating.value - flt->data.floating.value;
  } else {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }
}
/* }}} */

/* {{{ Float::mul() */
ZEND_METHOD(Cassandra_Float, mul)
{
  zval *num;
  php_driver_numeric *result = NULL;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(num)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_float_ce )) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    php_driver_numeric *flt = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_float_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    result->data.floating.value = self->data.floating.value * flt->data.floating.value;
  } else {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }
}
/* }}} */

/* {{{ Float::div() */
ZEND_METHOD(Cassandra_Float, div)
{
  zval *num;
  php_driver_numeric *result = NULL;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(num)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_float_ce )) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    php_driver_numeric *flt = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_float_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    if (flt->data.floating.value == 0) {
      zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
      return;
    }

    result->data.floating.value = self->data.floating.value / flt->data.floating.value;
  } else {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }
}
/* }}} */

/* {{{ Float::mod() */
ZEND_METHOD(Cassandra_Float, mod)
{
  zval *num;
  php_driver_numeric *result = NULL;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(num)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_float_ce )) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    php_driver_numeric *flt = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_float_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    if (flt->data.floating.value == 0) {
      zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
      return;
    }

    result->data.floating.value = fmod(self->data.floating.value, flt->data.floating.value);
  } else {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }
}

/* {{{ Float::abs() */
ZEND_METHOD(Cassandra_Float, abs)
{
  php_driver_numeric *result = NULL;
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  object_init_ex(return_value, php_driver_float_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);
  result->data.floating.value = fabsf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::neg() */
ZEND_METHOD(Cassandra_Float, neg)
{
  php_driver_numeric *result = NULL;
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
  object_init_ex(return_value, php_driver_float_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);
  result->data.floating.value = -self->data.floating.value;
}
/* }}} */

/* {{{ Float::sqrt() */
ZEND_METHOD(Cassandra_Float, sqrt)
{
  php_driver_numeric *result = NULL;
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

  if (self->data.floating.value < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                            "Cannot take a square root of a negative number");
  }

  object_init_ex(return_value, php_driver_float_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);
  result->data.floating.value = sqrtf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::toInt() */
ZEND_METHOD(Cassandra_Float, toInt)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

  RETURN_LONG((long) self->data.floating.value);
}
/* }}} */

/* {{{ Float::toDouble() */
ZEND_METHOD(Cassandra_Float, toDouble)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::min() */
ZEND_METHOD(Cassandra_Float, min)
{
  php_driver_numeric *flt = NULL;
  object_init_ex(return_value, php_driver_float_ce);
  flt = PHP_DRIVER_GET_NUMERIC(return_value);
  flt->data.floating.value = FLT_MIN;
}
/* }}} */

/* {{{ Float::max() */
ZEND_METHOD(Cassandra_Float, max)
{
  php_driver_numeric *flt = NULL;
  object_init_ex(return_value, php_driver_float_ce);
  flt = PHP_DRIVER_GET_NUMERIC(return_value);
  flt->data.floating.value = FLT_MAX;
}
/* }}} */


static php_driver_value_handlers php_driver_float_handlers;

static HashTable *
php_driver_float_gc(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object,
#else
        zendObject *object,
#endif
        zval** table, int *n
)
{
  *table = NULL;
  *n = 0;
  return NULL;
}

static HashTable *
php_driver_float_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
  zval type;
  zval value;

#if PHP_MAJOR_VERSION >= 8
  php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);
#else
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(object);
#endif
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT );
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);


  to_string(&value, self );
  (void)zend_hash_str_update(props, ZEND_STRL("value"), &value);

  return props;
}

static inline cass_int32_t
float_to_bits(cass_float_t value) {
  cass_int32_t bits;
  if (zend_isnan(value)) return 0x7fc00000; /* A canonical NaN value */
  memcpy(&bits, &value, sizeof(cass_int32_t));
  return bits;
}

static int
php_driver_float_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  cass_int32_t bits1, bits2;
  php_driver_numeric *flt1 = NULL;
  php_driver_numeric *flt2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  flt1 = PHP_DRIVER_GET_NUMERIC(obj1);
  flt2 = PHP_DRIVER_GET_NUMERIC(obj2);

  if (flt1->data.floating.value < flt2->data.floating.value) return -1;
  if (flt1->data.floating.value > flt2->data.floating.value) return  1;

  bits1 = float_to_bits(flt1->data.floating.value);
  bits2 = float_to_bits(flt2->data.floating.value);

  /* Handle NaNs and negative and positive 0.0 */
  return bits1 < bits2 ? -1 : bits1 > bits2;
}

static unsigned
php_driver_float_hash_value(zval *obj )
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(obj);
  return float_to_bits(self->data.floating.value);
}

static
#if PHP_VERSION_ID >= 80200
    zend_result
#else
    int
#endif
php_driver_float_cast(
        zend_object *object,
        zval *retval, int type
)
{
#if PHP_MAJOR_VERSION >= 8
  php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);
#else
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(object);
#endif

  switch (type) {
  case IS_LONG:
      ZVAL_LONG(retval, (long) self->data.floating.value);
      return SUCCESS;
  case IS_DOUBLE:
      ZVAL_DOUBLE(retval, (double) self->data.floating.value);
      return SUCCESS;
  case IS_STRING:
      return to_string(retval, self );
  default:
     return FAILURE;
  }

  return SUCCESS;
}

static void
php_driver_float_free(zend_object *object )
{
  php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_float_new(zend_class_entry *ce )
{
  php_driver_numeric *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(numeric, ce);

  PHP5TO7_ZEND_OBJECT_INIT_EX(numeric, float, self, ce);
}

void php_driver_define_Float()
{
  php_driver_float_ce = register_class_Cassandra_Float(php_driver_value_ce, php_driver_numeric_ce);
  php_driver_float_ce->create_object = php_driver_float_new;

  memcpy(&php_driver_float_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_float_handlers.std.get_properties = php_driver_float_properties;
  php_driver_float_handlers.std.get_gc = php_driver_float_gc;
  php_driver_float_handlers.std.compare = php_driver_float_compare;
  php_driver_float_handlers.std.cast_object = php_driver_float_cast;
  php_driver_float_handlers.hash_value = php_driver_float_hash_value;
  php_driver_float_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()