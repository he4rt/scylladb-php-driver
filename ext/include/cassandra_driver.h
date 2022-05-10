#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_CASSANDRA_DRIVER_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_CASSANDRA_DRIVER_H_

#define _POSIX_C_SOURCE 200809L

#include <Zend/zend_portability.h>

#define PHP_DRIVER_OBJECT(type, obj) ((type*) (((char*) (obj)) - XtOffsetOf(type, zval)))
#define PHP_DRIVER_ZVAL_TO_OBJECT(type, obj) PHP_DRIVER_OBJECT(type, Z_OBJ_P(obj))
#define PHP_DRIVER_THIS(type) PHP_DRIVER_ZVAL_TO_OBJECT(type, ZEND_THIS)

#if defined(__GNUC__) && __GNUC__ >= 4
#define PHP_DRIVER_API __attribute__((visibility("default")))
#else
#define PHP_DRIVER_API
#endif

#define make(type) (type*) emalloc(sizeof(type))

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
