/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <SSLOptions/SSLOptions.h>
#include <utility>

#include <php_driver.h>
#include <php.h>
#include <php_driver_types.h>


BEGIN_EXTERN_C()
#include <ext/standard/php_filestat.h>

static zend_result file_get_contents(const zend_string *path, zend_string** out_val) {
  php_stream *stream =
      php_stream_open_wrapper(ZSTR_VAL(path), "rb", USE_PATH | REPORT_ERRORS, NULL);
  if (!stream) {
    zend_throw_exception_ex(php_driver_runtime_exception_ce, 0,
                            "The path '%s' doesn't exist or is not readable", ZSTR_VAL(path));
    return FAILURE;
  }

  zend_string *str = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, 0);
  php_stream_close(stream);

  if (!str) {
    zend_throw_exception_ex(php_driver_runtime_exception_ce, 0, "Failed to allocate enough memory");
    return FAILURE;
  }

  *out_val = str;

  return SUCCESS;
}

#include "Builder_arginfo.h"

zend_class_entry *php_scylladb_ssl_builder_ce = nullptr;

ZEND_METHOD(Cassandra_SSLOptions_Builder, build) {
  const php_scylladb_ssl *ssl = php_scylladb_ssl_instantiate(return_value);
  if (ssl == nullptr) {
    zend_throw_exception_ex(php_driver_runtime_exception_ce, 0,
                            "Failed to initialize Cassandra\\SSLOptions");
    return;
  }

  const auto *builder = Z_SCYLLADB_SSL_BUILDER_P(ZEND_THIS);

  cass_ssl_set_verify_flags(ssl->ssl, static_cast<int>(builder->flags));

  if (builder->trusted_certs) {
    for (size_t i = 0; i < builder->trusted_certs_cnt; i++) {
      zend_string* str;
      if (file_get_contents(builder->trusted_certs[i], &str) == FAILURE) {
        return;
      }

      const CassError rc = cass_ssl_add_trusted_cert_n(ssl->ssl, ZSTR_VAL(str), ZSTR_LEN(str));
      zend_string_release(str);

      if (rc != CASS_OK) {
        zend_throw_exception_ex(exception_class(rc), rc, "%s", cass_error_desc(rc));
        return;
      }
    }
  }

  if (builder->client_cert) {
    zend_string *str = nullptr;
    if (file_get_contents(builder->client_cert, &str) == FAILURE) {
      return;
    }

    const CassError rc = cass_ssl_set_cert_n(ssl->ssl, ZSTR_VAL(str), ZSTR_LEN(str));
    zend_string_release(str);
    if (rc != CASS_OK) {
      zend_throw_exception_ex(exception_class(rc), rc, "%s", cass_error_desc(rc));
      return;
    }
  }

  if (builder->private_key) {
    zend_string *str = nullptr;

    if (file_get_contents(builder->private_key, &str) == FAILURE) {
      return;
    }

    const CassError rc = cass_ssl_set_cert_n(ssl->ssl, ZSTR_VAL(str), ZSTR_LEN(str));
    zend_string_release(str);
    if (rc != CASS_OK) {
      zend_throw_exception_ex(exception_class(rc), rc, "%s", cass_error_desc(rc));
    }
  }
}

ZEND_METHOD(Cassandra_SSLOptions_Builder, withTrustedCerts) {
  const zval *args = nullptr;
  int argc = 0;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_VARIADIC('+', args, argc)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  auto **certs = static_cast<zend_string **>(ecalloc(argc, sizeof(zend_string *)));

  for (int i = 0; i < argc; i++) {
    const zval *path = &args[i];

    if (Z_TYPE_P(path) != IS_STRING) {
      throw_invalid_argument(path, "path", "a path to a trusted cert file");
      efree(certs);
      return;
    }

    zval readable;
    php_stat(Z_STR_P(path), FS_IS_R, &readable);

    if (Z_TYPE(readable) == IS_FALSE) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "The path '%s' doesn't exist or is not readable", Z_STRVAL_P(path));
      efree(certs);
      return;
    }

    certs[i] = zend_string_copy(Z_STR_P(path));
  }

  auto *builder = Z_SCYLLADB_SSL_BUILDER_P(ZEND_THIS);

  if (builder->trusted_certs) {
    for (size_t i = 0; i < builder->trusted_certs_cnt; i++) {
      zend_string_release(builder->trusted_certs[i]);
    }

    efree(builder->trusted_certs);
  }

  builder->trusted_certs_cnt = argc;
  builder->trusted_certs = certs;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_SSLOptions_Builder, withVerifyFlags) {
  zend_long flags;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(flags)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  Z_SCYLLADB_SSL_BUILDER_P(ZEND_THIS)->flags = flags;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_SSLOptions_Builder, withClientCert) {
  zend_string *client_cert = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(client_cert)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  zval readable;
  php_stat(client_cert, FS_IS_R, &readable);

  if (Z_TYPE(readable) == IS_FALSE) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "The path '%s' doesn't exist or is not readable",
                            ZSTR_VAL(client_cert));
    return;
  }

  auto *builder = Z_SCYLLADB_SSL_BUILDER_P(ZEND_THIS);

  if (builder->client_cert) {
    zend_string_release(builder->client_cert);
  }

  builder->client_cert = zend_string_copy(client_cert);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_SSLOptions_Builder, withPrivateKey) {
  zend_string *private_key = nullptr, *passphrase = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_STR(private_key)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR(passphrase)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  zval readable;
  php_stat(private_key, FS_IS_R, &readable);

  if (Z_TYPE(readable) == IS_FALSE) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "The path '%s' doesn't exist or is not readable",
                            ZSTR_VAL(private_key));
    return;
  }

  auto *builder = ZendCPP::ObjectFetch<php_scylladb_ssl_builder>(ZEND_THIS);

  if (builder->private_key) {
    zend_string_release(builder->private_key);
  }

  builder->private_key = zend_string_copy(private_key);

  if (builder->passphrase) {
    zend_string_release(builder->passphrase);
    builder->passphrase = nullptr;
  }

  if (passphrase) {
    builder->passphrase = zend_string_copy(passphrase);
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

static zend_object_handlers php_driver_ssl_builder_handlers;

static HashTable *php_driver_ssl_builder_properties(zend_object *object) {
  HashTable *props = zend_std_get_properties(object);

  return props;
}

static int php_driver_ssl_builder_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void php_driver_ssl_builder_free(zend_object *object) {
  const auto *self = ZendCPP::ObjectFetch<php_scylladb_ssl_builder>(object);

  if (self->trusted_certs) {
    for (size_t i = 0; i < self->trusted_certs_cnt; i++) {
      zend_string_release(self->trusted_certs[i]);
    }
    efree(self->trusted_certs);
  }

  if (self->client_cert) {
    zend_string_release(self->client_cert);
  }

  if (self->private_key) {
    zend_string_release(self->private_key);
  }

  if (self->passphrase) {
    zend_string_release(self->passphrase);
  }
}

static zend_object *php_driver_ssl_builder_new(zend_class_entry *ce) {
  auto *self = ZendCPP::Allocate<php_scylladb_ssl_builder>(ce, &php_driver_ssl_builder_handlers);
  return &self->zendObject;
}

void php_driver_define_SSLOptionsBuilder() {
  php_scylladb_ssl_builder_ce = register_class_Cassandra_SSLOptions_Builder();
  php_scylladb_ssl_builder_ce->create_object = php_driver_ssl_builder_new;

  ZendCPP::InitHandlers<php_scylladb_ssl_builder>(&php_driver_ssl_builder_handlers);
  php_driver_ssl_builder_handlers.get_properties = php_driver_ssl_builder_properties;
  php_driver_ssl_builder_handlers.compare = php_driver_ssl_builder_compare;
  php_driver_ssl_builder_handlers.free_obj = php_driver_ssl_builder_free;
}
END_EXTERN_C()