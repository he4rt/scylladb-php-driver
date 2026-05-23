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

#include "php_scylladb.h"
#include "php_scylladb_types.h"
#include "Type/TypeFactory.h"

#include "Blob_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_blob_handlers;


static void php_scylladb_bytes_to_hex(const char *bin, size_t len, char **out, size_t *out_len) {
  static const char hex_str[] = "0123456789abcdef";
  const size_t size = len * 2 + 2;
  char *value = (char *)emalloc(size + 1);

  value[0] = '0';
  value[1] = 'x';
  value[size] = '\0';

  for (size_t i = 0; i < len; i++) {
    value[i * 2 + 2] = hex_str[(bin[i] >> 4) & 0x0F];
    value[i * 2 + 3] = hex_str[(bin[i]) & 0x0F];
  }

  *out = value;
  *out_len = size;
}

/* Legacy init helper used by src/Type/TypeFactory.cpp php_scylladb_scalar_init() */
void php_scylladb_blob_init(INTERNAL_FUNCTION_PARAMETERS) {
  char *string;
  size_t string_len;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(string, string_len)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_scylladb_blob_ce)) {
    auto self = PHP_SCYLLADB_GET_BLOB(ZEND_THIS);
    if (self->data) {
      efree(self->data);
    }
    self->data = (cass_byte_t *)emalloc(string_len * sizeof(cass_byte_t));
    self->size = string_len;
    memcpy(self->data, string, string_len);
  } else {
    object_init_ex(return_value, php_scylladb_blob_ce);
    auto self = PHP_SCYLLADB_GET_BLOB(return_value);
    self->data = (cass_byte_t *)emalloc(string_len * sizeof(cass_byte_t));
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

  auto self = PHP_SCYLLADB_GET_BLOB(ZEND_THIS);

  self->data = (cass_byte_t *)emalloc(ZSTR_LEN(bytes) * sizeof(cass_byte_t));
  self->size = ZSTR_LEN(bytes);
  memcpy(self->data, ZSTR_VAL(bytes), ZSTR_LEN(bytes));
}
/* }}} */

/* {{{ Blob::__toString() */
ZEND_METHOD(Cassandra_Blob, __toString) {
  auto self = PHP_SCYLLADB_GET_BLOB(ZEND_THIS);
  char *hex;
  size_t hex_len;
  php_scylladb_bytes_to_hex((const char *)self->data, self->size, &hex, &hex_len);

  RETVAL_STRINGL(hex, hex_len);
  efree(hex);
}
/* }}} */

/* {{{ Blob::type() */
ZEND_METHOD(Cassandra_Blob, type) {
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_BLOB);
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Blob::bytes() */
ZEND_METHOD(Cassandra_Blob, bytes) {
  auto self = PHP_SCYLLADB_GET_BLOB(ZEND_THIS);
  char *hex;
  size_t hex_len;
  php_scylladb_bytes_to_hex((const char *)self->data, self->size, &hex, &hex_len);

  RETVAL_STRINGL(hex, hex_len);
  efree(hex);
}
/* }}} */

/* {{{ Blob::toBinaryString() */
ZEND_METHOD(Cassandra_Blob, toBinaryString) {
  auto blob = PHP_SCYLLADB_GET_BLOB(ZEND_THIS);

  RETVAL_STRINGL((const char *)blob->data, blob->size);
}
/* }}} */


HashTable *php_scylladb_blob_gc(zend_object *object, zval** table, int *n) {
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *php_scylladb_blob_properties(zend_object *object) {
  char *hex;
  size_t hex_len;
  zval type;
  zval bytes;

  auto self = php_scylladb_blob_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  type = php_scylladb_type_scalar(CASS_VALUE_TYPE_BLOB);
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);

  php_scylladb_bytes_to_hex((const char *)self->data, self->size, &hex, &hex_len);

  ZVAL_STRINGL(&bytes, hex, hex_len);
  efree(hex);
  (void)zend_hash_str_update(props, ZEND_STRL("bytes"), &bytes);

  return props;
}

int php_scylladb_blob_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_scylladb_blob *blob1 = nullptr;
  php_scylladb_blob *blob2 = nullptr;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  blob1 = PHP_SCYLLADB_GET_BLOB(obj1);
  blob2 = PHP_SCYLLADB_GET_BLOB(obj2);

  if (blob1->size == blob2->size) {
    return memcmp((const char *)blob1->data, (const char *)blob2->data,
                  blob1->size);
  } else if (blob1->size < blob2->size) {
    return -1;
  } else {
    return 1;
  }
}

unsigned php_scylladb_blob_hash_value(zval *obj) {
  auto self = PHP_SCYLLADB_GET_BLOB(obj);
  return zend_inline_hash_func((const char *)self->data, self->size);
}

void php_scylladb_blob_free(zend_object *object) {
  auto self = php_scylladb_blob_object_fetch(object);

  if (self->data) {
    efree(self->data);
  }

  zend_object_std_dtor(&self->zendObject);
}

zend_object* php_scylladb_blob_new(zend_class_entry *ce) {
  php_scylladb_blob *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_blob, ce, &php_scylladb_blob_handlers);
  return &self->zendObject;
}


void php_scylladb_blob_post_register([[maybe_unused]] zend_class_entry *ce)
{
    php_scylladb_blob_handlers.std.offset = XtOffsetOf(php_scylladb_blob, zendObject);
}
