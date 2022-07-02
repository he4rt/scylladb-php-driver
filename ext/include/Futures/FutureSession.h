#include <CassandraDriver.h>
#include <php.h>

#include <cassandra.h>

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTURESESSION_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_FUTURES_FUTURESESSION_H_

typedef struct php_driver_future_session_ {
  CassFuture* future;
  php_driver_ref* session;
  zval default_session;
  cass_bool_t persist;
  char* hash_key;
  int hash_key_len;
  char* exception_message;
  CassError exception_code;
  char* session_keyspace;
  char* session_hash_key;
  zend_object zval;
} php_driver_future_session;

extern PHP_DRIVER_API zend_class_entry* php_driver_future_session_ce;

#define PHP_DRIVER_FUTURE_SESSION_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_future_session, obj)
#define PHP_DRIVER_FUTURE_SESSION_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_future_session, obj)
#define PHP_DRIVER_FUTURE_SESSION_THIS() PHP_DRIVER_THIS(php_driver_future_session)


#endif
