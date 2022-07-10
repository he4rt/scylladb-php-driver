#include "php_driver.h"
#include "php_driver_types.h"
#include "util/math.h"
#include "util/types.h"

#include <CassandraDriverAPI.h>
#include <Exception/Exceptions.h>
#include <Types/Numerics/Numerics.h>

#include "Smallint_arginfo.h"

PHP_DRIVER_API zend_class_entry* phpDriverSmallintCe = NULL;

static ZEND_RESULT_CODE
ToDouble(zval* result, PhpDriverNumeric* smallint)
{
  ZVAL_DOUBLE(result, (double) smallint->data.smallint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
ToLong(zval* result, PhpDriverNumeric* smallint)
{
  ZVAL_LONG(result, (zend_long) smallint->data.smallint.value);
  return SUCCESS;
}

static ZEND_RESULT_CODE
ToString(zval* result, PhpDriverNumeric* smallint)
{
  char* string;
  spprintf(&string, 0, "%d", smallint->data.smallint.value);
  ZVAL_STRING(result, string);
  efree(string);
  return SUCCESS;
}

void
PhpDriverSmallintInit(INTERNAL_FUNCTION_PARAMETERS)
{
  PhpDriverNumeric* self;
  zval* value;
  cass_int32_t number;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  if (getThis() && instanceof_function(Z_OBJCE_P(getThis()), phpDriverSmallintCe)) {
    self = PHP_DRIVER_NUMERIC_OBJECT(getThis());
  } else {
    object_init_ex(return_value, phpDriverSmallintCe);
    self = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  }

  if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), phpDriverSmallintCe)) {
    PhpDriverNumeric* other   = PHP_DRIVER_NUMERIC_OBJECT(value);
    self->data.smallint.value = other->data.smallint.value;
  } else {
    if (Z_TYPE_P(value) == IS_LONG) {
      number = (cass_int32_t) Z_LVAL_P(value);

      if (number < INT16_MIN || number > INT16_MAX) {
        zend_throw_exception_ex(spl_ce_RangeException, 0,
                                "value must be between -32768 and 32767, %ld given", Z_LVAL_P(value));
        return;
      }
    } else if (Z_TYPE_P(value) == IS_DOUBLE) {
      number = (cass_int32_t) Z_DVAL_P(value);

      if (number < INT16_MIN || number > INT16_MAX) {
        zend_throw_exception_ex(spl_ce_RangeException, 0,
                                "value must be between -32768 and 32767, %g given", Z_DVAL_P(value));
        return;
      }
    } else if (Z_TYPE_P(value) == IS_STRING) {
      if (!php_driver_parse_int(Z_STRVAL_P(value), Z_STRLEN_P(value),
                                &number)) {
        // If the parsing function fails, it would have set an exception. If it's
        // a range error, the error message would be wrong because the parsing
        // function supports all 32-bit values, so the "valid" range it reports would
        // be too large for Smallint. Reset the exception in that case.

        if (errno == ERANGE) {
          zend_throw_exception_ex(spl_ce_RangeException, 0,
                                  "value must be between -32768 and 32767, %s given", Z_STRVAL_P(value));
        }
        return;
      }

      if (number < INT16_MIN || number > INT16_MAX) {
        zend_throw_exception_ex(spl_ce_RangeException, 0,
                                "value must be between -32768 and 32767, %s given", Z_STRVAL_P(value));
        return;
      }
    } else {
      INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Smallint");
    }
    self->data.smallint.value = (cass_int16_t) number;
  }
}

/* {{{ Smallint::__construct(string) */
ZEND_METHOD(Cassandra_Smallint, __construct)
{
  PhpDriverSmallintInit(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Smallint::__toString() */
ZEND_METHOD(Cassandra_Smallint, __toString)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToString(return_value, self);
}
/* }}} */

/* {{{ Smallint::type() */
ZEND_METHOD(Cassandra_Smallint, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_SMALL_INT);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Smallint::value() */
ZEND_METHOD(Cassandra_Smallint, value)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToLong(return_value, self);
}
/* }}} */

/* {{{ Smallint::add() */
ZEND_METHOD(Cassandra_Smallint, add)
{
  zval* addend;
  PhpDriverNumeric* self;
  PhpDriverNumeric* smallint;
  PhpDriverNumeric* result;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &addend) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(addend) == IS_OBJECT && instanceof_function(Z_OBJCE_P(addend), phpDriverSmallintCe)) {
    self     = PHP_DRIVER_NUMERIC_OBJECT(getThis());
    smallint = PHP_DRIVER_NUMERIC_OBJECT(addend);

    object_init_ex(return_value, phpDriverSmallintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    result->data.smallint.value = self->data.smallint.value + smallint->data.smallint.value;
    if (result->data.smallint.value - smallint->data.smallint.value != self->data.smallint.value) {
      zend_throw_exception_ex(spl_ce_RangeException, 0, "Sum is out of range");
      return;
    }
  } else {
    INVALID_ARGUMENT(addend, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
  }
}
/* }}} */

/* {{{ Smallint::sub() */
ZEND_METHOD(Cassandra_Smallint, sub)
{
  zval* difference;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &difference) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(difference) == IS_OBJECT && instanceof_function(Z_OBJCE_P(difference), phpDriverSmallintCe)) {
    PhpDriverNumeric* self     = PHP_DRIVER_NUMERIC_THIS();
    PhpDriverNumeric* smallint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(difference);

    object_init_ex(return_value, phpDriverSmallintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    result->data.smallint.value = self->data.smallint.value - smallint->data.smallint.value;
    if (result->data.smallint.value + smallint->data.smallint.value != self->data.smallint.value) {
      zend_throw_exception_ex(spl_ce_RangeException, 0, "Difference is out of range");
      return;
    }
  } else {
    INVALID_ARGUMENT(difference, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
  }
}
/* }}} */

/* {{{ Smallint::mul() */
ZEND_METHOD(Cassandra_Smallint, mul)
{
  zval* multiplier;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &multiplier) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(multiplier) == IS_OBJECT && instanceof_function(Z_OBJCE_P(multiplier), phpDriverSmallintCe)) {
    PhpDriverNumeric* self     = PHP_DRIVER_NUMERIC_THIS();
    PhpDriverNumeric* smallint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(multiplier);

    object_init_ex(return_value, phpDriverSmallintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    result->data.smallint.value = self->data.smallint.value * smallint->data.smallint.value;
    if (smallint->data.smallint.value != 0 && result->data.smallint.value / smallint->data.smallint.value != self->data.smallint.value) {
      zend_throw_exception_ex(spl_ce_RangeException, 0, "Product is out of range");
      return;
    }
  } else {
    INVALID_ARGUMENT(multiplier, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
  }
}
/* }}} */

/* {{{ Smallint::div() */
ZEND_METHOD(Cassandra_Smallint, div)
{
  zval* divisor;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &divisor) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), phpDriverSmallintCe)) {
    PhpDriverNumeric* self     = PHP_DRIVER_NUMERIC_THIS();
    PhpDriverNumeric* smallint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(divisor);

    object_init_ex(return_value, phpDriverSmallintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    if (smallint->data.smallint.value == 0) {
      zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot divide by zero");
      return;
    }

    result->data.smallint.value = self->data.smallint.value / smallint->data.smallint.value;
  } else {
    INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
  }
}
/* }}} */

/* {{{ Smallint::mod() */
ZEND_METHOD(Cassandra_Smallint, mod)
{
  zval* divisor;
  PhpDriverNumeric* result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &divisor) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), phpDriverSmallintCe)) {
    PhpDriverNumeric* self     = PHP_DRIVER_NUMERIC_THIS();
    PhpDriverNumeric* smallint = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(divisor);

    object_init_ex(return_value, phpDriverSmallintCe);
    result = PHP_DRIVER_NUMERIC_OBJECT(return_value);

    if (smallint->data.smallint.value == 0) {
      zend_throw_exception_ex(phpDriverDivideByZeroExceptionCe, 0, "Cannot modulo by zero");
      return;
    }

    result->data.smallint.value = self->data.smallint.value % smallint->data.smallint.value;
  } else {
    INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
  }
}
/* }}} */

/* {{{ Smallint::abs() */
ZEND_METHOD(Cassandra_Smallint, abs)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.smallint.value == INT16_MIN) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, phpDriverSmallintCe);
  result                      = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  result->data.smallint.value = self->data.smallint.value < 0 ? -self->data.smallint.value : self->data.smallint.value;
}
/* }}} */

/* {{{ Smallint::neg() */
ZEND_METHOD(Cassandra_Smallint, neg)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.smallint.value == INT16_MIN) {
    zend_throw_exception_ex(spl_ce_RangeException, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, phpDriverSmallintCe);
  result                      = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  result->data.smallint.value = -self->data.smallint.value;
}
/* }}} */

/* {{{ Smallint::sqrt() */
ZEND_METHOD(Cassandra_Smallint, sqrt)
{
  PhpDriverNumeric* result = NULL;
  PhpDriverNumeric* self   = PHP_DRIVER_NUMERIC_THIS();

  if (self->data.smallint.value < 0) {
    zend_throw_exception_ex(spl_ce_RangeException, 0,
                            "Cannot take a square root of a negative number");
  }

  object_init_ex(return_value, phpDriverSmallintCe);
  result                      = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  result->data.smallint.value = (cass_int16_t) sqrt((long double) self->data.smallint.value);
}
/* }}} */

/* {{{ Smallint::toInt() */
ZEND_METHOD(Cassandra_Smallint, toInt)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToLong(return_value, self);
}
/* }}} */

/* {{{ Smallint::toDouble() */
ZEND_METHOD(Cassandra_Smallint, toDouble)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_THIS();

  ToDouble(return_value, self);
}
/* }}} */

/* {{{ Smallint::min() */
ZEND_METHOD(Cassandra_Smallint, min)
{
  PhpDriverNumeric* smallint = NULL;
  object_init_ex(return_value, phpDriverSmallintCe);
  smallint                      = PHP_DRIVER_NUMERIC_OBJECT(return_value);
  smallint->data.smallint.value = INT16_MIN;
}
/* }}} */

/* {{{ Smallint::max() */
ZEND_METHOD(Cassandra_Smallint, max)
{
  PhpDriverNumeric* smallint = NULL;
  object_init_ex(return_value, phpDriverSmallintCe);
  smallint                      = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(return_value);
  smallint->data.smallint.value = INT16_MAX;
}
/* }}} */

static php_driver_value_handlers php_driver_smallint_handlers;

static HashTable*
PhpDriverSmallintGc(
  zend_object* object,
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
PhpDriverSmallintProperties(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);
  HashTable* props       = zend_std_get_properties(object);

  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_SMALL_INT);
  PHP5TO7_ZEND_HASH_UPDATE(props, "type", sizeof("type"), &type, sizeof(zval));

  zval value;
  ToString(&value, self);
  PHP5TO7_ZEND_HASH_UPDATE(props, "value", sizeof("value"), &value, sizeof(zval));

  return props;
}

static int
PhpDriverSmallintCompare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  PhpDriverNumeric* smallint1 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj1);
  PhpDriverNumeric* smallint2 = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj2);

  if (smallint1->data.smallint.value == smallint2->data.smallint.value) {
    return 0;
  }

  if (smallint1->data.smallint.value < smallint2->data.smallint.value) {
    return -1;
  }

  return 1;
}

static uint32_t
PhpDriverSmallintHashValue(zval* obj)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj);
  return 31 * 17 + self->data.smallint.value;
}

static int
PhpDriverSmallintCast(
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
PhpDriverSmallintFree(zend_object* object)
{
  PhpDriverNumeric* self = PHP_DRIVER_NUMERIC_OBJECT(object);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
PhpDriverSmallintNew(zend_class_entry* ce)
{
  PhpDriverNumeric* self = make(PhpDriverNumeric);

  self->type          = PHP_DRIVER_SMALLINT;
  self->zval.handlers = &php_driver_smallint_handlers.std;

  zend_object_std_init(&self->zval, ce);

  return &self->zval;
}

void
PhpDriverDefineSmallint(zend_class_entry* valueInterface, zend_class_entry* numericInterface)
{
  phpDriverSmallintCe                = register_class_Cassandra_Smallint(valueInterface, numericInterface);
  phpDriverSmallintCe->create_object = PhpDriverSmallintNew;

  memcpy(&php_driver_smallint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_smallint_handlers.std.get_properties = PhpDriverSmallintProperties;
  php_driver_smallint_handlers.std.get_gc         = PhpDriverSmallintGc;
  php_driver_smallint_handlers.std.compare        = PhpDriverSmallintCompare;
  php_driver_smallint_handlers.std.cast_object    = PhpDriverSmallintCast;
  php_driver_smallint_handlers.std.free_obj       = PhpDriverSmallintFree;
  php_driver_smallint_handlers.hash_value         = PhpDriverSmallintHashValue;
  php_driver_smallint_handlers.std.offset         = XtOffsetOf(PhpDriverNumeric, zval);
}
