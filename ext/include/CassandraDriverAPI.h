#pragma once

#if defined(__GNUC__) && __GNUC__ >= 4
#define PHP_DRIVER_API __attribute__((visibility("default")))
#else
#define PHP_DRIVER_API
#endif

#define make(type) (type*) emalloc(sizeof(type))


#define PHP_DRIVER_OBJECT(type, obj) ((type*) (((char*) (obj)) - XtOffsetOf(type, zval)))
#define PHP_DRIVER_ZVAL_TO_OBJECT(type, obj) PHP_DRIVER_OBJECT(type, Z_OBJ_P(obj))
#define PHP_DRIVER_THIS(type) PHP_DRIVER_ZVAL_TO_OBJECT(type, ZEND_THIS)
