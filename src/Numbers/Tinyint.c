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

#include "php_scylladb.h"
#include "php_scylladb_types.h"
#include "Type/ValueHash.h"
#include "Numbers/NumberParser.h"
#include "Type/TypeFactory.h"

#include <math.h>

#if !defined(HAVE_STDINT_H) && !defined(_MSC_STDINT_H_)
#define INT8_MAX 127
#define INT8_MIN (-INT8_MAX - 1)
#endif
#include "Tinyint_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_tinyint_handlers;

static zend_result to_double(zval *result, php_scylladb_numeric *tinyint )
{
    ZVAL_DOUBLE(result, (double)tinyint->data.tinyint.value);
    return SUCCESS;
}

static zend_result to_long(zval *result, php_scylladb_numeric *tinyint )
{
    ZVAL_LONG(result, (zend_long)tinyint->data.tinyint.value);
    return SUCCESS;
}

static zend_result to_string(zval *result, php_scylladb_numeric *tinyint )
{
    ZVAL_STR(result, zend_strpprintf(0, "%d", tinyint->data.tinyint.value));
    return SUCCESS;
}

void php_scylladb_tinyint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    php_scylladb_numeric *self;
    zval *value = nullptr;
    cass_int32_t number;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_scylladb_tinyint_ce))
    {
        self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_scylladb_tinyint_ce);
        self = PHP_SCYLLADB_GET_NUMERIC(return_value);
    }

    if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_scylladb_tinyint_ce ))
    {
        auto other = PHP_SCYLLADB_GET_NUMERIC(value);
        self->data.tinyint.value = other->data.tinyint.value;
    }
    else
    {
        if (Z_TYPE_P(value) == IS_LONG)
        {
            number = (cass_int32_t)Z_LVAL_P(value);

            if (number < INT8_MIN || number > INT8_MAX)
            {
                zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                                        "value must be between -128 and 127, %ld given", Z_LVAL_P(value));
                return;
            }
        }
        else if (Z_TYPE_P(value) == IS_DOUBLE)
        {
            number = (cass_int32_t)Z_DVAL_P(value);

            if (number < INT8_MIN || number > INT8_MAX)
            {
                zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                                        "value must be between -128 and 127, %g given", Z_DVAL_P(value));
                return;
            }
        }
        else if (Z_TYPE_P(value) == IS_STRING)
        {
            if (!php_scylladb_parse_int(Z_STR_P(value), &number ))
            {
                // parse_int reports a 32-bit overflow via errno == ERANGE without
                // throwing (its 32-bit bounds are too wide for Tinyint); emit the
                // narrower range error here. Any other failure already carries its
                // own exception.

                if (errno == ERANGE)
                {
                    zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                                            "value must be between -128 and 127, %s given", Z_STRVAL_P(value));
                }
                return;
            }

            if (number < INT8_MIN || number > INT8_MAX)
            {
                zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                                        "value must be between -128 and 127, %s given", Z_STRVAL_P(value));
                return;
            }
        }
        else
        {
            INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
        }
        self->data.tinyint.value = (cass_int8_t)number;
    }
}

/* {{{ Tinyint::__construct(string) */
ZEND_METHOD(Cassandra_Tinyint, __construct)
{
    php_scylladb_tinyint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Tinyint::__toString() */
ZEND_METHOD(Cassandra_Tinyint, __toString)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self );
}
/* }}} */

/* {{{ Tinyint::type() */
ZEND_METHOD(Cassandra_Tinyint, type)
{
    zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_TINY_INT );
    RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Tinyint::value() */
ZEND_METHOD(Cassandra_Tinyint, value)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Tinyint::add() */
ZEND_METHOD(Cassandra_Tinyint, add)
{
    zval *addend = nullptr;
    php_scylladb_numeric *self;
    php_scylladb_numeric *tinyint;
    php_scylladb_numeric *result;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(addend)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(addend) == IS_OBJECT && instanceof_function(Z_OBJCE_P(addend), php_scylladb_tinyint_ce ))
    {
        self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        tinyint = PHP_SCYLLADB_GET_NUMERIC(addend);

        object_init_ex(return_value, php_scylladb_tinyint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        result->data.tinyint.value = self->data.tinyint.value + tinyint->data.tinyint.value;
        if (result->data.tinyint.value - tinyint->data.tinyint.value != self->data.tinyint.value)
        {
            zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Sum is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(addend, "a " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::sub() */
ZEND_METHOD(Cassandra_Tinyint, sub)
{
    zval *difference = nullptr;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(difference)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(difference) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(difference), php_scylladb_tinyint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto tinyint = PHP_SCYLLADB_GET_NUMERIC(difference);

        object_init_ex(return_value, php_scylladb_tinyint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        result->data.tinyint.value = self->data.tinyint.value - tinyint->data.tinyint.value;
        if (result->data.tinyint.value + tinyint->data.tinyint.value != self->data.tinyint.value)
        {
            zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Difference is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(difference, "a " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::mul() */
ZEND_METHOD(Cassandra_Tinyint, mul)
{
    zval *multiplier = nullptr;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(multiplier)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(multiplier) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(multiplier), php_scylladb_tinyint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto tinyint = PHP_SCYLLADB_GET_NUMERIC(multiplier);

        object_init_ex(return_value, php_scylladb_tinyint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        result->data.tinyint.value = self->data.tinyint.value * tinyint->data.tinyint.value;
        if (tinyint->data.tinyint.value != 0 &&
            result->data.tinyint.value / tinyint->data.tinyint.value != self->data.tinyint.value)
        {
            zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Product is out of range");
            return;
        }
    }
    else
    {
        INVALID_ARGUMENT(multiplier, "a " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::div() */
ZEND_METHOD(Cassandra_Tinyint, div)
{
    zval *divisor = nullptr;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(divisor)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), php_scylladb_tinyint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto tinyint = PHP_SCYLLADB_GET_NUMERIC(divisor);

        object_init_ex(return_value, php_scylladb_tinyint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        if (tinyint->data.tinyint.value == 0)
        {
            zend_throw_exception_ex(php_scylladb_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
            return;
        }

        result->data.tinyint.value = self->data.tinyint.value / tinyint->data.tinyint.value;
    }
    else
    {
        INVALID_ARGUMENT(divisor, "a " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::mod() */
ZEND_METHOD(Cassandra_Tinyint, mod)
{
    zval *divisor = nullptr;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(divisor)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(divisor) == IS_OBJECT && instanceof_function(Z_OBJCE_P(divisor), php_scylladb_tinyint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto tinyint = PHP_SCYLLADB_GET_NUMERIC(divisor);

        object_init_ex(return_value, php_scylladb_tinyint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        if (tinyint->data.tinyint.value == 0)
        {
            zend_throw_exception_ex(php_scylladb_divide_by_zero_exception_ce, 0 , "Cannot modulo by zero");
            return;
        }

        result->data.tinyint.value = self->data.tinyint.value % tinyint->data.tinyint.value;
    }
    else
    {
        INVALID_ARGUMENT(divisor, "a " PHP_SCYLLADB_NAMESPACE "\\Tinyint");
    }
}
/* }}} */

/* {{{ Tinyint::abs() */
ZEND_METHOD(Cassandra_Tinyint, abs)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    if (self->data.tinyint.value == INT8_MIN)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value doesn't exist");
        RETURN_THROWS();
    }

    object_init_ex(return_value, php_scylladb_tinyint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);
    result->data.tinyint.value = self->data.tinyint.value < 0
                                     ? (cass_int8_t)(-self->data.tinyint.value)
                                     : self->data.tinyint.value;
}
/* }}} */

/* {{{ Tinyint::neg() */
ZEND_METHOD(Cassandra_Tinyint, neg)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    if (self->data.tinyint.value == INT8_MIN)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value doesn't exist");
        RETURN_THROWS();
    }

    object_init_ex(return_value, php_scylladb_tinyint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);
    result->data.tinyint.value = (cass_int8_t)(-self->data.tinyint.value);
}
/* }}} */

/* {{{ Tinyint::sqrt() */
ZEND_METHOD(Cassandra_Tinyint, sqrt)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    if (self->data.tinyint.value < 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                                "Cannot take a square root of a negative number");
        RETURN_THROWS();
    }

    object_init_ex(return_value, php_scylladb_tinyint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);
    const long double root = sqrtl((long double)self->data.tinyint.value);
    result->data.tinyint.value = (cass_int8_t)root;
}
/* }}} */

/* {{{ Tinyint::toInt() */
ZEND_METHOD(Cassandra_Tinyint, toInt)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Tinyint::toDouble() */
ZEND_METHOD(Cassandra_Tinyint, toDouble)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self );
}
/* }}} */

/* {{{ Tinyint::min() */
ZEND_METHOD(Cassandra_Tinyint, min)
{
    php_scylladb_numeric *tinyint = nullptr;
    object_init_ex(return_value, php_scylladb_tinyint_ce);
    tinyint = PHP_SCYLLADB_GET_NUMERIC(return_value);
    tinyint->data.tinyint.value = INT8_MIN;
}
/* }}} */

/* {{{ Tinyint::max() */
ZEND_METHOD(Cassandra_Tinyint, max)
{
    php_scylladb_numeric *tinyint = nullptr;
    object_init_ex(return_value, php_scylladb_tinyint_ce);
    tinyint = PHP_SCYLLADB_GET_NUMERIC(return_value);
    tinyint->data.tinyint.value = INT8_MAX;
}
/* }}} */



HashTable *php_scylladb_tinyint_gc(
    zend_object *object,
    zval** table, int *n )
{
    *table = nullptr;
    *n = 0;
    return nullptr;
}

HashTable *php_scylladb_tinyint_properties(
    zend_object *object
)
{
    zval type;
    zval value;

    auto self = php_scylladb_numeric_object_fetch(object);
    HashTable *props = php_scylladb_properties_rebuild(object, 2);

    type = php_scylladb_type_scalar(CASS_VALUE_TYPE_TINY_INT );
    (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);


    to_string(&value, self );
    (void)zend_hash_str_update(props, ZEND_STRL("value"), &value);

    return props;
}

int php_scylladb_tinyint_compare(zval *obj1, zval *obj2 )
{
    PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
    php_scylladb_numeric *tinyint1 = nullptr;
    php_scylladb_numeric *tinyint2 = nullptr;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    tinyint1 = PHP_SCYLLADB_GET_NUMERIC(obj1);
    tinyint2 = PHP_SCYLLADB_GET_NUMERIC(obj2);

    if (tinyint1->data.tinyint.value == tinyint2->data.tinyint.value)
        return 0;
    else if (tinyint1->data.tinyint.value < tinyint2->data.tinyint.value)
        return -1;
    else
        return 1;
}

unsigned php_scylladb_tinyint_hash_value(zval *obj )
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(obj);
    return (unsigned)(31 * 17 + self->data.tinyint.value);
}

zend_result php_scylladb_tinyint_cast(zend_object *object, zval *retval, int type )
{
    auto self = php_scylladb_numeric_object_fetch(object);

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
}

void php_scylladb_tinyint_free(zend_object *object )
{
    auto self = php_scylladb_numeric_object_fetch(object);

    zend_object_std_dtor(&self->zendObject);

}

zend_object* php_scylladb_tinyint_new(zend_class_entry *ce )
{
    php_scylladb_numeric *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_numeric, ce, &php_scylladb_tinyint_handlers);

    self->type = PHP_SCYLLADB_TINYINT;
    return &self->zendObject;
}


void php_scylladb_tinyint_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_tinyint_handlers.std.offset = offsetof(php_scylladb_numeric, zendObject);
}
