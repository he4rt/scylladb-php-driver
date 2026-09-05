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

#ifdef PHP_SCYLLADB_HAVE_LEGACY_SCHEMA_META
#include "DefaultFunction.h"
#endif
#include "DefaultMaterializedView.h"
#include "DefaultTable.h"
#include "php_scylladb.h"
#include "php_scylladb_types.h"
#include "Database/ResultDecoder.h"
#include "Type/TypeFactory.h"

#include "Zend/zend_smart_str.h"
#include "DefaultKeyspace_arginfo.h"

extern zend_object_handlers php_scylladb_default_keyspace_handlers;
ZEND_METHOD(Cassandra_DefaultKeyspace, name) {
  php_scylladb_keyspace *self;
  zval value;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);

  php_scylladb_get_keyspace_field(self->meta, "keyspace_name", &value);
  RETURN_ZVAL(&value, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, replicationClassName) {
  php_scylladb_keyspace *self;
  zval value;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);

  php_scylladb_get_keyspace_field(self->meta, "strategy_class", &value);
  RETURN_ZVAL(&value, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, replicationOptions) {
  php_scylladb_keyspace *self;
  zval value;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);

  php_scylladb_get_keyspace_field(self->meta, "strategy_options", &value);
  RETURN_ZVAL(&value, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, hasDurableWrites) {
  php_scylladb_keyspace *self;
  zval value;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);

  php_scylladb_get_keyspace_field(self->meta, "durable_writes", &value);
  RETURN_ZVAL(&value, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, table) {
  zend_string *name = nullptr;
  php_scylladb_keyspace *self;
  zval ztable;
  const CassTableMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  meta = cass_keyspace_meta_table_by_name_n(self->meta, ZSTR_VAL(name), ZSTR_LEN(name));
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  ztable = php_scylladb_create_table(&self->schema, meta);
  if (Z_ISUNDEF(ztable)) {
    return;
  }

  RETURN_ZVAL(&ztable, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, tables) {
  php_scylladb_keyspace *self;
  CassIterator *iterator;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  iterator = cass_iterator_tables_from_keyspace_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassTableMeta *meta;
    zval ztable;
    php_scylladb_table *table;

    meta = cass_iterator_get_table_meta(iterator);
    ztable = php_scylladb_create_table(&self->schema, meta);

    if (Z_ISUNDEF(ztable)) {
      zval_ptr_dtor(return_value);
      cass_iterator_free(iterator);
      return;
    } else {
      table = PHP_SCYLLADB_GET_TABLE(&ztable);

      if (Z_TYPE(table->name) == IS_STRING) {
        add_assoc_zval_ex(return_value, Z_STRVAL(table->name), Z_STRLEN(table->name), &ztable);
      } else {
        add_next_index_zval(return_value, &ztable);
      }
    }
  }

  cass_iterator_free(iterator);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, userType) {
  zend_string *name = nullptr;
  php_scylladb_keyspace *self;
  zval ztype;
  const CassDataType *user_type;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  user_type = cass_keyspace_meta_user_type_by_name_n(self->meta, ZSTR_VAL(name), ZSTR_LEN(name));

  if (user_type == nullptr) {
    return;
  }

  ztype = php_scylladb_type_from_data_type(user_type);
  RETURN_ZVAL(&ztype, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, userTypes) {
  php_scylladb_keyspace *self;
  CassIterator *iterator;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  iterator = cass_iterator_user_types_from_keyspace_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassDataType *user_type;
    zval ztype;
    const char *type_name;
    size_t type_name_len;

    user_type = cass_iterator_get_user_type(iterator);
    ztype = php_scylladb_type_from_data_type(user_type);

    cass_data_type_type_name(user_type, &type_name, &type_name_len);
    add_assoc_zval_ex(return_value, type_name, type_name_len, &ztype);
  }

  cass_iterator_free(iterator);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, materializedView) {
  php_scylladb_keyspace *self;
  zend_string *name = nullptr;
  zval zview;
  const CassMaterializedViewMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  meta = cass_keyspace_meta_materialized_view_by_name_n(self->meta, ZSTR_VAL(name), ZSTR_LEN(name));
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  zview = php_scylladb_create_materialized_view(&self->schema, meta);
  if (Z_ISUNDEF(zview)) {
    return;
  }

  RETURN_ZVAL(&zview, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultKeyspace, materializedViews) {
  php_scylladb_keyspace *self;
  CassIterator *iterator;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  iterator = cass_iterator_materialized_views_from_keyspace_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassMaterializedViewMeta *meta;
    zval zview;
    php_scylladb_materialized_view *view;

    meta = cass_iterator_get_materialized_view_meta(iterator);
    zview = php_scylladb_create_materialized_view(&self->schema, meta);

    if (Z_ISUNDEF(zview)) {
      zval_ptr_dtor(return_value);
      cass_iterator_free(iterator);
      return;
    } else {
      view = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(&zview);

      if (Z_TYPE(view->name) == IS_STRING) {
        add_assoc_zval_ex(return_value, Z_STRVAL(view->name), Z_STRLEN(view->name), &zview);
      } else {
        add_next_index_zval(return_value, &zview);
      }
    }
  }

  cass_iterator_free(iterator);
}

int php_scylladb_arguments_string(zval* args, uint32_t argc, smart_str *arguments) {
  uint32_t i;

  for (i = 0; i < argc; ++i) {
    zval *argument_type = &args[i];

    if (i > 0) {
      smart_str_appendc_ex(arguments, ',', 0);
    }

    if (Z_TYPE_P(argument_type) == IS_STRING) {
      smart_str_appendl_ex(arguments, Z_STRVAL_P(argument_type), Z_STRLEN_P(argument_type), 0);
    } else if (Z_TYPE_P(argument_type) == IS_OBJECT &&
               instanceof_function(Z_OBJCE_P(argument_type), php_scylladb_type_ce)) {
      auto type = PHP_SCYLLADB_GET_TYPE(argument_type);
      php_scylladb_type_string(type, arguments);
    } else {
      zend_throw_exception_ex(
          php_scylladb_invalid_argument_exception_ce, 0,
          "Argument types must be either a string or an instance of " PHP_SCYLLADB_NAMESPACE
          "\\Type");
      return FAILURE;
    }
  }

  smart_str_0(arguments);

  return SUCCESS;
}

#ifdef PHP_SCYLLADB_HAVE_LEGACY_SCHEMA_META
ZEND_METHOD(Cassandra_DefaultKeyspace, function) {
  php_scylladb_keyspace *self;
  zend_string *name = nullptr;
  zval* args = nullptr;
  smart_str arguments = {nullptr,0};
  uint32_t argc = 0;
  const CassFunctionMeta *meta = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_STR(name)
    Z_PARAM_OPTIONAL
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);

  if (argc > 0) {
    if (php_scylladb_arguments_string(args, argc, &arguments) == FAILURE) {

      return;
    }
  }

  meta = cass_keyspace_meta_function_by_name_n(self->meta, ZSTR_VAL(name), ZSTR_LEN(name),
                                               (arguments.s ? arguments.s->val : nullptr),
                                               (arguments.s ? arguments.s->len : 0));
  if (meta) {
    zval zfunction = php_scylladb_create_function(&self->schema, meta);
    RETVAL_ZVAL(&zfunction, 1, 1);
  } else {
    RETVAL_FALSE;
  }

  smart_str_free(&arguments);

}

ZEND_METHOD(Cassandra_DefaultKeyspace, functions) {
  php_scylladb_keyspace *self;
  CassIterator *iterator;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  iterator = cass_iterator_functions_from_keyspace_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassFunctionMeta *meta = cass_iterator_get_function_meta(iterator);
    zval zfunction = php_scylladb_create_function(&self->schema, meta);

    if (!Z_ISUNDEF(zfunction)) {
      auto function = PHP_SCYLLADB_GET_FUNCTION(&zfunction);

      if (Z_TYPE(function->signature) == IS_STRING) {
        add_assoc_zval_ex(return_value, Z_STRVAL(function->signature), Z_STRLEN(function->signature), &zfunction);
      } else {
        add_next_index_zval(return_value, &zfunction);
      }
    }
  }

  cass_iterator_free(iterator);
}

static zval php_scylladb_create_aggregate(zval *schema,
                                                const CassAggregateMeta *meta) {
  zval result;
  php_scylladb_aggregate *aggregate;
  const char *full_name;
  size_t full_name_length;

  ZVAL_UNDEF(&result);

  object_init_ex(&result, php_scylladb_default_aggregate_ce);

  aggregate = PHP_SCYLLADB_GET_AGGREGATE(&result);
  ZVAL_COPY(&aggregate->schema, schema);
  aggregate->meta = meta;

  cass_aggregate_meta_full_name(aggregate->meta, &full_name, &full_name_length);

  ZVAL_STRINGL(&aggregate->signature, full_name, full_name_length);

  return result;
}

ZEND_METHOD(Cassandra_DefaultKeyspace, aggregate) {
  php_scylladb_keyspace *self;
  zend_string *name = nullptr;
  zval *args;
  args = nullptr;
  smart_str arguments = {nullptr, 0};
  uint32_t argc = 0;
  const CassAggregateMeta *meta = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_STR(name)
    Z_PARAM_OPTIONAL
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);

  if (argc > 0) {
    if (php_scylladb_arguments_string(args, argc, &arguments) == FAILURE) {

      return;
    }
  }

  meta = cass_keyspace_meta_aggregate_by_name_n(self->meta, ZSTR_VAL(name), ZSTR_LEN(name),
                                                (arguments.s ? arguments.s->val : nullptr),
                                                (arguments.s ? arguments.s->len : 0));
  if (meta) {
    zval zaggregate = php_scylladb_create_aggregate(&self->schema, meta);
    RETVAL_ZVAL(&zaggregate, 1, 1);
  } else {
    RETVAL_FALSE;
  }

  smart_str_free(&arguments);

}

ZEND_METHOD(Cassandra_DefaultKeyspace, aggregates) {
  php_scylladb_keyspace *self;
  CassIterator *iterator;

  ZEND_PARSE_PARAMETERS_NONE();

  self = PHP_SCYLLADB_GET_KEYSPACE(ZEND_THIS);
  iterator = cass_iterator_aggregates_from_keyspace_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassAggregateMeta *meta = cass_iterator_get_aggregate_meta(iterator);
    zval zaggregate = php_scylladb_create_aggregate(&self->schema, meta);

    if (!Z_ISUNDEF(zaggregate)) {
      auto aggregate = PHP_SCYLLADB_GET_AGGREGATE(&zaggregate);

      if (Z_TYPE(aggregate->signature) == IS_STRING) {
        add_assoc_zval_ex(return_value, Z_STRVAL(aggregate->signature), Z_STRLEN(aggregate->signature), &zaggregate);
      } else {
        add_next_index_zval(return_value, &zaggregate);
      }
    }
  }

  cass_iterator_free(iterator);
}
#else
ZEND_METHOD(Cassandra_DefaultKeyspace, function) {
  PHP_SCYLLADB_THROW_NO_LEGACY_SCHEMA_META("Cassandra\\Keyspace::function()");
}

ZEND_METHOD(Cassandra_DefaultKeyspace, functions) {
  PHP_SCYLLADB_THROW_NO_LEGACY_SCHEMA_META("Cassandra\\Keyspace::functions()");
}

ZEND_METHOD(Cassandra_DefaultKeyspace, aggregate) {
  PHP_SCYLLADB_THROW_NO_LEGACY_SCHEMA_META("Cassandra\\Keyspace::aggregate()");
}

ZEND_METHOD(Cassandra_DefaultKeyspace, aggregates) {
  PHP_SCYLLADB_THROW_NO_LEGACY_SCHEMA_META("Cassandra\\Keyspace::aggregates()");
}
#endif

HashTable *php_scylladb_default_keyspace_gc(zend_object *object, zval** table, int *n) {
  auto self = php_scylladb_keyspace_object_fetch(object);
  zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
  zend_get_gc_buffer_add_zval(buffer, &self->schema);
  zend_get_gc_buffer_use(buffer, table, n);

  return nullptr;
}

HashTable *php_scylladb_default_keyspace_properties(zend_object *object) {
  HashTable *props = zend_std_get_properties(object);

  return props;
}

int php_scylladb_default_keyspace_compare(zval *obj1, zval *obj2) {
  PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_default_keyspace_free(zend_object *object) {
  auto self = php_scylladb_keyspace_object_fetch(object);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = nullptr;

  zend_object_std_dtor(&self->zendObject);

}

zend_object* php_scylladb_default_keyspace_new(zend_class_entry *ce) {
  php_scylladb_keyspace *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_keyspace, ce, &php_scylladb_default_keyspace_handlers);

  self->meta = nullptr;
  ZVAL_UNDEF(&self->schema);

  return &self->zendObject;
}
