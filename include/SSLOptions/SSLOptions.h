#pragma once

#include <api.h>
#include <php.h>
#include <cassandra.h>

#include <ZendCPP/ZendCPP.hpp>

BEGIN_EXTERN_C()

typedef struct {
  int64_t flags;
  zend_string **trusted_certs;
  size_t trusted_certs_cnt;
  zend_string *client_cert;
  zend_string *private_key;
  zend_string *passphrase;
  zend_object zendObject;
} php_scylladb_ssl_builder;

typedef struct {
  CassSsl *ssl;
  zend_object zendObject;
} php_scylladb_ssl;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_ssl_builder_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_ssl_ce;

zend_always_inline php_scylladb_ssl_builder *php_scylladb_ssl_builder_from_obj(zend_object *obj) {
  return ZendCPP::ObjectFetch<php_scylladb_ssl_builder>(obj);
}

zend_always_inline php_scylladb_ssl *php_scylladb_ssl_from_obj(zend_object *obj) {
  return ZendCPP::ObjectFetch<php_scylladb_ssl>(obj);
}

#define Z_SCYLLADB_SSL_BUILDER_P(zv) php_scylladb_ssl_builder_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_SSL_BUILDER(zv) php_scylladb_ssl_builder_from_obj(Z_OBJ((zv)))

#define Z_SCYLLADB_SSL_P(zv) php_scylladb_ssl_from_obj(Z_OBJ_P((zv)))
#define Z_SCYLLADB_SSL(zv) php_scylladb_ssl_from_obj(Z_OBJ((zv)))

PHP_SCYLLADB_API php_scylladb_ssl *php_scylladb_ssl_instantiate(zval *object);

END_EXTERN_C();