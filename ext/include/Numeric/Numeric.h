//
// Created by Dusan Malusev on 5/3/22.
//

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_NUMERIC_NUMERIC_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_NUMERIC_NUMERIC_H_

#include "php_driver.h"
#include <php.h>

extern PHP_DRIVER_API zend_class_entry* php_driver_numeric_ce;

#define PHP_DRIVER_GET_NUMERIC(obj) ((php_driver_numeric*) (((char*) (obj)) - XtOffsetOf(php_driver_numeric, zval)))
#define PHP_DRIVER_GET_NUMERIC_ZVAL(obj) PHP_DRIVER_GET_NUMERIC(Z_OBJ_P(obj))

#define PHP_DRIVER_GET_NUMERIC_THIS() PHP_DRIVER_GET_NUMERIC_ZVAL(ZEND_THIS)

typedef enum {
  PHP_DRIVER_BIGINT,
  PHP_DRIVER_DECIMAL,
  PHP_DRIVER_FLOAT,
  PHP_DRIVER_VARINT,
  PHP_DRIVER_SMALLINT,
  PHP_DRIVER_TINYINT
} php_driver_numeric_type;

typedef struct {
  php_driver_numeric_type type;
  union {
    struct
    {
      cass_int8_t value;
    } tinyint;
    struct
    {
      cass_int16_t value;
    } smallint;
    struct
    {
      cass_int64_t value;
    } bigint;
    struct
    {
      cass_float_t value;
    } floating;
    struct
    {
      mpz_t value;
    } varint;
    struct
    {
      mpz_t value;
      long scale;
    } decimal;
  } data;
  zend_object zval;
} php_driver_numeric;

#endif // LIBPHPCASSANDRA_EXT_INCLUDE_NUMERIC_NUMERIC_H_
