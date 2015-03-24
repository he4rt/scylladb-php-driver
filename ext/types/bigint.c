#include <php.h>
#include <zend_exceptions.h>
#include "../php_cassandra.h"
#include "bigint.h"

extern zend_class_entry* cassandra_ce_InvalidArgumentException;

zend_class_entry* cassandra_ce_Bigint = NULL;

static int
ctype_digit(const char* s, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (!isdigit(s[i]))
      return 0;
  }

  return 1;
}

/* {{{ Cassandra\Bigint::__construct(string) */
PHP_METHOD(CassandraBigint, __construct)
{
  char* value;
  int value_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &value, &value_len) == FAILURE) {
    return;
  }

  if (!ctype_digit(value, value_len)) {
    zend_throw_exception_ex(cassandra_ce_InvalidArgumentException, 0 TSRMLS_CC, "Invalid integer value \"%s\"", value);
    return;
  }

  cassandra_bigint* number = (cassandra_bigint*) zend_object_store_get_object(getThis() TSRMLS_CC);
  number->value = strtoll(value, NULL, 10);
}
/* }}} */

/* {{{ Cassandra\Bigint::__toString() */
PHP_METHOD(CassandraBigint, __toString)
{
  char* string;
  cassandra_bigint* number = (cassandra_bigint*) zend_object_store_get_object(getThis() TSRMLS_CC);
#ifdef WIN32
  spprintf(&string, 0, "%I64d", number->value);
#else
  spprintf(&string, 0, "%lld", number->value);
#endif
  RETURN_STRING(string, 0);
}
/* }}} */

/* {{{ Cassandra\Bigint::value() */
PHP_METHOD(CassandraBigint, value)
{
  char* string;
  cassandra_bigint* number = (cassandra_bigint*) zend_object_store_get_object(getThis() TSRMLS_CC);
#ifdef WIN32
  spprintf(&string, 0, "%I64d", number->value);
#else
  spprintf(&string, 0, "%lld", number->value);
#endif
  RETURN_STRING(string, 0);
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo___construct, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

static zend_function_entry CassandraBigint_methods[] = {
  PHP_ME(CassandraBigint, __construct, arginfo___construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
  PHP_ME(CassandraBigint, __toString, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(CassandraBigint, value, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static zend_object_handlers cassandra_bigint_handlers;

static HashTable*
php_cassandra_bigint_properties(zval *object TSRMLS_DC)
{
  cassandra_bigint* number = (cassandra_bigint*) zend_object_store_get_object(object TSRMLS_CC);
  HashTable*        props  = zend_std_get_properties(object TSRMLS_CC);

  zval* value;
  char* string;
  int string_len;

#ifdef WIN32
  string_len = spprintf(&string, 0, "%I64d", number->value);
#else
  string_len = spprintf(&string, 0, "%lld", number->value);
#endif

  MAKE_STD_ZVAL(value);
  ZVAL_STRINGL(value, string, string_len, 0);

  zend_hash_update(props, "value", sizeof("value"), &value, sizeof(zval), NULL);

  return props;
}

static void
php_cassandra_bigint_free(void *object TSRMLS_DC)
{
  cassandra_bigint* number = (cassandra_bigint*) object;

  zend_object_std_dtor(&number->zval TSRMLS_CC);

  efree(number);
}

static zend_object_value
php_cassandra_bigint_new(zend_class_entry* class_type TSRMLS_DC)
{
  zend_object_value retval;
  cassandra_bigint *number;

  number = (cassandra_bigint*) emalloc(sizeof(cassandra_bigint));
  memset(number, 0, sizeof(cassandra_bigint));

  zend_object_std_init(&number->zval, class_type TSRMLS_CC);
  object_properties_init(&number->zval, class_type TSRMLS_CC);

  retval.handle   = zend_objects_store_put(number, (zend_objects_store_dtor_t) zend_objects_destroy_object, php_cassandra_bigint_free, NULL TSRMLS_CC);
  retval.handlers = &cassandra_bigint_handlers;

  return retval;
}

void cassandra_define_CassandraBigint(TSRMLS_D)
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, "Cassandra\\Bigint", CassandraBigint_methods);
  cassandra_ce_Bigint = zend_register_internal_class(&ce TSRMLS_CC);
  memcpy(&cassandra_bigint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  cassandra_bigint_handlers.get_properties = php_cassandra_bigint_properties;
  cassandra_ce_Bigint->ce_flags |= ZEND_ACC_FINAL_CLASS;
  cassandra_ce_Bigint->create_object = php_cassandra_bigint_new;
}