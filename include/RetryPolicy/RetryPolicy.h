#pragma once

#include <api.h>
#include <cassandra.h>
#include <php.h>

#include <ZendCPP/ZendCPP.hpp>

BEGIN_EXTERN_C()

typedef struct
{
  CassRetryPolicy *policy;
  zend_object zendObject;
} php_scylladb_retry_policy;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_default_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_retry_policy_downgrading_consistency_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_retry_policy_fallthrough_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_retry_policy_logging_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_ce;

PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_default_instantiate(zval *dst);
PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_downgrading_consistency_instantiate(zval *dst);
PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_fallthrough_instantiate(zval *dst);
PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_logging_instantiate(zval *dst, php_scylladb_retry_policy *retry_policy);

zend_always_inline php_scylladb_retry_policy *php_scylladb_retry_policy_from_obj(zend_object *obj) {
  return ZendCPP::ObjectFetch<php_scylladb_retry_policy>(obj);
}

#define Z_SCYLLADB_RETRY_POLICY_P(zv) php_scylladb_retry_policy_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_RETRY_POLICY_DATE(zv) php_scylladb_retry_policy_from_obj(Z_OBJ((zv)))

END_EXTERN_C()