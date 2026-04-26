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
#define INT16_MAX 32767
#define INT16_MIN (-INT16_MAX - 1)
#endif

BEGIN_EXTERN_C()
#include "Smallint_arginfo.h"

zend_class_entry *php_driver_smallint_ce = NULL;

static zend_result to_double(zval *result, php_driver_numeric *smallint )
{
    ZVAL_DOUBLE(result, (double)smallint->data.smallint.value);
    return SUCCESS;
}

static zend_result to_long(zval *result, php_driver_numeric *smallint )
{
    ZVAL_LONG(result, (zend_long)smallint->data.smallint.value);
    return SUCCESS;
}

static zend_result to_string(zval *result, php_driver_numeric *smallint )
{
    char *string;
    spprintf(&string, 0, "%d", smallint->data.smallint.value);
    ZVAL_STRING(result, string);
    efree(string);
    return SUCCESS;
}

void php_driver_smallint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    php_driver_numeric *self;
    zval *value;
    cass_int32_t number;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_smallint_ce))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_driver_smallint_ce);
        self = PHP_DRIVER_GET_NUMERIC(return_value);
    }

    if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_driver_smallint_ce ))
    {
        php_driver_numeric *other = PHP_DRIVER_GET_NUMERIC(value);
        self->data.smallint.value = other->data.smallint.value;
    }
    else
    {
        if (Z_TYPE_P(value) == IS_LONG)
        {
            number = (cass_int32_t)Z_LVAL_P(value);

            if (number < INT16_MIN || number > INT16_MAX)
            {
                zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                        "value must be between -32768 and 32767, %ld given", Z_LVAL_P(value));
                return;
            }
        }
        else if (Z_TYPE_P(value) == IS_DOUBLE)
        {
            number = (cass_int32_t)Z_DVAL_P(value);

            if (number < INT16_MIN || number > INT16_MAX)
            {
                zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                        "value must be between -32768 and 32767, %g given", Z_DVAL_P(value));
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
                // be too large for Smallint. Reset the exception in that case.

                if (errno == ERANGE)
                {
                    zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                            "value must be between -32768 and 32767, %s given", Z_STRVAL_P(value));
                }
                return;
            }

            if (number < INT16_MIN || number > INT16_MAX)
            {
                zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                        "value must be between -32768 and 32767, %s given", Z_STRVAL_P(value));
                return;
            }
        }
        else
        {
            INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_DRIVER_NAMESPACE "\\Smallint");
        }
        self->data.smallint.value = (cass_int16_t)number;
    }
}

/* {{{ Smallint::__construct(string) */
ZEND_METHOD(Cassandra_Smallint, __construct)
{
    php_driver_smallint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Smallint::__toString() */
ZEND_METHOD(Cassandra_Smallint, __toString)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self );
}
/* }}} */

/* {{{ Smallint::type() */
ZEND_METHOD(Cassandra_Smallint, type)
{
    zval type = php_driver_type_scalar(CASS_VALUE_TYPE_SMALL_INT );
    RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Smallint::value() */
ZEND_METHOD(Cassandra_Smallint, value)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Smallint::add() */
ZEND_METHOD(Cassandra_Smallint, add)
{
    zval *addend;
    php_driver_numeric *self;
    php_driver_numeric *smallint;
    php_driver_numeric *result;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(addend)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(addend) == IS_OBJECT && instanceof_function(Z_OBJCE_P(addend), php_driver_smallint_ce ))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        smallint = PHP_DRIVER_GET_NUMERIC(addend);

        object_init_ex(return_value, php_driver_smallint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.smallint.value = self->data.smallint.value + smallint->data.smallint.value;
        if (result->data.smallint.value - smallint->data.smallint.value != self->data.smallint.value)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Sum is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(addend, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
    }
}
/* }}} */

/* {{{ Smallint::sub() */
ZEND_METHOD(Cassandra_Smallint, sub)
{
    zval *difference;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(difference)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(difference) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(difference), php_driver_smallint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *smallint = PHP_DRIVER_GET_NUMERIC(difference);

        object_init_ex(return_value, php_driver_smallint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.smallint.value = self->data.smallint.value - smallint->data.smallint.value;
        if (result->data.smallint.value + smallint->data.smallint.value != self->data.smallint.value)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Difference is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(difference, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
    }
}
/* }}} */

/* {{{ Smallint::mul() */
ZEND_METHOD(Cassandra_Smallint, mul)
{
    zval *multiplier;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(multiplier)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(multiplier) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(multiplier), php_driver_smallint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *smallint = PHP_DRIVER_GET_NUMERIC(multiplier);

        object_init_ex(return_value, php_driver_smallint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        result->data.smallint.value = self->data.smallint.value * smallint->data.smallint.value;
        if (smallint->data.smallint.value != 0 &&
            result->data.smallint.value / smallint->data.smallint.value != self->data.smallint.value)
        {
            zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Product is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(multiplier, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
    }
}
/* }}} */

/* {{{ Smallint::div() */
ZEND_METHOD(Cassandra_Smallint, div)
{
    zval *divisor;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(divisor)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), php_driver_smallint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *smallint = PHP_DRIVER_GET_NUMERIC(divisor);

        object_init_ex(return_value, php_driver_smallint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (smallint->data.smallint.value == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
            return;
        }

        result->data.smallint.value = self->data.smallint.value / smallint->data.smallint.value;
    }
    else
    {
        INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
    }
}
/* }}} */

/* {{{ Smallint::mod() */
ZEND_METHOD(Cassandra_Smallint, mod)
{
    zval *divisor;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(divisor)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), php_driver_smallint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *smallint = PHP_DRIVER_GET_NUMERIC(divisor);

        object_init_ex(return_value, php_driver_smallint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (smallint->data.smallint.value == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot modulo by zero");
            return;
        }

        result->data.smallint.value = self->data.smallint.value % smallint->data.smallint.value;
    }
    else
    {
        INVALID_ARGUMENT(divisor, "a " PHP_DRIVER_NAMESPACE "\\Smallint");
    }
}
/* }}} */

/* {{{ Smallint::abs() */
ZEND_METHOD(Cassandra_Smallint, abs)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.smallint.value == INT16_MIN)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value doesn't exist");
        return;
    }

    object_init_ex(return_value, php_driver_smallint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.smallint.value =
        self->data.smallint.value < 0 ? -self->data.smallint.value : self->data.smallint.value;
}
/* }}} */

/* {{{ Smallint::neg() */
ZEND_METHOD(Cassandra_Smallint, neg)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.smallint.value == INT16_MIN)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value doesn't exist");
        return;
    }

    object_init_ex(return_value, php_driver_smallint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.smallint.value = -self->data.smallint.value;
}
/* }}} */

/* {{{ Smallint::sqrt() */
ZEND_METHOD(Cassandra_Smallint, sqrt)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (self->data.smallint.value < 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                "Cannot take a square root of a negative number");
    }

    object_init_ex(return_value, php_driver_smallint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);
    result->data.smallint.value = (cass_int16_t)sqrt((long double)self->data.smallint.value);
}
/* }}} */

/* {{{ Smallint::toInt() */
ZEND_METHOD(Cassandra_Smallint, toInt)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Smallint::toDouble() */
ZEND_METHOD(Cassandra_Smallint, toDouble)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self );
}
/* }}} */

/* {{{ Smallint::min() */
ZEND_METHOD(Cassandra_Smallint, min)
{
    php_driver_numeric *smallint = NULL;
    object_init_ex(return_value, php_driver_smallint_ce);
    smallint = PHP_DRIVER_GET_NUMERIC(return_value);
    smallint->data.smallint.value = INT16_MIN;
}
/* }}} */

/* {{{ Smallint::max() */
ZEND_METHOD(Cassandra_Smallint, max)
{
    php_driver_numeric *smallint = NULL;
    object_init_ex(return_value, php_driver_smallint_ce);
    smallint = PHP_DRIVER_GET_NUMERIC(return_value);
    smallint->data.smallint.value = INT16_MAX;
}
/* }}} */


static php_driver_value_handlers php_driver_smallint_handlers;

static HashTable *php_driver_smallint_gc(
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

static HashTable *php_driver_smallint_properties(
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

    type = php_driver_type_scalar(CASS_VALUE_TYPE_SMALL_INT );
    (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);


    to_string(&value, self );
    (void)zend_hash_str_update(props, ZEND_STRL("value"), &value);

    return props;
}

static int php_driver_smallint_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
    php_driver_numeric *smallint1 = NULL;
    php_driver_numeric *smallint2 = NULL;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return 1; /* different classes */

    smallint1 = PHP_DRIVER_GET_NUMERIC(obj1);
    smallint2 = PHP_DRIVER_GET_NUMERIC(obj2);

    if (smallint1->data.smallint.value == smallint2->data.smallint.value)
        return 0;
    else if (smallint1->data.smallint.value < smallint2->data.smallint.value)
        return -1;
    else
        return 1;
}

static unsigned php_driver_smallint_hash_value(zval *obj )
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(obj);
    return 31 * 17 + self->data.smallint.value;
}

static
#if PHP_VERSION_ID >= 80200
    zend_result
#else
    int
#endif
    php_driver_smallint_cast(zend_object *object, zval *retval, int type )
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

static void php_driver_smallint_free(zend_object *object )
{
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);

    zend_object_std_dtor(&self->zendObject);

}

static zend_object* php_driver_smallint_new(zend_class_entry *ce )
{
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_ECALLOC(numeric, ce);

    self->type = PHP_DRIVER_SMALLINT;

    PHP5TO7_ZEND_OBJECT_INIT_EX(numeric, smallint, self, ce);
}

void php_driver_define_Smallint()
{
    php_driver_smallint_ce = register_class_Cassandra_Smallint(php_driver_value_ce, php_driver_numeric_ce);
    php_driver_smallint_ce->create_object = php_driver_smallint_new;

    memcpy(&php_driver_smallint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_smallint_handlers.std.get_properties = php_driver_smallint_properties;
    php_driver_smallint_handlers.std.get_gc = php_driver_smallint_gc;
    php_driver_smallint_handlers.std.compare = php_driver_smallint_compare;
    php_driver_smallint_handlers.std.cast_object = php_driver_smallint_cast;
    php_driver_smallint_handlers.hash_value = php_driver_smallint_hash_value;
}
END_EXTERN_C()