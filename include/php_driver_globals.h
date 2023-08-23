#ifndef PHP_DRIVER_GLOBALS_H
#define PHP_DRIVER_GLOBALS_H

#include <php.h>
#include <stdint.h>

BEGIN_EXTERN_C()

// clang-format off
ZEND_BEGIN_MODULE_GLOBALS(php_driver)
  CassUuidGen *uuid_gen;
  pid_t uuid_gen_pid;
  uint64_t persistent_clusters;
  uint64_t persistent_sessions;
  uint64_t persistent_prepared_statements;
ZEND_END_MODULE_GLOBALS(php_driver)
// clang-format on

ZEND_EXTERN_MODULE_GLOBALS(php_driver)
END_EXTERN_C()
#endif /* PHP_DRIVER_GLOBALS_H */