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

#include <float.h>
BEGIN_EXTERN_C()
#include "Varint_arginfo.h"
zend_class_entry *php_driver_varint_ce = NULL;

static zend_result to_double(zval *result, php_driver_numeric *varint )
{
    if (mpz_cmp_d(varint->data.varint.value, -DBL_MAX) < 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value is too small");
        return FAILURE;
    }

    if (mpz_cmp_d(varint->data.varint.value, DBL_MAX) > 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value is too big");
        return FAILURE;
    }

    ZVAL_DOUBLE(result, mpz_get_d(varint->data.varint.value));
    return SUCCESS;
}

static zend_result to_long(zval *result, php_driver_numeric *varint )
{
    if (mpz_cmp_si(varint->data.varint.value, LONG_MIN) < 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value is too small");
        return FAILURE;
    }

    if (mpz_cmp_si(varint->data.varint.value, LONG_MAX) > 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 , "Value is too big");
        return FAILURE;
    }

    ZVAL_LONG(result, mpz_get_si(varint->data.varint.value));
    return SUCCESS;
}

static zend_result to_string(zval *result, php_driver_numeric *varint )
{
    char *string;
    int string_len;
    php_driver_format_integer(varint->data.varint.value, &string, &string_len);

    ZVAL_STRINGL(result, string, string_len);
    efree(string);

    return SUCCESS;
}

void php_driver_varint_init(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *num;
    php_driver_numeric *self;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_varint_ce))
    {
        self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
    }
    else
    {
        object_init_ex(return_value, php_driver_varint_ce);
        self = PHP_DRIVER_GET_NUMERIC(return_value);
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
        php_driver_parse_varint(Z_STRVAL_P(num), Z_STRLEN_P(num), &self->data.varint.value );
    }
    else if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_varint_ce ))
    {
        php_driver_numeric *varint = PHP_DRIVER_GET_NUMERIC(num);
        mpz_set(self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "a long, double, numeric string or a " PHP_DRIVER_NAMESPACE "\\Varint instance");
    }
}
/* {{{ Varint::__construct(string) */
ZEND_METHOD(Cassandra_Varint, __construct)
{
    php_driver_varint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Varint::__toString() */
ZEND_METHOD(Cassandra_Varint, __toString)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_string(return_value, self );
}
/* }}} */

/* {{{ Varint::type() */
ZEND_METHOD(Cassandra_Varint, type)
{
    zval type = php_driver_type_scalar(CASS_VALUE_TYPE_VARINT );
    RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Varint::value() */
ZEND_METHOD(Cassandra_Varint, value)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    char *string;
    int string_len;
    php_driver_format_integer(self->data.varint.value, &string, &string_len);

    RETVAL_STRINGL(string, string_len);
    efree(string);
}
/* }}} */

/* {{{ Varint::add() */
ZEND_METHOD(Cassandra_Varint, add)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_varint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *varint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_varint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        mpz_add(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::sub() */
ZEND_METHOD(Cassandra_Varint, sub)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_varint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *varint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_varint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        mpz_sub(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::mul() */
ZEND_METHOD(Cassandra_Varint, mul)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_varint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *varint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_varint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        mpz_mul(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::div() */
ZEND_METHOD(Cassandra_Varint, div)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_varint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *varint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_varint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (mpz_sgn(varint->data.varint.value) == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot divide by zero");
            return;
        }

        mpz_div(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::mod() */
ZEND_METHOD(Cassandra_Varint, mod)
{
    zval *num;
    php_driver_numeric *result = NULL;

    // clang-format off
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(num)
    ZEND_PARSE_PARAMETERS_END();
    // clang-format on

    if (Z_TYPE_P(num) == IS_OBJECT && instanceof_function(Z_OBJCE_P(num), php_driver_varint_ce ))
    {
        php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);
        php_driver_numeric *varint = PHP_DRIVER_GET_NUMERIC(num);

        object_init_ex(return_value, php_driver_varint_ce);
        result = PHP_DRIVER_GET_NUMERIC(return_value);

        if (mpz_sgn(varint->data.varint.value) == 0)
        {
            zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0 , "Cannot modulo by zero");
            return;
        }

        mpz_mod(result->data.varint.value, self->data.varint.value, varint->data.varint.value);
    }
    else
    {
        INVALID_ARGUMENT(num, "an instance of " PHP_DRIVER_NAMESPACE "\\Varint");
    }
}
/* }}} */

/* {{{ Varint::abs() */
ZEND_METHOD(Cassandra_Varint, abs)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    object_init_ex(return_value, php_driver_varint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    mpz_abs(result->data.varint.value, self->data.varint.value);
}
/* }}} */

/* {{{ Varint::neg() */
ZEND_METHOD(Cassandra_Varint, neg)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    object_init_ex(return_value, php_driver_varint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    mpz_neg(result->data.varint.value, self->data.varint.value);
}
/* }}} */

/* {{{ Varint::sqrt() */
ZEND_METHOD(Cassandra_Varint, sqrt)
{
    php_driver_numeric *result = NULL;
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    if (mpz_sgn(self->data.varint.value) < 0)
    {
        zend_throw_exception_ex(php_driver_range_exception_ce, 0 ,
                                "Cannot take a square root of a negative number");
        return;
    }

    object_init_ex(return_value, php_driver_varint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    mpz_sqrt(result->data.varint.value, self->data.varint.value);
}
/* }}} */

/* {{{ Varint::toInt() */
ZEND_METHOD(Cassandra_Varint, toInt)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_long(return_value, self );
}
/* }}} */

/* {{{ Varint::toDouble() */
ZEND_METHOD(Cassandra_Varint, toDouble)
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(ZEND_THIS);

    to_double(return_value, self );
}
/* }}} */


static php_driver_value_handlers php_driver_varint_handlers;

static HashTable *php_driver_varint_gc(
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

static HashTable *php_driver_varint_properties(
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
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);
#else
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(object);
#endif
    if (object->properties) {
        zend_array_release(object->properties);
    }
    object->properties = zend_new_array(2);
    HashTable *props = object->properties;

    php_driver_format_integer(self->data.varint.value, &string, &string_len);

    type = php_driver_type_scalar(CASS_VALUE_TYPE_VARINT );
    (void)zend_hash_str_update(props, "type", sizeof("type") - 1, &type);


    ZVAL_STRINGL(&value, string, string_len);
    efree(string);
    (void)zend_hash_str_update(props, "value", sizeof("value") - 1, &value);

    return props;
}

static int php_driver_varint_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
    ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
    php_driver_numeric *varint1 = NULL;
    php_driver_numeric *varint2 = NULL;

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return 1; /* different classes */

    varint1 = PHP_DRIVER_GET_NUMERIC(obj1);
    varint2 = PHP_DRIVER_GET_NUMERIC(obj2);

    return mpz_cmp(varint1->data.varint.value, varint2->data.varint.value);
}

static unsigned php_driver_varint_hash_value(zval *obj )
{
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(obj);
    return php_driver_mpz_hash(0, self->data.varint.value);
}

static
#if PHP_VERSION_ID >= 80200
    zend_result
#else
    int
#endif
    php_driver_varint_cast(zend_object *object, zval *retval, int type )
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

static void php_driver_varint_free(zend_object *object )
{
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_GET(numeric, object);

    mpz_clear(self->data.varint.value);

    zend_object_std_dtor(&self->zendObject);

}

static zend_object* php_driver_varint_new(zend_class_entry *ce )
{
    php_driver_numeric *self = PHP5TO7_ZEND_OBJECT_ECALLOC(numeric, ce);

    mpz_init(self->data.varint.value);

    PHP5TO7_ZEND_OBJECT_INIT_EX(numeric, varint, self, ce);
}

void php_driver_define_Varint()
{
    php_driver_varint_ce = register_class_Cassandra_Varint(php_driver_value_ce, php_driver_numeric_ce);
    php_driver_varint_ce->create_object = php_driver_varint_new;

    memcpy(&php_driver_varint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_varint_handlers.std.get_properties = php_driver_varint_properties;
    php_driver_varint_handlers.std.get_gc = php_driver_varint_gc;
    php_driver_varint_handlers.std.compare = php_driver_varint_compare;
    php_driver_varint_handlers.std.cast_object = php_driver_varint_cast;
    php_driver_varint_handlers.hash_value = php_driver_varint_hash_value;
    php_driver_varint_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()