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
#include "util/result.h"
#include "util/types.h"

#include "DefaultColumn.h"
BEGIN_EXTERN_C()
#include "DefaultColumn_arginfo.h"
zend_class_entry *php_driver_default_column_ce = NULL;

zval
php_driver_create_column(zval *schema,
                         const CassColumnMeta *meta )
{
  zval result;
  php_driver_column *column;
  const char *name;
  size_t name_length;
  const CassValue *value;

  ZVAL_UNDEF(&result);


  object_init_ex(&result, php_driver_default_column_ce);

  column = PHP_DRIVER_GET_COLUMN(&result);
  ZVAL_COPY(&column->schema, schema);
  column->meta   = meta;

  cass_column_meta_name(meta, &name, &name_length);

  ZVAL_STRINGL(&column->name, name, name_length);

  value = cass_column_meta_field_by_name(meta, "validator");
  if (value) {
    const char *validator;
    size_t validator_length;

    ASSERT_SUCCESS_BLOCK(cass_value_get_string(value,
                                               &validator,
                                               &validator_length),
     zval_ptr_dtor(&result);
     ZVAL_UNDEF(&result);
     return result;
    );

    if (php_driver_parse_column_type(validator, validator_length,
                                        &column->reversed, &column->frozen,
                                        &column->type ) == FAILURE) {
      zval_ptr_dtor(&result);
      ZVAL_UNDEF(&result);
      return result;
    }
  } else {
    const CassDataType *data_type = cass_column_meta_data_type(meta);
    if (data_type) {
      const char *clustering_order;
      size_t clustering_order_length;
      column->type = php_driver_type_from_data_type(data_type );

#if CURRENT_CPP_DRIVER_VERSION > CPP_DRIVER_VERSION(2, 2, 0)
      column->frozen = cass_data_type_is_frozen(data_type);
#else
      column->frozen = 0;
#endif

      value = cass_column_meta_field_by_name(meta, "clustering_order");
      if (!value) {
        zend_throw_exception_ex(php_driver_runtime_exception_ce, 0 ,
                                "Unable to get column field \"clustering_order\"");
        zval_ptr_dtor(&result);
        ZVAL_UNDEF(&result);
        return result;
      }

     ASSERT_SUCCESS_BLOCK(cass_value_get_string(value,
                                                &clustering_order,
                                                &clustering_order_length),
        zval_ptr_dtor(&result);
        ZVAL_UNDEF(&result);
        return result;
      );
      column->reversed =
          (clustering_order_length == sizeof("desc") - 1 &&
           memcmp(clustering_order, "desc", clustering_order_length) == 0) ? 1 : 0;
    }
  }

  return result;
}


ZEND_METHOD(Cassandra_DefaultColumn, name)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_ZVAL(&self->name, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultColumn, type)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  if (Z_ISUNDEF(self->type)) {
    RETURN_NULL();
  }

  RETURN_ZVAL(&self->type, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultColumn, isReversed)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_BOOL(self->reversed);
}

ZEND_METHOD(Cassandra_DefaultColumn, isStatic)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self  = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_BOOL(cass_column_meta_type(self->meta) == CASS_COLUMN_TYPE_STATIC);
}

ZEND_METHOD(Cassandra_DefaultColumn, isFrozen)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_BOOL(self->frozen);
}

ZEND_METHOD(Cassandra_DefaultColumn, indexName)
{
  php_driver_column *self;
  zval value;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  php_driver_get_column_field(self->meta, "index_name", &value );
  RETURN_ZVAL(&value, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultColumn, indexOptions)
{
  php_driver_column *self;
  zval value;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  php_driver_get_column_field(self->meta, "index_options", &value );
  RETURN_ZVAL(&value, 0, 1);
}

static zend_object_handlers php_driver_default_column_handlers;

static HashTable *
php_driver_type_default_column_gc(
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
php_driver_default_column_properties(
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
php_driver_default_column_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

static void
php_driver_default_column_free(zend_object *object )
{
  php_driver_column *self = php_driver_column_object_fetch(object);

  zval_ptr_dtor(&self->name);
  zval_ptr_dtor(&self->type);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = NULL;

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_default_column_new(zend_class_entry *ce )
{
  php_driver_column *self =
      (php_driver_column *)ecalloc(1, sizeof(php_driver_column) + zend_object_properties_size(ce));

  self->reversed = 0;
  self->frozen   = 0;
  ZVAL_UNDEF(&self->schema);
  self->meta     = NULL;
  ZVAL_UNDEF(&self->name);
  ZVAL_UNDEF(&self->type);

  zend_object_std_init(&self->zendObject, ce);
  php_driver_default_column_handlers.offset = XtOffsetOf(php_driver_column, zendObject);
  php_driver_default_column_handlers.free_obj = php_driver_default_column_free;
  self->zendObject.handlers = (zend_object_handlers *)&php_driver_default_column_handlers;
  return &self->zendObject;
}

void php_driver_define_DefaultColumn()
{
  php_driver_default_column_ce = register_class_Cassandra_DefaultColumn(php_driver_column_ce);
  php_driver_default_column_ce->create_object = php_driver_default_column_new;

  memcpy(&php_driver_default_column_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_default_column_handlers.get_properties  = php_driver_default_column_properties;
#if PHP_VERSION_ID >= 50400
  php_driver_default_column_handlers.get_gc          = php_driver_type_default_column_gc;
#endif
#if PHP_MAJOR_VERSION >= 8
  php_driver_default_column_handlers.compare = php_driver_default_column_compare;
#else
  php_driver_default_column_handlers.compare_objects = php_driver_default_column_compare;
#endif
  php_driver_default_column_handlers.clone_obj = NULL;
}
END_EXTERN_C()