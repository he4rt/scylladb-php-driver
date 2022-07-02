#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_CASSANDRADRIVER_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_CASSANDRADRIVER_H_

#define _POSIX_C_SOURCE 200809L

#include <Zend/zend_API.h>
#include <Zend/zend_portability.h>

#include "CassandraDriverAPI.h"

typedef void (*php_driver_free_function)(void* data);

typedef struct
{
  size_t count;
  php_driver_free_function destruct;
  void* data;
} php_driver_ref;

#define ZVAL_DESTROY(zv)    \
  do {                      \
    if (!Z_ISUNDEF(zv)) {   \
      zval_ptr_dtor(&(zv)); \
      ZVAL_UNDEF(&(zv));    \
    }                       \
  } while (0)

#endif
