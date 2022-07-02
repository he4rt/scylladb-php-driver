#pragma once

#include <CassandraDriverAPI.h>
#include <cassandra.h>
#include <zend_types.h>

typedef struct {
  CassTimestampGen* gen;
  zend_object zval;
} php_driver_timestamp_gen;

#define PHP_DRIVER_TIMESTAMP_GENERATOR_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_timestamp_gen, obj)
#define PHP_DRIVER_TIMESTAMP_GENERATOR_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_timestamp_gen, obj)
#define PHP_DRIVER_TIMESTAMP_GENERATOR_THIS() PHP_DRIVER_THIS(php_driver_timestamp_gen)

extern PHP_DRIVER_API zend_class_entry* phpDriverTimestampGeneratorInterfaceCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverTimestampGenServerSideCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverTimestampGenMonotonicCe;

void PhpDriverDefineTimestampGenerators();