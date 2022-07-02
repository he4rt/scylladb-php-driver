#include <CassandraDriver.h>
#include <php.h>

#include <cassandra.h>

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTUREPREPAREDSTATEMENT_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTUREPREPAREDSTATEMENT_H_

typedef struct php_driver_future_prepared_statement_ {
  CassFuture* future;
  zval prepared_statement;
  zend_object zval;
} php_driver_future_prepared_statement;

extern PHP_DRIVER_API zend_class_entry* php_driver_future_prepared_statement_ce;

#define PHP_DRIVER_FUTURE_PREPARED_STATEMENT_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_future_prepared_statement, obj)
#define PHP_DRIVER_FUTURE_PREPARED_STATEMENT_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_future_prepared_statement, obj)
#define PHP_DRIVER_FUTURE_PREPARED_STATEMENT_THIS() PHP_DRIVER_THIS(php_driver_future_prepared_statement)

#endif
