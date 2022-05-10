#include <cassandra_driver.h>
#include <php.h>

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTUREVALUE_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTUREVALUE_H_

typedef struct php_driver_future_value_ {
  zval value;
  zend_object zval;
} php_driver_future_value;

extern PHP_DRIVER_API zend_class_entry* php_driver_future_value_ce;


#define PHP_DRIVER_FUTURE_VALUE_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_future_value, obj)
#define PHP_DRIVER_FUTURE_VALUE_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_future_value, obj)
#define PHP_DRIVER_FUTURE_VALUE_THIS() PHP_DRIVER_THIS(php_driver_future_value)

#endif
