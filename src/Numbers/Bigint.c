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

#include "Bigint_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_bigint_handlers;
#if !defined(HAVE_STDINT_H) && !defined(_MSC_STDINT_H_)
#define INT64_MAX 9223372036854775807LL
#define INT64_MIN (-INT64_MAX - 1)
#endif


static zend_result to_double(zval *result, php_scylladb_numeric *bigint)
{
    ZVAL_DOUBLE(result, (double)bigint->data.bigint.value);
    return SUCCESS;
}

static zend_result to_long(zval *result, php_scylladb_numeric *bigint)
{
    if (bigint->data.bigint.value < (cass_int64_t)INT64_MIN)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0, "Value is too small");
        return FAILURE;
    }

    if (bigint->data.bigint.value > (cass_int64_t)INT64_MAX)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0, "Value is too big");
        return FAILURE;
    }

    ZVAL_LONG(result, (zend_long)bigint->data.bigint.value);
    return SUCCESS;
}

static zend_result to_string(zval *result, php_scylladb_numeric *bigint)
{
    char *string;
    spprintf(&string, 0, "%" PRId64, bigint->data.bigint.value);
    ZVAL_STRING(result, string);
    efree(string);
    return SUCCESS;
}

void php_scylladb_bigint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    php_scylladb_numeric *self;
    zval *value;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_scylladb_bigint_ce))
    {
        self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_scylladb_bigint_ce);
        self = PHP_SCYLLADB_GET_NUMERIC(return_value);
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
            zend_throw_exception_ex(php_scylladb_range_exception_ce, 0,
                                    "value must be between %" PRId64 " and %" PRId64 ", %g given", INT64_MIN,
                                    INT64_MAX, double_value);
            return;
        }

        self->data.bigint.value = (cass_int64_t)Z_DVAL_P(value);
    }
    else if (Z_TYPE_P(value) == IS_STRING)
    {
        if (!php_scylladb_parse_bigint(Z_STRVAL_P(value), Z_STRLEN_P(value), &self->data.bigint.value))
        {
            return;
        }
    }
    else if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_scylladb_bigint_ce))
    {
        auto bigint = PHP_SCYLLADB_GET_NUMERIC(value);
        self->data.bigint.value = bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " PHP_SCYLLADB_NAMESPACE "\\Bigint");
    }
}

/* {{{ Bigint::__construct(string) */
ZEND_METHOD(Cassandra_Bigint, __construct)
{
    php_scylladb_bigint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Bigint::__toString() */
ZEND_METHOD(Cassandra_Bigint, __toString)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self);
}
/* }}} */

/* {{{ Bigint::type() */
ZEND_METHOD(Cassandra_Bigint, type)
{
    zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_BIGINT);
    RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Bigint::value() */
ZEND_METHOD(Cassandra_Bigint, value)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self);
}
/* }}} */

/* {{{ Bigint::add() */
ZEND_METHOD(Cassandra_Bigint, add)
{
    zval *num;
    php_scylladb_numeric *self;
    php_scylladb_numeric *bigint;
    php_scylladb_numeric *result;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_bigint_ce))
    {
        self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        bigint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_bigint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        result->data.bigint.value = self->data.bigint.value + bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_SCYLLADB_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::sub() */
ZEND_METHOD(Cassandra_Bigint, sub)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_bigint_ce))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto bigint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_bigint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        result->data.bigint.value = self->data.bigint.value - bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_SCYLLADB_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::mul() */
ZEND_METHOD(Cassandra_Bigint, mul)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_bigint_ce))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto bigint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_bigint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        result->data.bigint.value = self->data.bigint.value * bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_SCYLLADB_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::div() */
ZEND_METHOD(Cassandra_Bigint, div)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_bigint_ce))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto bigint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_bigint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        if (bigint->data.bigint.value == 0)
        {
            zend_throw_exception_ex(php_scylladb_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
            return;
        }

        result->data.bigint.value = self->data.bigint.value / bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_SCYLLADB_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::mod() */
ZEND_METHOD(Cassandra_Bigint, mod)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_bigint_ce))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto bigint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_bigint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        if (bigint->data.bigint.value == 0)
        {
            zend_throw_exception_ex(php_scylladb_divide_by_zero_exception_ce, 0, "Cannot modulo by zero");
            return;
        }

        result->data.bigint.value = self->data.bigint.value % bigint->data.bigint.value;
    }
    else
    {
        INVALID_ARGUMENT(num, "a " PHP_SCYLLADB_NAMESPACE "\\Bigint");
    }
}
/* }}} */

/* {{{ Bigint::abs() */
ZEND_METHOD(Cassandra_Bigint, abs)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    if (self->data.bigint.value == INT64_MIN)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0, "Value doesn't exist");
        return;
    }

    object_init_ex(return_value, php_scylladb_bigint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);
    result->data.bigint.value = self->data.bigint.value < 0 ? -self->data.bigint.value : self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::neg() */
ZEND_METHOD(Cassandra_Bigint, neg)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    object_init_ex(return_value, php_scylladb_bigint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);
    result->data.bigint.value = -self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::sqrt() */
ZEND_METHOD(Cassandra_Bigint, sqrt)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    if (self->data.bigint.value < 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0,
                                "Cannot take a square root of a negative number");
    }

    object_init_ex(return_value, php_scylladb_bigint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);
    result->data.bigint.value = (cass_int64_t)sqrt((long double)self->data.bigint.value);
}
/* }}} */

/* {{{ Bigint::toInt() */
ZEND_METHOD(Cassandra_Bigint, toInt)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self);
}
/* }}} */

/* {{{ Bigint::toDouble() */
ZEND_METHOD(Cassandra_Bigint, toDouble)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self);
}
/* }}} */

/* {{{ Bigint::min() */
ZEND_METHOD(Cassandra_Bigint, min)
{
    php_scylladb_numeric *bigint = nullptr;
    object_init_ex(return_value, php_scylladb_bigint_ce);
    bigint = PHP_SCYLLADB_GET_NUMERIC(return_value);
    bigint->data.bigint.value = INT64_MIN;
}
/* }}} */

/* {{{ Bigint::max() */
ZEND_METHOD(Cassandra_Bigint, max)
{
    php_scylladb_numeric *bigint = nullptr;
    object_init_ex(return_value, php_scylladb_bigint_ce);
    bigint = PHP_SCYLLADB_GET_NUMERIC(return_value);
    bigint->data.bigint.value = INT64_MAX;
}
/* }}} */


HashTable *php_scylladb_bigint_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object,
#else
    zendObject *object,
#endif
    zval** table, int *n)
{
    *table = nullptr;
    *n = 0;
    return nullptr;
}

HashTable *php_scylladb_bigint_properties(
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
    auto self = php_scylladb_numeric_object_fetch(object);
#else
    auto self = PHP_SCYLLADB_GET_NUMERIC(object);
#endif
    if (object->properties) {
        zend_array_release(object->properties);
    }
    object->properties = zend_new_array(2);
    HashTable *props = object->properties;

    type = php_scylladb_type_scalar(CASS_VALUE_TYPE_BIGINT);
    (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);


    to_string(&value, self);
    (void)zend_hash_str_update(props, ZEND_STRL("value"), &value);

    return props;
}

int php_scylladb_bigint_compare(zval *obj1, zval *obj2)
{
#if PHP_MAJOR_VERSION >= 8
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
    php_scylladb_numeric *bigint1 = nullptr;
    php_scylladb_numeric *bigint2 = nullptr;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    bigint1 = PHP_SCYLLADB_GET_NUMERIC(obj1);
    bigint2 = PHP_SCYLLADB_GET_NUMERIC(obj2);

    if (bigint1->data.bigint.value == bigint2->data.bigint.value)
        return 0;
    else if (bigint1->data.bigint.value < bigint2->data.bigint.value)
        return -1;
    else
        return 1;
}

unsigned php_scylladb_bigint_hash_value(zval *obj)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(obj);
    return (unsigned)(self->data.bigint.value ^ (self->data.bigint.value >> 32));
}

zend_result php_scylladb_bigint_cast(zend_object *object, zval *retval, int type)
{
#if PHP_MAJOR_VERSION >= 8
    auto self = php_scylladb_numeric_object_fetch(object);
#else
    auto self = PHP_SCYLLADB_GET_NUMERIC(object);
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

void php_scylladb_bigint_free(zend_object *object)
{
    auto self = php_scylladb_numeric_object_fetch(object);

    zend_object_std_dtor(&self->zendObject);

}

zend_object* php_scylladb_bigint_new(zend_class_entry *ce)
{
    php_scylladb_numeric *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_numeric, ce, &php_scylladb_bigint_handlers);

    self->type = PHP_SCYLLADB_BIGINT;
    return &self->zendObject;
}

void php_scylladb_bigint_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_bigint_handlers.std.offset = XtOffsetOf(php_scylladb_numeric, zendObject);
}
