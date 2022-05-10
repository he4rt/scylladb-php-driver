#include <cassandra_driver.h>
#include <php.h>

#include <cassandra.h>

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTURECLOSE_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTURECLOSE_H_

typedef struct php_driver_future_close_ {
  CassFuture* future;
  zend_object zval;
} php_driver_future_close;

extern PHP_DRIVER_API zend_class_entry* php_driver_future_close_ce;

#define PHP_DRIVER_FUTURE_CLOSE_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_future_close, obj)
#define PHP_DRIVER_FUTURE_CLOSE_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_future_close, obj)
#define PHP_DRIVER_FUTURE_CLOSE_THIS() PHP_DRIVER_THIS(php_driver_future_close)

#endif
