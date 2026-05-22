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
#include "Util/result.h"
#include "Util/types.h"

#include "DefaultFunction.h"
BEGIN_EXTERN_C()
#include "DefaultAggregate_arginfo.h"
zend_class_entry *php_driver_default_aggregate_ce = NULL;

ZEND_METHOD(Cassandra_DefaultAggregate, name)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());

  RETURN_ZVAL(&self->signature, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, simpleName)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->simple_name)) {
    const char *name;
    size_t name_length;
    cass_aggregate_meta_name(self->meta, &name, &name_length);

    ZVAL_STRINGL(&self->simple_name, name, name_length);
  }

  RETURN_ZVAL(&self->simple_name, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, argumentTypes)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->argument_types)) {
    size_t i, count = cass_aggregate_meta_argument_count(self->meta);

    array_init(&self->argument_types);
    for (i = 0; i < count; ++i) {
      const CassDataType* data_type = cass_aggregate_meta_argument_type(self->meta, i);
      if (data_type) {
        zval type = php_driver_type_from_data_type(data_type );
        if (!Z_ISUNDEF(type)) {
          add_next_index_zval(&self->argument_types,
                              &type);
        }
      }
    }
  }

  RETURN_ZVAL(&self->argument_types, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, stateFunction)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->state_function)) {
    const CassFunctionMeta* function = cass_aggregate_meta_state_func(self->meta);
    if (!function) {
      return;
    }
    self->state_function =
        php_driver_create_function(&self->schema, function );
  }

  RETURN_ZVAL(&self->state_function, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, finalFunction)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->final_function)) {
    const CassFunctionMeta* function = cass_aggregate_meta_final_func(self->meta);
    if (!function) {
      return;
    }
    self->final_function =
        php_driver_create_function(&self->schema, function );
  }

  RETURN_ZVAL(&self->final_function, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, initialCondition)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->initial_condition)) {
    const CassValue *value = cass_aggregate_meta_init_cond(self->meta);
    const CassDataType *data_type = NULL;
    if (!value) {
      return;
    }
    data_type = cass_value_data_type(value);
    if (!data_type) {
      return;
    }
    php_driver_value(value, data_type, &self->initial_condition );
  }

  RETURN_ZVAL(&self->initial_condition, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, stateType)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->state_type)) {
    const CassDataType* data_type = cass_aggregate_meta_state_type(self->meta);
    if (!data_type) {
      return;
    }
    self->state_type = php_driver_type_from_data_type(data_type );
  }

  RETURN_ZVAL(&self->state_type, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, returnType)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->return_type)) {
    const CassDataType* data_type = cass_aggregate_meta_return_type(self->meta);
    if (!data_type) {
      return;
    }
    self->return_type = php_driver_type_from_data_type(data_type );
  }

  RETURN_ZVAL(&self->return_type, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultAggregate, signature)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  RETURN_ZVAL(&self->signature, 1, 0);
}

static zend_object_handlers php_driver_default_aggregate_handlers;

static HashTable *
php_driver_type_default_aggregate_gc(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object,
#else
        zendObject *object,
#endif
        zval** table, int *n
)
{
  *table = NULL;
  *n = 0;
  return NULL;
}

static HashTable *
php_driver_default_aggregate_properties(
#if PHP_MAJOR_VERSION >= 8
        zend_object *object
#else
        zendObject *object
#endif
)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

static int
php_driver_default_aggregate_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

static void
php_driver_default_aggregate_free(zend_object *object )
{
  php_driver_aggregate *self = php_driver_aggregate_object_fetch(object);

  zval_ptr_dtor(&self->simple_name);
  zval_ptr_dtor(&self->argument_types);
  zval_ptr_dtor(&self->state_function);
  zval_ptr_dtor(&self->final_function);
  zval_ptr_dtor(&self->initial_condition);
  zval_ptr_dtor(&self->state_type);
  zval_ptr_dtor(&self->return_type);
  zval_ptr_dtor(&self->signature);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = NULL;

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_default_aggregate_new(zend_class_entry *ce )
{
  php_driver_aggregate *self =
      (php_driver_aggregate *)ecalloc(1, sizeof(php_driver_aggregate) + zend_object_properties_size(ce));

  ZVAL_UNDEF(&self->simple_name);
  ZVAL_UNDEF(&self->argument_types);
  ZVAL_UNDEF(&self->state_function);
  ZVAL_UNDEF(&self->final_function);
  ZVAL_UNDEF(&self->initial_condition);
  ZVAL_UNDEF(&self->state_type);
  ZVAL_UNDEF(&self->return_type);
  ZVAL_UNDEF(&self->signature);

  ZVAL_UNDEF(&self->schema);
  self->meta = NULL;

  zend_object_std_init(&self->zendObject, ce);
  php_driver_default_aggregate_handlers.offset = XtOffsetOf(php_driver_aggregate, zendObject);
  php_driver_default_aggregate_handlers.free_obj = php_driver_default_aggregate_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_default_aggregate_handlers;
  return &self->zendObject;
}

void php_driver_define_DefaultAggregate()
{
  php_driver_default_aggregate_ce = register_class_Cassandra_DefaultAggregate(php_driver_aggregate_ce);
  php_driver_default_aggregate_ce->create_object = php_driver_default_aggregate_new;

  memcpy(&php_driver_default_aggregate_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_default_aggregate_handlers.get_properties  = php_driver_default_aggregate_properties;
#if PHP_VERSION_ID >= 50400
  php_driver_default_aggregate_handlers.get_gc          = php_driver_type_default_aggregate_gc;
#endif
#if PHP_MAJOR_VERSION >= 8
  php_driver_default_aggregate_handlers.compare = php_driver_default_aggregate_compare;
#else
  php_driver_default_aggregate_handlers.compare_objects = php_driver_default_aggregate_compare;
#endif
  php_driver_default_aggregate_handlers.clone_obj = NULL;
}
END_EXTERN_C()