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

#include "php_driver.h"
#include "php_driver_types.h"
#include "util/bytes.h"
#include "util/types.h"
BEGIN_EXTERN_C()

#include "Blob_arginfo.h"

zend_class_entry *php_driver_blob_ce = NULL;

/* Legacy init helper used by util/src/types.cpp php_driver_scalar_init() */
void php_driver_blob_init(INTERNAL_FUNCTION_PARAMETERS) {
  char *string;
  size_t string_len;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(string, string_len)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_driver_blob_ce)) {
    php_driver_blob *self = PHP_DRIVER_GET_BLOB(ZEND_THIS);
    self->data = static_cast<cass_byte_t *>(emalloc(string_len * sizeof(cass_byte_t)));
    self->size = string_len;
    memcpy(self->data, string, string_len);
  } else {
    object_init_ex(return_value, php_driver_blob_ce);
    php_driver_blob *self = PHP_DRIVER_GET_BLOB(return_value);
    self->data = static_cast<cass_byte_t *>(emalloc(string_len * sizeof(cass_byte_t)));
    self->size = string_len;
    memcpy(self->data, string, string_len);
  }
}

/* {{{ Blob::__construct(string) */
ZEND_METHOD(Cassandra_Blob, __construct) {
  zend_string *bytes = nullptr;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(bytes)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_driver_blob *self = PHP_DRIVER_GET_BLOB(ZEND_THIS);

  self->data = static_cast<cass_byte_t *>(emalloc(ZSTR_LEN(bytes) * sizeof(cass_byte_t)));
  self->size = ZSTR_LEN(bytes);
  memcpy(self->data, ZSTR_VAL(bytes), ZSTR_LEN(bytes));
}
/* }}} */

/* {{{ Blob::__toString() */
ZEND_METHOD(Cassandra_Blob, __toString) {
  php_driver_blob *self = PHP_DRIVER_GET_BLOB(ZEND_THIS);
  char *hex;
  size_t hex_len;
  php_driver_bytes_to_hex((const char *)self->data, self->size, &hex, &hex_len);

  RETVAL_STRINGL(hex, hex_len);
  efree(hex);
}
/* }}} */

/* {{{ Blob::type() */
ZEND_METHOD(Cassandra_Blob, type) {
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_BLOB);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Blob::bytes() */
ZEND_METHOD(Cassandra_Blob, bytes) {
  php_driver_blob *self = PHP_DRIVER_GET_BLOB(ZEND_THIS);
  char *hex;
  size_t hex_len;
  php_driver_bytes_to_hex((const char *)self->data, self->size, &hex, &hex_len);

  RETVAL_STRINGL(hex, hex_len);
  efree(hex);
}
/* }}} */

/* {{{ Blob::toBinaryString() */
ZEND_METHOD(Cassandra_Blob, toBinaryString) {
  php_driver_blob *blob = PHP_DRIVER_GET_BLOB(ZEND_THIS);

  RETVAL_STRINGL((const char *)blob->data, blob->size);
}
/* }}} */

static php_driver_value_handlers php_driver_blob_handlers;

static HashTable *php_driver_blob_gc(zend_object *object, zval** table, int *n) {
  *table = NULL;
  *n = 0;
  return NULL;
}

static HashTable *php_driver_blob_properties(zend_object *object) {
  char *hex;
  size_t hex_len;
  zval type;
  zval bytes;

  php_driver_blob *self = PHP5TO7_ZEND_OBJECT_GET(blob, object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  type = php_driver_type_scalar(CASS_VALUE_TYPE_BLOB);
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);

  php_driver_bytes_to_hex((const char *)self->data, self->size, &hex, &hex_len);

  ZVAL_STRINGL(&bytes, hex, hex_len);
  efree(hex);
  (void)zend_hash_str_update(props, ZEND_STRL("bytes"), &bytes);

  return props;
}

static int php_driver_blob_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_driver_blob *blob1 = NULL;
  php_driver_blob *blob2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return 1; /* different classes */

  blob1 = PHP_DRIVER_GET_BLOB(obj1);
  blob2 = PHP_DRIVER_GET_BLOB(obj2);

  if (blob1->size == blob2->size) {
    return memcmp((const char *)blob1->data, (const char *)blob2->data,
                  blob1->size);
  } else if (blob1->size < blob2->size) {
    return -1;
  } else {
    return 1;
  }
}

static unsigned php_driver_blob_hash_value(zval *obj) {
  php_driver_blob *self = PHP_DRIVER_GET_BLOB(obj);
  return zend_inline_hash_func((const char *)self->data, self->size);
}

static void php_driver_blob_free(zend_object *object) {
  php_driver_blob *self = PHP5TO7_ZEND_OBJECT_GET(blob, object);

  if (self->data) {
    efree(self->data);
  }

  zend_object_std_dtor(&self->zendObject);
}

static zend_object* php_driver_blob_new(zend_class_entry *ce) {
  php_driver_blob *self = PHP5TO7_ZEND_OBJECT_ECALLOC(blob, ce);

  PHP5TO7_ZEND_OBJECT_INIT(blob, self, ce);
}

void php_driver_define_Blob() {
  php_driver_blob_ce = register_class_Cassandra_Blob(php_driver_value_ce);
  php_driver_blob_ce->create_object = php_driver_blob_new;

  memcpy(&php_driver_blob_handlers, zend_get_std_object_handlers(),
         sizeof(zend_object_handlers));
  php_driver_blob_handlers.std.get_properties = php_driver_blob_properties;
  php_driver_blob_handlers.std.get_gc = php_driver_blob_gc;
  php_driver_blob_handlers.std.compare = php_driver_blob_compare;
  php_driver_blob_handlers.hash_value = php_driver_blob_hash_value;
  php_driver_blob_handlers.std.clone_obj = NULL;
}
END_EXTERN_C()
