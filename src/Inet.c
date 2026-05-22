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

#include "InetUtil.h"
#include "php_scylladb.h"
#include "php_scylladb_types.h"
#include "Type/TypeFactory.h"

#include "Inet_arginfo.h"

extern php_scylladb_value_handlers php_scylladb_inet_handlers;


void
php_scylladb_inet_init(INTERNAL_FUNCTION_PARAMETERS)
{
  php_scylladb_inet *self;
  char *string;
  size_t string_len;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(string, string_len)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  if (ZEND_THIS && instanceof_function(Z_OBJCE_P(ZEND_THIS), php_scylladb_inet_ce)) {
    self = PHP_SCYLLADB_GET_INET(ZEND_THIS);
  } else {
    object_init_ex(return_value, php_scylladb_inet_ce);
    self = PHP_SCYLLADB_GET_INET(return_value);
  }

  if (!php_scylladb_parse_ip_address(string, &self->inet )) {
    return;
  }
}

/* {{{ Inet::__construct(string) */
ZEND_METHOD(Cassandra_Inet, __construct)
{
  php_scylladb_inet_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Inet::__toString() */
ZEND_METHOD(Cassandra_Inet, __toString)
{
  php_scylladb_inet *inet = PHP_SCYLLADB_GET_INET(ZEND_THIS);
  char *string;
  php_scylladb_format_address(inet->inet, &string);

  RETVAL_STRING(string);
  efree(string);
}
/* }}} */

/* {{{ Inet::type() */
ZEND_METHOD(Cassandra_Inet, type)
{
  zval type = php_scylladb_type_scalar(CASS_VALUE_TYPE_INET );
  RETURN_ZVAL(&type, 1, 1);
}
/* }}} */

/* {{{ Inet::address() */
ZEND_METHOD(Cassandra_Inet, address)
{
  php_scylladb_inet *inet = PHP_SCYLLADB_GET_INET(ZEND_THIS);
  char *string;
  php_scylladb_format_address(inet->inet, &string);

  RETVAL_STRING(string);
  efree(string);
}
/* }}} */


HashTable *
php_scylladb_inet_gc(zend_object *object, zval** table, int *n)
{
  *table = NULL;
  *n = 0;
  return NULL;
}

HashTable *
php_scylladb_inet_properties(zend_object *object)
{
  char *string;
  zval type;
  zval address;

  php_scylladb_inet *self = php_scylladb_inet_object_fetch(object);
  if (object->properties) {
    zend_array_release(object->properties);
  }
  object->properties = zend_new_array(2);
  HashTable *props = object->properties;

  type = php_scylladb_type_scalar(CASS_VALUE_TYPE_INET );
  (void)zend_hash_str_update(props, ZEND_STRL("type"), &type);

  php_scylladb_format_address(self->inet, &string);

  ZVAL_STRING(&address, string);
  efree(string);
  (void)zend_hash_str_update(props, ZEND_STRL("address"), &address);

  return props;
}

int
php_scylladb_inet_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  php_scylladb_inet *inet1 = NULL;
  php_scylladb_inet *inet2 = NULL;

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  inet1 = PHP_SCYLLADB_GET_INET(obj1);
  inet2 = PHP_SCYLLADB_GET_INET(obj2);

  if (inet1->inet.address_length != inet2->inet.address_length) {
   return inet1->inet.address_length < inet2->inet.address_length ? -1 : 1;
  }
  return memcmp(inet1->inet.address, inet2->inet.address, inet1->inet.address_length);
}

unsigned
php_scylladb_inet_hash_value(zval *obj )
{
  php_scylladb_inet *self = PHP_SCYLLADB_GET_INET(obj);
  return zend_inline_hash_func((const char *) self->inet.address,
                               self->inet.address_length);
}

void
php_scylladb_inet_free(zend_object *object)
{
  php_scylladb_inet *self = php_scylladb_inet_object_fetch(object);

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_inet_new(zend_class_entry *ce)
{
  php_scylladb_inet *self = (php_scylladb_inet *)ecalloc(1, sizeof(php_scylladb_inet) + zend_object_properties_size(ce));

  zend_object_std_init(&self->zendObject, ce);
  self->zendObject.handlers = (zend_object_handlers *)&php_scylladb_inet_handlers;
  return &self->zendObject;
}


void php_scylladb_inet_post_register(zend_class_entry *ce)
{
    (void)ce;
    php_scylladb_inet_handlers.std.offset = XtOffsetOf(php_scylladb_inet, zendObject);
}
