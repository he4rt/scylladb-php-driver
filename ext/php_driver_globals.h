#ifndef PHP_DRIVER_GLOBALS_H
#define PHP_DRIVER_GLOBALS_H

ZEND_BEGIN_MODULE_GLOBALS(php_driver)
  CassUuidGen  *uuid_gen;
  pid_t         uuid_gen_pid;
  unsigned int  persistent_clusters;
  unsigned int  persistent_sessions;
  unsigned int  persistent_prepared_statements;
  zval  type_varchar;
  zval  type_text;
  zval  type_blob;
  zval  type_ascii;
  zval  type_bigint;
  zval  type_counter;
  zval  type_int;
  zval  type_varint;
  zval  type_boolean;
  zval  type_decimal;
  zval  type_double;
  zval  type_float;
  zval  type_inet;
  zval  type_timestamp;
  zval  type_date;
  zval  type_time;
  zval  type_uuid;
  zval  type_timeuuid;
  zval  type_smallint;
  zval  type_tinyint;
  zval  type_duration;
  zend_resource stmt;
ZEND_END_MODULE_GLOBALS(php_driver)

ZEND_EXTERN_MODULE_GLOBALS(php_driver)

#endif /* PHP_DRIVER_GLOBALS_H */
