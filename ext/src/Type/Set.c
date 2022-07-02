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
#include "util/types.h"
#if PHP_MAJOR_VERSION >= 7
#include <zend_smart_str.h>
#else
#include <ext/standard/php_smart_str.h>
#endif
#include "src/Types/Container/Set/Set.h"

zend_class_entry *php_driver_type_set_ce = NULL;

PHP_METHOD(TypeSet, __construct)
{
  zend_throw_exception_ex(spl_ce_LogicException, 0,
                          "Instantiation of a " PHP_DRIVER_NAMESPACE "\\Type\\Set type is not supported.");
}

PHP_METHOD(TypeSet, name)
{
  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  RETVAL_STRING("set");
}

PHP_METHOD(TypeSet, valueType)
{
  php_driver_type *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());
  RETURN_ZVAL(PHP5TO7_ZVAL_MAYBE_P(self->data.set.value_type), 1, 0);
}

PHP_METHOD(TypeSet, __toString)
{
  php_driver_type *self;
  smart_str string = PHP5TO7_SMART_STR_INIT;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_TYPE(getThis());

  php_driver_type_string(self, &string);
  smart_str_0(&string);

  RETVAL_STRING(PHP5TO7_SMART_STR_VAL(string));
  smart_str_free(&string);
}

PHP_METHOD(TypeSet, create)
{
  php_driver_set *set;
  zval* args = NULL;
  int argc = 0, i;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "*",
                            &args, &argc)
      == FAILURE) {
    return;
  }

  object_init_ex(return_value, php_driver_set_ce);
  set = PHP_DRIVER_GET_SET(return_value);

  PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(set->type), getThis());

  if (argc > 0) {
    for (i = 0; i < argc; i++) {
      if (!php_driver_set_add(set, PHP5TO7_ZVAL_ARG(args[i]))) {
                return;
      }
    }

      }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_value, 0, ZEND_RETURN_VALUE, 0)
#if PHP_MAJOR_VERSION >= 8
  ZEND_ARG_VARIADIC_INFO(0, value)
#else
  ZEND_ARG_INFO(0, value)
#endif
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_type_set_methods[] = {
  PHP_ME(TypeSet, __construct, arginfo_none,  ZEND_ACC_PRIVATE)
  PHP_ME(TypeSet, name,        arginfo_none,  ZEND_ACC_PUBLIC)
  PHP_ME(TypeSet, valueType,   arginfo_none,  ZEND_ACC_PUBLIC)
  PHP_ME(TypeSet, __toString,  arginfo_none,  ZEND_ACC_PUBLIC)
  PHP_ME(TypeSet, create,      arginfo_value, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static zend_object_handlers php_driver_type_set_handlers;

static HashTable*
php_driver_type_set_gc(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object,
#else
  zval* object,
#endif
  zval** table,
  int* n)
{
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *
php_driver_type_set_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
  zval* object
#endif
)
{
#if PHP_MAJOR_VERSION >= 8
  php_driver_type *self  = PHP5TO7_ZEND_OBJECT_GET(type, object);
#else
  php_driver_type *self  = PHP_DRIVER_GET_TYPE(object);
#endif
  HashTable* props = zend_std_get_properties(object);

  PHP5TO7_ZEND_HASH_UPDATE(props,
                           "valueType", sizeof("valueType"),
                           PHP5TO7_ZVAL_MAYBE_P(self->data.set.value_type), sizeof(zval));
  Z_ADDREF_P(PHP5TO7_ZVAL_MAYBE_P(self->data.set.value_type));

  return props;
}

static int
php_driver_type_set_compare(zval* obj1, zval* obj2)
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  php_driver_type* type1 = PHP_DRIVER_GET_TYPE(obj1);
  php_driver_type* type2 = PHP_DRIVER_GET_TYPE(obj2);

  return php_driver_type_compare(type1, type2);
}

static void
php_driver_type_set_free(zend_object* object)
{
  php_driver_type *self = PHP5TO7_ZEND_OBJECT_GET(type, object);

  if (self->data_type) cass_data_type_free(self->data_type);
  ZVAL_DESTROY(self->data.set.value_type);

  zend_object_std_dtor(&self->zval);
  }

static zend_object*
php_driver_type_set_new(zend_class_entry* ce)
{
  php_driver_type *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(type, ce);

  self->type = CASS_VALUE_TYPE_SET;
  self->data_type = cass_data_type_new(self->type);
  ZVAL_UNDEF(&self->data.set.value_type);

  PHP5TO7_ZEND_OBJECT_INIT_EX(type, type_set, self, ce);
}

void
php_driver_define_TypeSet()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Type\\Set", php_driver_type_set_methods);
  php_driver_type_set_ce = php5to7_zend_register_internal_class_ex(&ce, php_driver_type_ce);
  memcpy(&php_driver_type_set_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_type_set_handlers.get_properties  = php_driver_type_set_properties;
#if PHP_VERSION_ID >= 50400
  php_driver_type_set_handlers.get_gc          = php_driver_type_set_gc;
#endif
#if PHP_MAJOR_VERSION >= 8
  php_driver_type_set_handlers.compare = php_driver_type_set_compare;
#else
  php_driver_type_set_handlers.compare_objects = php_driver_type_set_compare;
#endif
  php_driver_type_set_ce->ce_flags     |= ZEND_ACC_FINAL;
  php_driver_type_set_ce->create_object = php_driver_type_set_new;
}
