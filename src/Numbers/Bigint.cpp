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
BEGIN_EXTERN_C()
#include "Bigint_arginfo.h"
#if !defined(HAVE_STDINT_H) && !defined(_MSC_STDINT_H_)
#define INT64_MAX 9223372036854775807LL
#define INT64_MIN (-INT64_MAX - 1)
#endif

zend_class_entry *php_driver_bigint_ce = NULL;

static zend_result to_double(zval *result, php_driver_numeric *bigint)
{
    ZVAL_DOUBLE(result, (double)bigint->data.bigint.value);
    return SUCCESS;
}

static zend_result to_long(zval *result, php_driver_numeric *bigint)
{
    if (bigint->data.bigint.value < (cass_int64_t)INT64_MIN)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too small");
        return FAILURE;
    }

    if (bigint->data.bigint.value > (cass_int64_t)INT64_MAX)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too big");
        return FAILURE;
    }

    ZVAL_LONG(result, (zend_long)bigint->data.bigint.value);
    return SUCCESS;
}

static zend_result to_string(zval *result, php_driver_numeric *bigint)
{
    char *string;
    spprintf(&string, 0, "%" PRId64, bigint->data.bigint.value);
    ZVAL_STRING(result, string);
    efree(string);
    return SUCCESS;
}

void php_driver_bigint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    php_driver_numeric *self;
    zval *value;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_bigint_ce))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_driver_bigint_ce);
        self = PHP_DRIVER_GET_NUMERIC(return_value);
    }

    if (Z_TYPE_P(value) == IS_LONG)
    {
        self->data.bigint.value = (cass_int64_t)Z_LVAL_P(value);
    }
    else if (Z_TYPE_P(value) == IS_DOUBLE)
    {
        double double_value = Z_DVAL_P(value);

        if (double_value > INT64_MAX || double_value < INT64_MIN)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0,
                                    "value must be between %" PRId64 " and %" PRId64 ", %g given", INT64_MIN,
                                    INT64_MAX, double_value);
            return;
        }

        self->data.bigint.value = (cass_int64_t)Z_DVAL_P(value);
    }
    else if (Z_TYPE_P(value) == IS_STRING)
    {
        if (!php_driver_parse_bigint(Z_STRVAL_P(value), Z_STRLEN_P(value), &self->data.bigint.value))
        {
            return;
        }
    }
    else if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_driver_bigint_ce))
    {
        php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(value);
        self->data.bigint.value = bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Bigint");
    }
}

/* {{{ Bigint::__construct(string) */
ZEND_METHOD(Cassandra_Bigint, __construct)
{
    php_driver_bigint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Bigint::__toString() */
ZEND_METHOD(Cassandra_Bigint, __toString)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

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
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self);
}
/* }}} */

/* {{{ Bigint::add() */
ZEND_METHOD(Cassandra_Bigint, add)
{
    zval *num;
    php_driver_numeric *self;
    php_driver_numeric *bigint;
    php_driver_numeric *result;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        bigint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_bigint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.bigint.value = self->data.bigint.value + bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::sub() */
ZEND_METHOD(Cassandra_Bigint, sub)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_bigint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.bigint.value = self->data.bigint.value - bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::mul() */
ZEND_METHOD(Cassandra_Bigint, mul)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_bigint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.bigint.value = self->data.bigint.value * bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::div() */
ZEND_METHOD(Cassandra_Bigint, div)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_bigint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (bigint->data.bigint.value == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
            return;
        }

        result->data.bigint.value = self->data.bigint.value / bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::mod() */
ZEND_METHOD(Cassandra_Bigint, mod)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_bigint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (bigint->data.bigint.value == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot modulo by zero");
            return;
        }

        result->data.bigint.value = self->data.bigint.value % bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::abs() */
ZEND_METHOD(Cassandra_Bigint, abs)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.bigint.value == INT64_MIN)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value doesn't exist");
        return;
    }

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.bigint.value = self->data.bigint.value < 0 ? -self->data.bigint.value : self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::neg() */
ZEND_METHOD(Cassandra_Bigint, neg)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.bigint.value = -self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::sqrt() */
ZEND_METHOD(Cassandra_Bigint, sqrt)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.bigint.value < 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0,
                                "Cannot take a square root of a negative number");
    }

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.bigint.value = (cass_int64_t)sqrt((long double)self->data.bigint.value);
}
/* }}} */

/* {{{ Bigint::toInt() */
ZEND_METHOD(Cassandra_Bigint, toInt)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self);
}
/* }}} */

/* {{{ Bigint::toDouble() */
ZEND_METHOD(Cassandra_Bigint, toDouble)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self);
}
/* }}} */

/* {{{ Bigint::min() */
ZEND_METHOD(Cassandra_Bigint, min)
{
    php_driver_numeric *bigint = NULL;
    object_init_ex(return_value, php_driver_bigint_ce);
    bigint = PHP_DRIVER_GET_NUMERIC(return_value);
    bigint->data.bigint.value = INT64_MIN;
}
/* }}} */

/* {{{ Bigint::max() */
ZEND_METHOD(Cassandra_Bigint, max)
{
    php_driver_numeric *bigint = NULL;
    object_init_ex(return_value, php_driver_bigint_ce);
    bigint = PHP_DRIVER_GET_NUMERIC(return_value);
    bigint->data.bigint.value = INT64_MAX;
}
/* }}} */

static php_driver_value_handlers php_driver_bigint_handlers;

static HashTable *php_driver_bigint_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object,
#else
    zendObject *object,
#endif
    zval** table, int *n)
{
    *table = NULL;
    *n = 0;
    return NULL;
}

static HashTable *php_driver_bigint_properties(
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
    php_driver_numeric *self = php_driver_numeric_object_fetch(object);
#else
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(object);
#endif
    if (object->properties) {
        zend_array_release(object->properties);
    }
    object->properties = zend_new_array(2);
    HashTable *props = object->properties;

    type = php_driver_type_scalar(CASS_VALUE_TYPE_BIGINT);
    (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);


    to_string(&value, self);
    (void)zend_hash_str_update(props, ZEND_STRL("value"), &value);

    return props;
}

static int php_driver_bigint_compare(zval *obj1, zval *obj2)
{
#if PHP_MAJOR_VERSION >= 8
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
    php_driver_numeric *bigint1 = NULL;
    php_driver_numeric *bigint2 = NULL;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return 1; /* different classes */

    bigint1 = PHP_DRIVER_GET_NUMERIC(obj1);
    bigint2 = PHP_DRIVER_GET_NUMERIC(obj2);

    if (bigint1->data.bigint.value == bigint2->data.bigint.value)
        return 0;
    else if (bigint1->data.bigint.value < bigint2->data.bigint.value)
        return -1;
    else
        return 1;
}

static unsigned php_driver_bigint_hash_value(zval *obj)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(obj);
    return (unsigned)(self->data.bigint.value ^ (self->data.bigint.value >> 32));
}

static
#if PHP_VERSION_ID >= 80200
    zend_result
#else
    int
#endif
    php_driver_bigint_cast(zend_object *object, zval *retval, int type)
{
#if PHP_MAJOR_VERSION >= 8
    php_driver_numeric *self = php_driver_numeric_object_fetch(object);
#else
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(object);
#endif

    switch (type)
    {
    case IS_LONG:
        return to_long(retval, self);
    case IS_DOUBLE:
        return to_double(retval, self);
    case IS_STRING:
        return to_string(retval, self);
    default:
        return FAILURE;
    }

    return SUCCESS;
}

static void php_driver_bigint_free(zend_object *object)
{
    php_driver_numeric *self = php_driver_numeric_object_fetch(object);

    zend_object_std_dtor(&self->zendObject);

}

static zend_object* php_driver_bigint_new(zend_class_entry *ce)
{
    php_driver_numeric *self = (php_driver_numeric *)ecalloc(1, sizeof(php_driver_numeric) + zend_object_properties_size(ce));

    self->type = PHP_DRIVER_BIGINT;

    zend_object_std_init(&self->zendObject, ce);
    self->zendObject.handlers = (zend_object_handlers *)&php_driver_bigint_handlers;
    return &self->zendObject;
}

void php_driver_define_Bigint()
{
    php_driver_bigint_ce = register_class_Cassandra_Bigint(php_driver_value_ce, php_driver_numeric_ce);
    php_driver_bigint_ce->create_object = php_driver_bigint_new;

    memcpy(&php_driver_bigint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_bigint_handlers.std.offset = XtOffsetOf(php_driver_numeric, zendObject);
    php_driver_bigint_handlers.std.free_obj = php_driver_bigint_free;
    php_driver_bigint_handlers.std.get_properties = php_driver_bigint_properties;
    php_driver_bigint_handlers.std.get_gc = php_driver_bigint_gc;
    php_driver_bigint_handlers.std.compare = php_driver_bigint_compare;
    php_driver_bigint_handlers.std.cast_object = (zend_object_cast_t)php_driver_bigint_cast;
    php_driver_bigint_handlers.hash_value = php_driver_bigint_hash_value;
    php_driver_bigint_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()