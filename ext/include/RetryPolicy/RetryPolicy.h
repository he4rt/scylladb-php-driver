#include <CassandraDriver.h>

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_RETRYPOLICY_RETRYPOLICY_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_RETRYPOLICY_RETRYPOLICY_H_

#define PHP_DRIVER_RETRY_POLICY_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_retry_policy, obj)
#define PHP_DRIVER_RETRY_POLICY_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_retry_policy, obj)
#define PHP_DRIVER_RETRY_POLICY_THIS() PHP_DRIVER_THIS(php_driver_retry_policy)

typedef struct {
  CassRetryPolicy* policy;
  zend_object zval;
} php_driver_retry_policy;

extern PHP_DRIVER_API zend_class_entry* phpDriverRetryPolicyInterfaceCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverRetryPolicyDefaultCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverRetryPolicyFallthroughCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverRetryPolicyLoggingCe;

#endif
