#ifndef PHP_SCYLLADB_GLOBALS_H
#define PHP_SCYLLADB_GLOBALS_H

BEGIN_EXTERN_C()

/* Shared async reactor (see src/Async/Reactor.c); per-request, per-thread under
   ZTS. Forward-declared to avoid a hard include here. */
typedef struct php_scylladb_reactor_ php_scylladb_reactor;

ZEND_BEGIN_MODULE_GLOBALS(php_scylladb)
  php_scylladb_reactor *reactor;
  CassUuidGen  *uuid_gen;
  pid_t         uuid_gen_pid;
  unsigned int  persistent_clusters;
  unsigned int  persistent_sessions;
  unsigned int  persistent_prepared_statements;
  bool          expose_credentials;
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
ZEND_END_MODULE_GLOBALS(php_scylladb)

ZEND_EXTERN_MODULE_GLOBALS(php_scylladb)
END_EXTERN_C()

#endif /* PHP_SCYLLADB_GLOBALS_H */

