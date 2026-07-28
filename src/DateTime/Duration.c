#include "Duration.h"

#include <php_scylladb.h>
#include <php_scylladb_types.h>
#include "Type/ValueHash.h"
#include "Numbers/NumberParser.h"
#include "Type/TypeFactory.h"

#include <spl/spl_exceptions.h>
#include "Duration_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_duration_handlers;


static void to_string(zval *result, cass_int64_t value)
{
  char *string;
  spprintf(&string, 0, "%" PRId64, value);
  ZVAL_STRING(result, string);
  efree(string);
}

static int get_param(zval* value,
                     const char* param_name,
                     cass_int64_t min,
                     cass_int64_t max,
                     cass_int64_t *destination  )
{
  if (Z_TYPE_P(value) == IS_LONG) {
    zend_long long_value = Z_LVAL_P(value);

    if (long_value > max || long_value < min) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
        "%s must be between %" PRId64 " and %" PRId64 ", %" PRId64 " given",
        param_name, min, max, (int64_t)long_value);
      return 0;
    }

    *destination = long_value;
  } else if (Z_TYPE_P(value) == IS_DOUBLE) {
    double double_value = Z_DVAL_P(value);

    if (double_value > max || double_value < min) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
        "%s must be between %" PRId64 " and %" PRId64 ", %g given",
        param_name, min, max, double_value);
      return 0;
    }
    *destination = (cass_int64_t) double_value;
  } else if (Z_TYPE_P(value) == IS_STRING) {
    cass_int64_t parsed_big_int;
    if (!php_scylladb_parse_bigint(Z_STRVAL_P(value), Z_STRLEN_P(value), &parsed_big_int )) {
      return 0;
    }

    if (parsed_big_int > max || parsed_big_int < min) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
        "%s must be between " "%" PRId64 " and " "%" PRId64 ", " "%" PRId64 " given",
        param_name, min, max, parsed_big_int);
      return 0;
    }
    *destination = parsed_big_int;
  } else if (Z_TYPE_P(value) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(value), php_scylladb_bigint_ce )) {
    auto bigint = PHP_SCYLLADB_GET_NUMERIC(value);
    cass_int64_t bigint_value = bigint->data.bigint.value;

    if (bigint_value > max || bigint_value < min) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 ,
        "%s must be between " "%" PRId64 " and " "%" PRId64 ", " "%" PRId64 " given",
        param_name, min, max, bigint_value);
      return 0;
    }

    *destination = bigint_value;
  } else {
    throw_invalid_argument(value, param_name, "a long, a double, a numeric string or a " \
                            PHP_SCYLLADB_NAMESPACE "\\Bigint" );
    return 0;
  }
  return 1;
}

char *php_scylladb_duration_to_string(php_scylladb_duration *duration)
{
  // String representation of Duration is of the form -?MmoDdNns, for int M, D, N.
  // Negative durations lead with a minus sign. So (-3, -2, -1) results in
  // -3mo2d1ns.

  char* rep;
  int is_negative = 0;
  cass_int32_t final_months = duration->months;
  cass_int32_t final_days = duration->days;
  cass_int64_t final_nanos = duration->nanos;

  is_negative = final_months < 0 || final_days < 0 || final_nanos < 0;
  if (final_months < 0)
    final_months = -final_months;
  if (final_days < 0)
    final_days = -final_days;
  if (final_nanos < 0)
    final_nanos = -final_nanos;

  spprintf(&rep, 0, "%s%dmo%dd%" PRId64 "ns", is_negative ? "-" : "", final_months, final_days, final_nanos);
  return rep;
}

void
php_scylladb_duration_init(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *months, *days, *nanos;
  cass_int64_t param;
  php_scylladb_duration *self = nullptr;

  ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_ZVAL(months)
    Z_PARAM_ZVAL(days)
    Z_PARAM_ZVAL(nanos)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_DURATION(getThis());

  if (!get_param(months, "months", INT32_MIN, INT32_MAX, &param  )) {
    return;
  }
  self->months = (cass_int32_t)param;

  if (!get_param(days, "days", INT32_MIN, INT32_MAX, &param )) {
    return;
  }
  self->days = (cass_int32_t)param;

  if (!get_param(nanos, "nanos", INT64_MIN, INT64_MAX, &self->nanos )) {
    return;
  }

  // Verify that all three attributes are non-negative or non-positive.
  if (!(self->months <= 0 && self->days <= 0 && self->nanos <= 0) &&
      !(self->months >= 0 && self->days >= 0 && self->nanos >= 0)) {
    zend_throw_exception_ex(spl_ce_BadFunctionCallException, 0 , "%s",
      "A duration must have all non-negative or non-positive attributes"
    );
  }
}

ZEND_METHOD(Cassandra_Duration, __construct)
{
  php_scylladb_duration_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Cassandra_Duration, __toString)
{
  char* rep;
  php_scylladb_duration *self = nullptr;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_DURATION(getThis());

  // Build up string representation of this duration.
  rep = php_scylladb_duration_to_string(self);
  RETVAL_STRING(rep);
  efree(rep);
}

ZEND_METHOD(Cassandra_Duration, type)
{
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_DURATION );
  RETURN_ZVAL(&type, 1, 1);
}

ZEND_METHOD(Cassandra_Duration, months)
{
  php_scylladb_duration *self = nullptr;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_DURATION(getThis());
  to_string(return_value, self->months);
}

ZEND_METHOD(Cassandra_Duration, days)
{
  php_scylladb_duration *self = nullptr;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_DURATION(getThis());
  to_string(return_value, self->days);
}

ZEND_METHOD(Cassandra_Duration, nanos)
{
  php_scylladb_duration *self = nullptr;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_DURATION(getThis());
  to_string(return_value, self->nanos);
}


HashTable *php_scylladb_duration_gc(zend_object *object, zval **table, int *n)
{
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *
php_scylladb_duration_properties(zend_object *object)
{
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(3);
  HashTable *props = object->properties;

  auto self = php_scylladb_duration_object_fetch(object);
  zval wrapped_months, wrapped_days, wrapped_nanos;

  ZVAL_LONG(&wrapped_months, self->months);
  ZVAL_LONG(&wrapped_days, self->days);
  ZVAL_LONG(&wrapped_nanos, self->nanos);
  (void)zend_hash_str_update(props, ZEND_STRL("months"), &wrapped_months);
  (void)zend_hash_str_update(props, ZEND_STRL("days"), &wrapped_days);
  (void)zend_hash_str_update(props, ZEND_STRL("nanos"), &wrapped_nanos);

  return props;
}

int
php_scylladb_duration_compare(zval *obj1, zval *obj2 )
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_scylladb_duration *left, *right;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  left = PHP_SCYLLADB_GET_DURATION(obj1);
  right = PHP_SCYLLADB_GET_DURATION(obj2);

  // Comparisons compare months, then days, then nanos.

  if (left->months < right->months)
    return -1;

  if (left->months > right->months)
    return 1;

  // months are the same; compare days.
  if (left->days < right->days)
    return -1;

  if (left->days > right->days)
    return 1;

  // days are the same; compare nanos.
  if (left->nanos < right->nanos)
    return -1;

  return (left->nanos == right->nanos) ? 0 : 1;
}

unsigned
php_scylladb_duration_hash_value(zval *obj )
{
  auto self = PHP_SCYLLADB_GET_DURATION(obj);
  unsigned hashv = 0;

  hashv = php_scylladb_combine_hash(hashv, (unsigned) self->months);
  hashv = php_scylladb_combine_hash(hashv, (unsigned) self->days);
  hashv = php_scylladb_combine_hash(hashv, (unsigned) self->nanos);

  return hashv;
}

void
php_scylladb_duration_free(zend_object *object )
{
  auto self = php_scylladb_duration_object_fetch(object);

  /* Clean up */

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_duration_new(zend_class_entry *ce )
{
  php_scylladb_duration *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_duration, ce, &php_scylladb_duration_handlers);
  return &self->zendObject;
}


void php_scylladb_duration_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_duration_handlers.std.offset = offsetof(php_scylladb_duration, zendObject);
}
