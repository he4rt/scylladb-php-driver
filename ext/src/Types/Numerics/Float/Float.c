#include <CassandraDriverAPI.h>
#include <Exception/Exceptions.h>
#include <Types/Numerics/Numerics.h>
#include <float.h>

#include "Float.h"
#include "Float_arginfo.h"
#include "util/math.h"
#include "util/types.h"

PHP_DRIVER_API zend_class_entry* phpDriverFloatCe = NULL;

static zend_always_inline ZEND_RESULT_CODE
ToString(zval* result, PhpDriverNumeric* flt)
{
  smart_str string = { 0 };
  smart_str_append_double(&string, (double) flt->data.floating.value, EG(precision), false);
  smart_str_0(&string);

  ZVAL_STR(result, string.s);
  return SUCCESS;
}

void zend_always_inline
PhpDriverFloatInit(PhpDriverNumeric* self, zval* value)
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
    if (instanceof_function(Z_OBJCE_P(value), phpDriverFloatCe)) {
      PhpDriverNumeric* other   = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(value);
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

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  PhpDriverFloatInit(self, value);
}
/* }}} */

/* {{{ Float::__toString() */
ZEND_METHOD(Cassandra_Float, __toString)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToString(return_value, self);
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
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::isInfinite() */
ZEND_METHOD(Cassandra_Float, isInfinite)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_BOOL(zend_isinf(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isFinite() */
ZEND_METHOD(Cassandra_Float, isFinite)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  RETURN_BOOL(zend_finite(self->data.floating.value));
}
/* }}} */

/* {{{ Float::isNaN() */
ZEND_METHOD(Cassandra_Float, isNaN)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
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

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), phpDriverFloatCe))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = self->data.floating.value + flt->data.floating.value;
}
/* }}} */

/* {{{ Float::sub() */
ZEND_METHOD(Cassandra_Float, sub)
{
  zval* num;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), phpDriverFloatCe))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverFloatCe);
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

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), phpDriverFloatCe))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

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

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), phpDriverFloatCe))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  if (flt->data.floating.value == 0) {
    zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot divide by zero");
    return;
  }

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

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

  if ((Z_TYPE_P(num) != IS_OBJECT && !instanceof_function(Z_OBJCE_P(num), phpDriverFloatCe))) {
    INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Float");
  }

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();
  PhpDriverNumeric* flt  = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(num);

  if (flt->data.floating.value == 0) {
    zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot divide by zero");
    return;
  }

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result    = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.floating.value = (cass_float_t) fmod(self->data.floating.value, flt->data.floating.value);
}

/* {{{ Float::abs() */
ZEND_METHOD(Cassandra_Float, abs)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = fabsf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::neg() */
ZEND_METHOD(Cassandra_Float, neg)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = -self->data.floating.value;
}
/* }}} */

/* {{{ Float::sqrt() */
ZEND_METHOD(Cassandra_Float, sqrt)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.floating.value < 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0,
                            "Cannot take a square root of a negative number");
    return;
  }

  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* result = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  result->data.floating.value = sqrtf(self->data.floating.value);
}
/* }}} */

/* {{{ Float::toInt() */
ZEND_METHOD(Cassandra_Float, toInt)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  RETURN_LONG((zend_long) self->data.floating.value);
}
/* }}} */

/* {{{ Float::toDouble() */
ZEND_METHOD(Cassandra_Float, toDouble)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  RETURN_DOUBLE((double) self->data.floating.value);
}
/* }}} */

/* {{{ Float::min() */
ZEND_METHOD(Cassandra_Float, min)
{
  object_init_ex(return_value, phpDriverFloatCe);

  PhpDriverNumeric* flt    = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  flt->data.floating.value = FLT_MIN;
}
/* }}} */

/* {{{ Float::max() */
ZEND_METHOD(Cassandra_Float, max)
{
  object_init_ex(return_value, phpDriverFloatCe);
  PhpDriverNumeric* flt = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);

  flt->data.floating.value = FLT_MAX;
}
/* }}} */

static php_driver_value_handlers php_driver_float_handlers;

static HashTable*
PhpDriverFloatGc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
PhpDriverFloatProperties(zend_object* object)
{
  zval type;
  zval value;

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props       = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_FLOAT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  ToString(&value, self);
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
PhpDriverFloatCompare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  PhpDriverNumeric* flt1 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj1);
  PhpDriverNumeric* flt2 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj2);

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
PhpDriverFloatHashValue(zval* obj)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj);
  return float_to_bits(self->data.floating.value);
}

static int
PhpDriverFloatCast(
  zend_object* object,
  zval* retval,
  int type)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

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
    return ToString(retval, self);
  }
  default: {
    return FAILURE;
  }
  }
}

void
PhpDriverFloatFree(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  zend_object_std_dtor(&self->zval);
}

zend_object*
PhpDriverFloatNew(zend_class_entry* ce)
{
  PhpDriverNumeric* self = make(PhpDriverNumeric);

  self->type = PHP_DRIVER_FLOAT;

  zend_object_std_init(&self->zval, ce);
  self->zval.handlers = &php_driver_float_handlers.std;

  return &self->zval;
}

void
PhpDriverDefineFloat(zend_class_entry* value_interface, zend_class_entry* numeric_interface)
{
  phpDriverFloatCe                = register_class_Cassandra_Float(value_interface, numeric_interface);
  phpDriverFloatCe->create_object = PhpDriverFloatNew;

  memcpy(&php_driver_float_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  php_driver_float_handlers.std.get_properties = PhpDriverFloatProperties;
  php_driver_float_handlers.std.get_gc         = PhpDriverFloatGc;
  php_driver_float_handlers.std.compare        = PhpDriverFloatCompare;
  php_driver_float_handlers.std.cast_object    = PhpDriverFloatCast;
  php_driver_float_handlers.std.free_obj       = PhpDriverFloatFree;
  php_driver_float_handlers.hash_value         = PhpDriverFloatHashValue;
  php_driver_float_handlers.std.offset         = XtOffsetOf(PhpDriverNumeric, zval);
  php_driver_float_handlers.std.clone_obj      = NULL;
}
