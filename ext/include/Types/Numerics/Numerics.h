#pragma once

#include <cassandra.h>
#include <gmp.h>
#include <php.h>

#include <CassandraDriverAPI.h>

typedef enum {
  PHP_DRIVER_BIGINT,
  PHP_DRIVER_DECIMAL,
  PHP_DRIVER_FLOAT,
  PHP_DRIVER_VARINT,
  PHP_DRIVER_SMALLINT,
  PHP_DRIVER_TINYINT
} PhpDriverNumericType;

typedef struct {
  PhpDriverNumericType type;
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
} PhpDriverNumeric;

#define PHP_DRIVER_NUMERIC_OBJECT(obj) PHP_DRIVER_OBJECT(PhpDriverNumeric, obj)
#define PHP_DRIVER_NUMERIC_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(PhpDriverNumeric, obj)
#define PHP_DRIVER_NUMERIC_THIS() PHP_DRIVER_THIS(PhpDriverNumeric)

extern PHP_DRIVER_API zend_class_entry* phpDriverNumericInterfaceCe;

extern PHP_DRIVER_API zend_class_entry* phpDriverBigintCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverDecimalCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverFloatCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverSmallintCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverTinyintCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverVarintCe;

void PhpDriverDefineNumerics(zend_class_entry* valueInterface);
