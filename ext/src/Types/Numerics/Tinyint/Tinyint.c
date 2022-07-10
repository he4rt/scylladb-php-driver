#include <CassandraDriverAPI.h>
#include <Exception/Exceptions.h>
#include <Types/Numerics/Numerics.h>

#include "Tinyint.h"
#include "Tinyint_arginfo.h"
#include "util/math.h"
#include "util/types.h"

PHP_DRIVER_API zend_class_entry* phpDriverTinyintCe = NULL;

static ZEND_RESULT_CODE
ToDouble(zval* result, PhpDriverNumeric* tinyint)
{
  ZVAL_DOUBLE(result, (double) tinyint->data.tinyint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
ToLong(zval* result, PhpDriverNumeric* tinyint)
{
  ZVAL_LONG(result, (zend_long) tinyint->data.tinyint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
ToString(zval* result, PhpDriverNumeric* tinyint)
{
  char* string;
  spprintf(&string, 0, "%d", tinyint->data.tinyint.value);
  ZVAL_STRING(result, string);
  efree(string);
  return SUCCESS;
}

void
PhpDriverTinyintInit(INTERNAL_FUNCTION_PARAMETERS)
{
  PhpDriverNumeric* self;
  zval* value;
  cass_int32_t number;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  if (getThis() && instanceof_function(Z_OBJCE_P(getThis()), phpDriverTinyintCe)) {
    self = PHP_DRIVER_NUMERIC_OBJECT(getThis());
  } else {
    object_init_ex(return_value, phpDriverTinyintCe);
    self = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  }

  if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), phpDriverTinyintCe)) {
    PhpDriverNumeric* other  = PHP_DRIVER_NUMERIC_OBJECT(value);
    self->data.tinyint.value = other->data.tinyint.value;
  } else {
    if (Z_TYPE_P(value) == IS_LONG) {
      number = (cass_int32_t) Z_LVAL_P(value);

      if (number < INT8_MIN || number > INT8_MAX) {
        zend_throw_exception_ex(spl_ce_RangeException, 0,
                                "value must be between -128 and 127, %ld given", Z_LVAL_P(value));
        return;
      }
    } else if (Z_TYPE_P(value) == IS_DOUBLE) {
      number = (cass_int32_t) Z_DVAL_P(value);

      if (number < INT8_MIN || number > INT8_MAX) {
        zend_throw_exception_ex(spl_ce_RangeException, 0,
                                "value must be between -128 and 127, %g given", Z_DVAL_P(value));
        return;
      }
    } else if (Z_TYPE_P(value) == IS_STRING) {
      if (!php_driver_parse_int(Z_STRVAL_P(value), Z_STRLEN_P(value),
                                &number)) {
        // If the parsing function fails, it would have set an exception. If it's
        // a range error, the error message would be wrong because the parsing
        // function supports all 32-bit values, so the "valid" range it reports would
        // be too large for Tinyint. Reset the exception in that case.

        if (errno == ERANGE) {
          zend_throw_exception_ex(spl_ce_RangeException, 0,
                                  "value must be between -128 and 127, %s given", Z_STRVAL_P(value));
        }
        return;
      }

      if (number < INT8_MIN || number > INT8_MAX) {
        zend_throw_exception_ex(spl_ce_RangeException, 0,
                                "value must be between -128 and 127, %s given", Z_STRVAL_P(value));
        return;
      }
    } else {
      INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Tinyint");
    }
    self->data.tinyint.value = (cass_int8_t) number;
  }
}

/* {{{ Tinyint::__construct(string) */
ZEND_METHOD(Cassandra_Tinyint, __construct)
{
  PhpDriverTinyintInit(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Tinyint::__toString() */
ZEND_METHOD(Cassandra_Tinyint, __toString)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToString(return_value, self);
}
/* }}} */

/* {{{ Tinyint::type() */
ZEND_METHOD(Cassandra_Tinyint, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TINY_INT);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Tinyint::value() */
ZEND_METHOD(Cassandra_Tinyint, value)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  ToLong(return_value, self);
}
/* }}} */

/* {{{ Tinyint::add() */
ZEND_METHOD(Cassandra_Tinyint, add)
{
  zval* addend;
  PhpDriverNumeric* self;
  PhpDriverNumeric* tinyint;
  PhpDriverNumeric* result;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &addend) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(addend) == IS_OBJECT && instanceof_function(Z_OBJCE_P(addend), phpDriverTinyintCe)) {
    self    = PHP_DRIVER_NUMERIC_OBJECT(getThis());
    tinyint = PHP_DRIVER_NUMERIC_OBJECT(addend);

    object_init_ex(return_value, phpDriverTinyintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    result->data.tinyint.value = self->data.tinyint.value + tinyint->data.tinyint.value;
    if (result->data.tinyint.value - tinyint->data.tinyint.value != self->data.tinyint.value) {
      zend_throw_exception_ex(spl_ce_RangeException, 0, "Sum is out of range");
      return;
    }
  } else {
    INVALID_ARGUMENT(addend, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
  }
}
/* }}} */

/* {{{ Tinyint::sub() */
ZEND_METHOD(Cassandra_Tinyint, sub)
{
  zval* difference;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &difference) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(difference) == IS_OBJECT && instanceof_function(Z_OBJCE_P(difference), phpDriverTinyintCe)) {
    PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_OBJECT(getThis());
    PhpDriverNumeric* tinyint = PHP_DRIVER_NUMERIC_OBJECT(difference);

    object_init_ex(return_value, phpDriverTinyintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    result->data.tinyint.value = self->data.tinyint.value - tinyint->data.tinyint.value;
    if (result->data.tinyint.value + tinyint->data.tinyint.value != self->data.tinyint.value) {
      zend_throw_exception_ex(spl_ce_RangeException, 0, "Difference is out of range");
      return;
    }
  } else {
    INVALID_ARGUMENT(difference, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
  }
}
/* }}} */

/* {{{ Tinyint::mul() */
ZEND_METHOD(Cassandra_Tinyint, mul)
{
  zval* multiplier;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &multiplier) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(multiplier) == IS_OBJECT && instanceof_function(Z_OBJCE_P(multiplier), phpDriverTinyintCe)) {
    PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_OBJECT(getThis());
    PhpDriverNumeric* tinyint = PHP_DRIVER_NUMERIC_OBJECT(multiplier);

    object_init_ex(return_value, phpDriverTinyintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    result->data.tinyint.value = self->data.tinyint.value * tinyint->data.tinyint.value;
    if (tinyint->data.tinyint.value != 0 && result->data.tinyint.value / tinyint->data.tinyint.value != self->data.tinyint.value) {
      zend_throw_exception_ex(spl_ce_RangeException, 0, "Product is out of range");
      return;
    }
  } else {
    INVALID_ARGUMENT(multiplier, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
  }
}
/* }}} */

/* {{{ Tinyint::div() */
ZEND_METHOD(Cassandra_Tinyint, div)
{
  zval* divisor;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &divisor) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), phpDriverTinyintCe)) {
    PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_OBJECT(getThis());
    PhpDriverNumeric* tinyint = PHP_DRIVER_NUMERIC_OBJECT(divisor);

    object_init_ex(return_value, phpDriverTinyintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    if (tinyint->data.tinyint.value == 0) {
      zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot divide by zero");
      return;
    }

    result->data.tinyint.value = self->data.tinyint.value / tinyint->data.tinyint.value;
  } else {
    INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
  }
}
/* }}} */

/* {{{ Tinyint::mod() */
ZEND_METHOD(Cassandra_Tinyint, mod)
{
  zval* divisor;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &divisor) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), phpDriverTinyintCe)) {
    PhpDriverNumeric* self    = PHP_DRIVER_NUMERIC_OBJECT(getThis());
    PhpDriverNumeric* tinyint = PHP_DRIVER_NUMERIC_OBJECT(divisor);

    object_init_ex(return_value, phpDriverTinyintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    if (tinyint->data.tinyint.value == 0) {
      zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot modulo by zero");
      return;
    }

    result->data.tinyint.value = self->data.tinyint.value % tinyint->data.tinyint.value;
  } else {
    INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
  }
}
/* }}} */

/* {{{ Tinyint::abs() */
ZEND_METHOD(Cassandra_Tinyint, abs)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  if (self->data.tinyint.value == INT8_MIN) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, phpDriverTinyintCe);
  result                     = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  result->data.tinyint.value = self->data.tinyint.value < 0 ? -self->data.tinyint.value : self->data.tinyint.value;
}
/* }}} */

/* {{{ Tinyint::neg() */
ZEND_METHOD(Cassandra_Tinyint, neg)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  if (self->data.tinyint.value == INT8_MIN) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, phpDriverTinyintCe);
  result                     = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  result->data.tinyint.value = -self->data.tinyint.value;
}
/* }}} */

/* {{{ Tinyint::sqrt() */
ZEND_METHOD(Cassandra_Tinyint, sqrt)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  if (self->data.tinyint.value < 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0,
                            "Cannot take a square root of a negative number");
    return;
  }

  object_init_ex(return_value, phpDriverTinyintCe);
  result                     = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  result->data.tinyint.value = (cass_int8_t) sqrt((long double) self->data.tinyint.value);
}
/* }}} */

/* {{{ Tinyint::toInt() */
ZEND_METHOD(Cassandra_Tinyint, toInt)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  ToLong(return_value, self);
}
/* }}} */

/* {{{ Tinyint::toDouble() */
ZEND_METHOD(Cassandra_Tinyint, toDouble)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(getThis());

  ToDouble(return_value, self);
}
/* }}} */

/* {{{ Tinyint::min() */
ZEND_METHOD(Cassandra_Tinyint, min)
{
  PhpDriverNumeric* tinyint = NULL;
  object_init_ex(return_value, phpDriverTinyintCe);
  tinyint                     = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  tinyint->data.tinyint.value = INT8_MIN;
}
/* }}} */

/* {{{ Tinyint::max() */
ZEND_METHOD(Cassandra_Tinyint, max)
{
  PhpDriverNumeric* tinyint = NULL;
  object_init_ex(return_value, phpDriverTinyintCe);
  tinyint                     = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  tinyint->data.tinyint.value = INT8_MAX;
}
/* }}} */

static php_driver_value_handlers phpDriverTinyintHandlers;

static HashTable*
PhpDriverTinyintGc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
PhpDriverTinyintProperties(
  zend_object* object)
{
  zval type;
  zval value;

  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props       = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_TINY_INT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), PHP5TO7_ZVAL_MAYBE_P(type), sizeof(zval));

  ToString(PHP5TO7_ZVAL_MAYBE_P(value), self);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), PHP5TO7_ZVAL_MAYBE_P(value), sizeof(zval));

  return props;
}

static int
PhpDriverTinyintCompare(zval* obj1, zval* obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  PhpDriverNumeric* tinyint1 = NULL;
  PhpDriverNumeric* tinyint2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  tinyint1 = PHP_DRIVER_NUMERIC_OBJECT(obj1);
  tinyint2 = PHP_DRIVER_NUMERIC_OBJECT(obj2);

  if (tinyint1->data.tinyint.value == tinyint2->data.tinyint.value)
    return 0;
  else if (tinyint1->data.tinyint.value < tinyint2->data.tinyint.value)
    return -1;
  else
    return 1;
}

static unsigned
PhpDriverTinyintHashValue(zval* obj)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(obj);
  return 31 * 17 + self->data.tinyint.value;
}

static int
PhpDriverTinyintCast(
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
PhpDriverTinyintFree(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
PhpDriverTinyintNew(zend_class_entry* ce)
{
  PhpDriverNumeric* self = make(PhpDriverNumeric);
  self->type             = PHP_DRIVER_TINYINT;
  self->zval.handlers    = &phpDriverTinyintHandlers.std;

  zend_object_std_init(&self->zval, ce);

  return &self->zval;
}

void
PhpDriverDefineTinyint(zend_class_entry* valueInterface, zend_class_entry* numericInterface)
{
  phpDriverTinyintCe                = register_class_Cassandra_Tinyint(valueInterface, numericInterface);
  phpDriverTinyintCe->create_object = PhpDriverTinyintNew;

  memcpy(&phpDriverTinyintHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  phpDriverTinyintHandlers.std.get_properties = PhpDriverTinyintProperties;
  phpDriverTinyintHandlers.std.get_gc         = PhpDriverTinyintGc;
  phpDriverTinyintHandlers.std.compare        = PhpDriverTinyintCompare;
  phpDriverTinyintHandlers.std.cast_object    = PhpDriverTinyintCast;
  phpDriverTinyintHandlers.std.free_obj       = PhpDriverTinyintFree;
  phpDriverTinyintHandlers.hash_value         = PhpDriverTinyintHashValue;
  phpDriverTinyintHandlers.std.offset         = XtOffsetOf(PhpDriverNumeric, zval);
}
