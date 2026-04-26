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

#if !defined(HAVE_STDINT_H) && !defined(_MSC_STDINT_H_)
#define INT8_MAX 127
#define INT8_MIN (-INT8_MAX - 1)
#endif
BEGIN_EXTERN_C()
#include "Tinyint_arginfo.h"
zend_class_entry *php_driver_tinyint_ce = NULL;

static zend_result to_double(zval *result, php_driver_numeric *tinyint )
{
    ZVAL_DOUBLE(result, (double)tinyint->data.tinyint.value);
    return SUCCESS;
}

static zend_result to_long(zval *result, php_driver_numeric *tinyint )
{
    ZVAL_LONG(result, (zend_long)tinyint->data.tinyint.value);
    return SUCCESS;
}

static zend_result to_string(zval *result, php_driver_numeric *tinyint )
{
    char *string;
    spprintf(&string, 0, "%d", tinyint->data.tinyint.value);
    ZVAL_STRING(result, string);
    efree(string);
    return SUCCESS;
}

void php_driver_tinyint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    php_driver_numeric *self;
    zval *value;
    cass_int32_t number;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_tinyint_ce))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_driver_tinyint_ce);
        self = PHP_DRIVER_GET_NUMERIC(return_value);
    }

    if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_driver_tinyint_ce ))
    {
        php_driver_numeric *other = PHP_DRIVER_GET_NUMERIC(value);
        self->data.tinyint.value = other->data.tinyint.value;
    }
    else
    {
        if (Z_TYPE_P(value) == IS_LONG)
        {
            number = (cass_int32_t)Z_LVAL_P(value);

            if (number < INT8_MIN || number > INT8_MAX)
            {
                zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                        "value must be between -128 and 127, %ld given", Z_LVAL_P(value));
                return;
            }
        }
        else if (Z_TYPE_P(value) == IS_DOUBLE)
        {
            number = (cass_int32_t)Z_DVAL_P(value);

            if (number < INT8_MIN || number > INT8_MAX)
            {
                zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                        "value must be between -128 and 127, %g given", Z_DVAL_P(value));
                return;
            }
        }
        else if (Z_TYPE_P(value) == IS_STRING)
        {
            if (!php_driver_parse_int(Z_STRVAL_P(value), Z_STRLEN_P(value), &number ))
            {
                // If the parsing function fails, it would have set an exception. If it's
                // a range error, the error message would be wrong because the parsing
                // function supports all 32-bit values, so the "valid" range it reports would
                // be too large for Tinyint. Reset the exception in that case.

                if (errno == ERANGE)
                {
                    zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                            "value must be between -128 and 127, %s given", Z_STRVAL_P(value));
                }
                return;
            }

            if (number < INT8_MIN || number > INT8_MAX)
            {
                zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                        "value must be between -128 and 127, %s given", Z_STRVAL_P(value));
                return;
            }
        }
        else
        {
            INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Tinyint");
        }
        self->data.tinyint.value = (cass_int8_t)number;
    }
}

/* {{{ Tinyint::__construct(string) */
ZEND_METHOD(Cassandra_Tinyint, __construct)
{
    php_driver_tinyint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Tinyint::__toString() */
ZEND_METHOD(Cassandra_Tinyint, __toString)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self );
}
/* }}} */

/* {{{ Tinyint::type() */
ZEND_METHOD(Cassandra_Tinyint, type)
{
    zval type = php_driver_type_scalar(CASS_VALUE_TYPE_TINY_INT );
    RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Tinyint::value() */
ZEND_METHOD(Cassandra_Tinyint, value)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Tinyint::add() */
ZEND_METHOD(Cassandra_Tinyint, add)
{
    zval *addend;
    php_driver_numeric *self;
    php_driver_numeric *tinyint;
    php_driver_numeric *result;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(addend)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(addend) == IS_OBJECT && instanceof_function(Z_OBJCE_P(addend), php_driver_tinyint_ce ))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        tinyint = PHP_DRIVER_GET_NUMERIC(addend);

        object_init_ex(return_value, php_driver_tinyint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.tinyint.value = self->data.tinyint.value + tinyint->data.tinyint.value;
        if (result->data.tinyint.value - tinyint->data.tinyint.value != self->data.tinyint.value)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Sum is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(addend, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::sub() */
ZEND_METHOD(Cassandra_Tinyint, sub)
{
    zval *difference;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(difference)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(difference) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(difference), php_driver_tinyint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *tinyint = PHP_DRIVER_GET_NUMERIC(difference);

        object_init_ex(return_value, php_driver_tinyint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.tinyint.value = self->data.tinyint.value - tinyint->data.tinyint.value;
        if (result->data.tinyint.value + tinyint->data.tinyint.value != self->data.tinyint.value)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Difference is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(difference, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::mul() */
ZEND_METHOD(Cassandra_Tinyint, mul)
{
    zval *multiplier;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(multiplier)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(multiplier) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(multiplier), php_driver_tinyint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *tinyint = PHP_DRIVER_GET_NUMERIC(multiplier);

        object_init_ex(return_value, php_driver_tinyint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.tinyint.value = self->data.tinyint.value * tinyint->data.tinyint.value;
        if (tinyint->data.tinyint.value != 0 &&
            result->data.tinyint.value / tinyint->data.tinyint.value != self->data.tinyint.value)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Product is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(multiplier, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::div() */
ZEND_METHOD(Cassandra_Tinyint, div)
{
    zval *divisor;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(divisor)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), php_driver_tinyint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *tinyint = PHP_DRIVER_GET_NUMERIC(divisor);

        object_init_ex(return_value, php_driver_tinyint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (tinyint->data.tinyint.value == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
            return;
        }

        result->data.tinyint.value = self->data.tinyint.value / tinyint->data.tinyint.value;
    }
    else
    {
        INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::mod() */
ZEND_METHOD(Cassandra_Tinyint, mod)
{
    zval *divisor;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(divisor)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), php_driver_tinyint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *tinyint = PHP_DRIVER_GET_NUMERIC(divisor);

        object_init_ex(return_value, php_driver_tinyint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (tinyint->data.tinyint.value == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot modulo by zero");
            return;
        }

        result->data.tinyint.value = self->data.tinyint.value % tinyint->data.tinyint.value;
    }
    else
    {
        INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::abs() */
ZEND_METHOD(Cassandra_Tinyint, abs)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.tinyint.value == INT8_MIN)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value doesn't exist");
        return;
    }

    object_init_ex(return_value, php_driver_tinyint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.tinyint.value = self->data.tinyint.value < 0 ? -self->data.tinyint.value : self->data.tinyint.value;
}
/* }}} */

/* {{{ Tinyint::neg() */
ZEND_METHOD(Cassandra_Tinyint, neg)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.tinyint.value == INT8_MIN)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value doesn't exist");
        return;
    }

    object_init_ex(return_value, php_driver_tinyint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.tinyint.value = -self->data.tinyint.value;
}
/* }}} */

/* {{{ Tinyint::sqrt() */
ZEND_METHOD(Cassandra_Tinyint, sqrt)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.tinyint.value < 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                "Cannot take a square root of a negative number");
        return;
    }

    object_init_ex(return_value, php_driver_tinyint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.tinyint.value = (cass_int8_t)sqrt((long double)self->data.tinyint.value);
}
/* }}} */

/* {{{ Tinyint::toInt() */
ZEND_METHOD(Cassandra_Tinyint, toInt)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Tinyint::toDouble() */
ZEND_METHOD(Cassandra_Tinyint, toDouble)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self );
}
/* }}} */

/* {{{ Tinyint::min() */
ZEND_METHOD(Cassandra_Tinyint, min)
{
    php_driver_numeric *tinyint = NULL;
    object_init_ex(return_value, php_driver_tinyint_ce);
    tinyint = PHP_DRIVER_GET_NUMERIC(return_value);
    tinyint->data.tinyint.value = INT8_MIN;
}
/* }}} */

/* {{{ Tinyint::max() */
ZEND_METHOD(Cassandra_Tinyint, max)
{
    php_driver_numeric *tinyint = NULL;
    object_init_ex(return_value, php_driver_tinyint_ce);
    tinyint = PHP_DRIVER_GET_NUMERIC(return_value);
    tinyint->data.tinyint.value = INT8_MAX;
}
/* }}} */


static php_driver_value_handlers php_driver_tinyint_handlers;

static HashTable *php_driver_tinyint_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object,
#else
    zendObject *object,
#endif
    zval** table, int *n )
{
    *table = NULL;
    *n = 0;
    return NULL;
}

static HashTable *php_driver_tinyint_properties(
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

    type = php_driver_type_scalar(CASS_VALUE_TYPE_TINY_INT );
    (void)zend_hash_str_update(props, "type", sizeof("type") - 1, &type);


    to_string(&value, self );
    (void)zend_hash_str_update(props, "value", sizeof("value") - 1, &value);

    return props;
}

static int php_driver_tinyint_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
    php_driver_numeric *tinyint1 = NULL;
    php_driver_numeric *tinyint2 = NULL;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return 1; /* different classes */

    tinyint1 = PHP_DRIVER_GET_NUMERIC(obj1);
    tinyint2 = PHP_DRIVER_GET_NUMERIC(obj2);

    if (tinyint1->data.tinyint.value == tinyint2->data.tinyint.value)
        return 0;
    else if (tinyint1->data.tinyint.value < tinyint2->data.tinyint.value)
        return -1;
    else
        return 1;
}

static unsigned php_driver_tinyint_hash_value(zval *obj )
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(obj);
    return 31 * 17 + self->data.tinyint.value;
}

static
#if PHP_VERSION_ID >= 80200
    zend_result
#else
    int
#endif
    php_driver_tinyint_cast(zend_object *object, zval *retval, int type )
{
#if PHP_MAJOR_VERSION >= 8
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);
#else
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(object);
#endif

    switch (type)
    {
    case IS_LONG:
        return to_long(retval, self );
    case IS_DOUBLE:
        return to_double(retval, self );
    case IS_STRING:
        return to_string(retval, self );
    default:
        return FAILURE;
    }

    return SUCCESS;
}

static void php_driver_tinyint_free(zend_object *object )
{
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);

    zend_object_std_dtor(&self->zendObject);

}

static zend_object* php_driver_tinyint_new(zend_class_entry *ce )
{
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_ECALLOC(numeric, ce);

    self->type = PHP_DRIVER_TINYINT;

    PHP5TO7_ZEND_OBJECT_INIT_EX(numeric, tinyint, self, ce);
}

void php_driver_define_Tinyint()
{
    php_driver_tinyint_ce = register_class_Cassandra_Tinyint(php_driver_value_ce, php_driver_numeric_ce);
    php_driver_tinyint_ce->create_object = php_driver_tinyint_new;

    memcpy(&php_driver_tinyint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_tinyint_handlers.std.get_properties = php_driver_tinyint_properties;
    php_driver_tinyint_handlers.std.get_gc = php_driver_tinyint_gc;
    php_driver_tinyint_handlers.std.compare = php_driver_tinyint_compare;
    php_driver_tinyint_handlers.std.cast_object = php_driver_tinyint_cast;
    php_driver_tinyint_handlers.hash_value = php_driver_tinyint_hash_value;
}
END_EXTERN_C()