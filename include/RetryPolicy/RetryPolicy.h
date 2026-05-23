#pragma once

#include <api.h>
#include <cassandra.h>
#include <php.h>

#include <php_scylladb_object.h>

BEGIN_EXTERN_C()

typedef struct
{
  CassRetryPolicy *policy;
  zend_object zendObject;
} php_scylladb_retry_policy;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_default_policy_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_downgrading_consistency_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_fallthrough_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_logging_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_retry_policy_ce;

[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_default_policy_instantiate(zval *dst);
[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_downgrading_consistency_instantiate(zval *dst);
[[nodiscard, gnu::nonnull(1)]] PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_fallthrough_instantiate(zval *dst);
[[nodiscard, gnu::nonnull(1, 2)]] PHP_SCYLLADB_API php_scylladb_retry_policy *php_scylladb_retry_policy_logging_instantiate(zval *dst, php_scylladb_retry_policy *retry_policy);

static zend_always_inline php_scylladb_retry_policy *php_scylladb_retry_policy_from_obj(zend_object *obj) {
  return PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, obj);
}

#define Z_SCYLLADB_RETRY_POLICY_P(zv) php_scylladb_retry_policy_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_RETRY_POLICY_DATE(zv) php_scylladb_retry_policy_from_obj(Z_OBJ((zv)))

END_EXTERN_C()