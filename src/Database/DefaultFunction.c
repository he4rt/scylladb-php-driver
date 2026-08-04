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
#include "Database/ResultDecoder.h"
#include "Type/TypeFactory.h"

#include "DefaultFunction.h"

#include "DefaultFunction_arginfo.h"

extern zend_object_handlers php_scylladb_default_function_handlers;
zval
php_scylladb_create_function(zval *schema,
                              const CassFunctionMeta *meta )
{
  zval result;
  php_scylladb_function *function;
  const char *full_name;
  size_t full_name_length;

  ZVAL_UNDEF(&result);

  object_init_ex(&result, php_scylladb_default_function_ce);

  function = PHP_SCYLLADB_GET_FUNCTION(&result);
  ZVAL_COPY(&function->schema, schema);
  function->meta   = meta;

  cass_function_meta_full_name(function->meta, &full_name, &full_name_length);

  ZVAL_STRINGL(&function->signature, full_name, full_name_length);

  return result;
}

ZEND_METHOD(Cassandra_DefaultFunction, name)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());

  RETURN_ZVAL(&self->signature, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, simpleName)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  if (Z_ISUNDEF(self->simple_name)) {
    const char *name;
    size_t name_length;
    cass_function_meta_name(self->meta, &name, &name_length);

    ZVAL_STRINGL(&self->simple_name, name, name_length);
  }

  RETURN_ZVAL(&self->simple_name, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, arguments)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  if (Z_ISUNDEF(self->arguments)) {
    size_t i, count = cass_function_meta_argument_count(self->meta);

    array_init(&self->arguments);
    for (i = 0; i < count; ++i) {
      const char *name;
      size_t name_length;
      const CassDataType* data_type;
      if (cass_function_meta_argument(self->meta, i, &name, &name_length, &data_type) == CASS_OK) {
        zval type = php_scylladb_type_from_data_type(data_type );
        if (!Z_ISUNDEF(type)) {
          add_assoc_zval_ex(&self->arguments, name, name_length, &type);
        }
      }
    }
  }

  RETURN_ZVAL(&self->arguments, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, returnType)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  if (Z_ISUNDEF(self->return_type)) {
    const CassDataType* data_type = cass_function_meta_return_type(self->meta);
    if (!data_type) {
      return;
    }
    self->return_type = php_scylladb_type_from_data_type(data_type );
  }

  RETURN_ZVAL(&self->return_type, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, signature)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  RETURN_ZVAL(&self->signature, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, language)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  if (Z_ISUNDEF(self->language)) {
    const char *language;
    size_t language_length;
    cass_function_meta_language(self->meta, &language, &language_length);

    ZVAL_STRINGL(&self->language, language, language_length);
  }

  RETURN_ZVAL(&self->language, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, body)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  if (Z_ISUNDEF(self->body)) {
    const char *body;
    size_t body_length;
    cass_function_meta_body(self->meta, &body, &body_length);

    ZVAL_STRINGL(&self->body, body, body_length);
  }

  RETURN_ZVAL(&self->body, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultFunction, isCalledOnNullInput)
{
  php_scylladb_function *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_FUNCTION(getThis());
  RETURN_BOOL((int)cass_function_meta_called_on_null_input(self->meta));
}

HashTable *
php_scylladb_default_function_gc(zend_object *object, zval** table, int *n)
{
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *
php_scylladb_default_function_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

int
php_scylladb_default_function_compare(zval *obj1, zval *obj2 )
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_default_function_free(zend_object *object )
{
  auto self = php_scylladb_function_object_fetch(object);

  zval_ptr_dtor(&self->simple_name);
  zval_ptr_dtor(&self->arguments);
  zval_ptr_dtor(&self->return_type);
  zval_ptr_dtor(&self->signature);
  zval_ptr_dtor(&self->language);
  zval_ptr_dtor(&self->body);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = nullptr;

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_default_function_new(zend_class_entry *ce )
{
  php_scylladb_function *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_function, ce, &php_scylladb_default_function_handlers);

  ZVAL_UNDEF(&self->simple_name);
  ZVAL_UNDEF(&self->arguments);
  ZVAL_UNDEF(&self->return_type);
  ZVAL_UNDEF(&self->signature);
  ZVAL_UNDEF(&self->language);
  ZVAL_UNDEF(&self->body);

  ZVAL_UNDEF(&self->schema);
  self->meta = nullptr;

  php_scylladb_default_function_handlers.offset = offsetof(php_scylladb_function, zendObject);
  php_scylladb_default_function_handlers.free_obj = php_scylladb_default_function_free;
  return &self->zendObject;
}
