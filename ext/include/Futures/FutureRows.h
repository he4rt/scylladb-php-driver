#include <cassandra_driver.h>
#include <php.h>

#include <cassandra.h>

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTUREROWS_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTUREROWS_H_

typedef struct php_driver_future_rows_ {
  php_driver_ref* statement;
  php_driver_ref* session;
  zval rows;
  php_driver_ref* result;
  CassFuture* future;
  zend_object zval;
} php_driver_future_rows;

extern PHP_DRIVER_API zend_class_entry* php_driver_future_rows_ce;

#define PHP_DRIVER_FUTURE_ROWS_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_future_rows, obj)
#define PHP_DRIVER_FUTURE_ROWS_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_future_rows, obj)
#define PHP_DRIVER_FUTURE_ROWS_THIS() PHP_DRIVER_THIS(php_driver_future_rows)

#endif
