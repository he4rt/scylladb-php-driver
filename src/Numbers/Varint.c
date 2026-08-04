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

#include <float.h>

#include "Varint_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_varint_handlers;

static zend_result to_double(zval *result, php_scylladb_numeric *varint )
{
    if (mpz_cmp_d(varint->data.varint.value, -DBL_MAX) < 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value is too small");
        return FAILURE;
    }

    if (mpz_cmp_d(varint->data.varint.value, DBL_MAX) > 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value is too big");
        return FAILURE;
    }

    ZVAL_DOUBLE(result, mpz_get_d(varint->data.varint.value));
    return SUCCESS;
}

static zend_result to_long(zval *result, php_scylladb_numeric *varint )
{
    if (mpz_cmp_si(varint->data.varint.value, LONG_MIN) < 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value is too small");
        return FAILURE;
    }

    if (mpz_cmp_si(varint->data.varint.value, LONG_MAX) > 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value is too big");
        return FAILURE;
    }

    ZVAL_LONG(result, mpz_get_si(varint->data.varint.value));
    return SUCCESS;
}

static zend_result to_string(zval *result, php_scylladb_numeric *varint )
{
    char *string;
    int string_len;
    php_scylladb_format_integer(varint->data.varint.value, &string, &string_len);

    ZVAL_STRINGL(result, string, string_len);
    efree(string);

    return SUCCESS;
}

void php_scylladb_varint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *num;
    php_scylladb_numeric *self;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_scylladb_varint_ce))
    {
        self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_scylladb_varint_ce);
        self = PHP_SCYLLADB_GET_NUMERIC(return_value);
    }

    if (Z_TYPE_P(num) == IS_LONG)
    {
        mpz_set_si(self->data.varint.value, Z_LVAL_P(num));
    }
    else if (Z_TYPE_P(num) == IS_DOUBLE)
    {
        mpz_set_d(self->data.varint.value, Z_DVAL_P(num));
    }
    else if (Z_TYPE_P(num) == IS_STRING)
    {
        php_scylladb_parse_varint(Z_STRVAL_P(num), Z_STRLEN_P(num), &self->data.varint.value );
    }
    else if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_varint_ce ))
    {
        auto varint = PHP_SCYLLADB_GET_NUMERIC(num);
        mpz_set(self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "a long, double, numeric string or a " PHP_SCYLLADB_NAMESPACE "\\Varint instance");
    }
}
/* {{{ Varint::__construct(string) */
ZEND_METHOD(Cassandra_Varint, __construct)
{
    php_scylladb_varint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Varint::__toString() */
ZEND_METHOD(Cassandra_Varint, __toString)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self );
}
/* }}} */

/* {{{ Varint::type() */
ZEND_METHOD(Cassandra_Varint, type)
{
    zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_VARINT );
    RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Varint::value() */
ZEND_METHOD(Cassandra_Varint, value)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    char *string;
    int string_len;
    php_scylladb_format_integer(self->data.varint.value, &string, &string_len);

    RETVAL_STRINGL(string, string_len);
    efree(string);
}
/* }}} */

/* {{{ Varint::add() */
ZEND_METHOD(Cassandra_Varint, add)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_varint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto varint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_varint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        mpz_add(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_SCYLLADB_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::sub() */
ZEND_METHOD(Cassandra_Varint, sub)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_varint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto varint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_varint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        mpz_sub(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_SCYLLADB_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::mul() */
ZEND_METHOD(Cassandra_Varint, mul)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_varint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto varint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_varint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        mpz_mul(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_SCYLLADB_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::div() */
ZEND_METHOD(Cassandra_Varint, div)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_varint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto varint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_varint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        if (mpz_sgn(varint->data.varint.value) == 0)
        {
            zend_throw_exception_ex(php_scylladb_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
            return;
        }

        mpz_div(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_SCYLLADB_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::mod() */
ZEND_METHOD(Cassandra_Varint, mod)
{
    zval *num;
    php_scylladb_numeric *result = nullptr;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_scylladb_varint_ce ))
    {
        auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);
        auto varint = PHP_SCYLLADB_GET_NUMERIC(num);

        object_init_ex(return_value, php_scylladb_varint_ce);
        result = PHP_SCYLLADB_GET_NUMERIC(return_value);

        if (mpz_sgn(varint->data.varint.value) == 0)
        {
            zend_throw_exception_ex(php_scylladb_divide_by_zero_exception_ce, 0 , "Cannot modulo by zero");
            return;
        }

        mpz_mod(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_SCYLLADB_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::abs() */
ZEND_METHOD(Cassandra_Varint, abs)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    object_init_ex(return_value, php_scylladb_varint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);

    mpz_abs(result->data.varint.value, self->data.varint.value);
}
/* }}} */

/* {{{ Varint::neg() */
ZEND_METHOD(Cassandra_Varint, neg)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    object_init_ex(return_value, php_scylladb_varint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);

    mpz_neg(result->data.varint.value, self->data.varint.value);
}
/* }}} */

/* {{{ Varint::sqrt() */
ZEND_METHOD(Cassandra_Varint, sqrt)
{
    php_scylladb_numeric *result = nullptr;
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    if (mpz_sgn(self->data.varint.value) < 0)
    {
        zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                                "Cannot take a square root of a negative number");
        return;
    }

    object_init_ex(return_value, php_scylladb_varint_ce);
    result = PHP_SCYLLADB_GET_NUMERIC(return_value);

    mpz_sqrt(result->data.varint.value, self->data.varint.value);
}
/* }}} */

/* {{{ Varint::toInt() */
ZEND_METHOD(Cassandra_Varint, toInt)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Varint::toDouble() */
ZEND_METHOD(Cassandra_Varint, toDouble)
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self );
}
/* }}} */



HashTable *php_scylladb_varint_gc(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object,
#else
    zendObject *object,
#endif
    zval** table, int *n )
{
    /* Holds no zvals (mpz_t only). Report no roots directly instead of going
     * through zend_std_get_gc, which would invoke the side-effecting
     * get_properties (it rebuilds object->properties on every call) and corrupt
     * refcounts across the collector's mark/scan passes. */
    *table = nullptr;
    *n = 0;
    return nullptr;
}

HashTable *php_scylladb_varint_properties(
#if PHP_MAJOR_VERSION >= 8
    zend_object *object
#else
    zendObject *object
#endif
)
{
    char *string;
    int string_len;
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

    php_scylladb_format_integer(self->data.varint.value, &string, &string_len);

    type = php_scylladb_type_scalar(CASS_VALUE_TYPE_VARINT );
    (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);


    ZVAL_STRINGL(&value, string, string_len);
    efree(string);
    (void)zend_hash_str_update(props, ZEND_STRL("value"), &value);

    return props;
}

int php_scylladb_varint_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
    php_scylladb_numeric *varint1 = nullptr;
    php_scylladb_numeric *varint2 = nullptr;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    varint1 = PHP_SCYLLADB_GET_NUMERIC(obj1);
    varint2 = PHP_SCYLLADB_GET_NUMERIC(obj2);

    return mpz_cmp(varint1->data.varint.value, varint2->data.varint.value);
}

unsigned php_scylladb_varint_hash_value(zval *obj )
{
    auto self = PHP_SCYLLADB_GET_NUMERIC(obj);
    return php_scylladb_mpz_hash(0, self->data.varint.value);
}

zend_result php_scylladb_varint_cast(zend_object *object, zval *retval, int type )
{
#if PHP_MAJOR_VERSION >= 8
    auto self = php_scylladb_numeric_object_fetch(object);
#else
    auto self = PHP_SCYLLADB_GET_NUMERIC(object);
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

void php_scylladb_varint_free(zend_object *object )
{
    auto self = php_scylladb_numeric_object_fetch(object);

    mpz_clear(self->data.varint.value);

    zend_object_std_dtor(&self->zendObject);

}

zend_object* php_scylladb_varint_new(zend_class_entry *ce )
{
    php_scylladb_numeric *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_numeric, ce, &php_scylladb_varint_handlers);

    mpz_init(self->data.varint.value);
    return &self->zendObject;
}


void php_scylladb_varint_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_varint_handlers.std.offset = offsetof(php_scylladb_numeric, zendObject);
}
