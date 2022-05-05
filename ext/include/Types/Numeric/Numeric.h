#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_NUMERIC_NUMERIC_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_NUMERIC_NUMERIC_H_

// FIXME: Should be removed
#include "php_driver.h"

#include <cassandra.h>
#include <gmp.h>
#include <php.h>

#include <cassandra_driver.h>

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

#define PHP_DRIVER_NUMERIC_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_numeric, obj)
#define PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_numeric, obj)
#define PHP_DRIVER_NUMERIC_THIS() PHP_DRIVER_THIS(php_driver_numeric)

extern PHP_DRIVER_API zend_class_entry* php_driver_numeric_ce;

extern PHP_DRIVER_API zend_class_entry* php_driver_bigint_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_decimal_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_float_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_smallint_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_tinyint_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_varint_ce;

#endif
