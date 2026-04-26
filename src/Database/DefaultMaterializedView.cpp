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
#include "util/ref.h"
#include "util/result.h"
#include "util/types.h"

#include "DefaultColumn.h"
#include "DefaultIndex.h"
#include "DefaultTable.h"
#include "Table.h"
BEGIN_EXTERN_C()
zend_class_entry *php_driver_default_materialized_view_ce = NULL;

static void
populate_partition_key(php_driver_materialized_view *view, zval *result )
{
  size_t i, count = cass_materialized_view_meta_partition_key_count(view->meta);
  for (i = 0; i < count; ++i) {
    const CassColumnMeta *column =
        cass_materialized_view_meta_partition_key(view->meta, i);
    if (column) {
      zval zcolumn = php_driver_create_column(view->schema, column );
      if (!Z_ISUNDEF(zcolumn)) {
        add_next_index_zval(result, &zcolumn);
      }
    }
  }
}

static void
populate_clustering_key(php_driver_materialized_view *view, zval *result )
{
  size_t i, count = cass_materialized_view_meta_clustering_key_count(view->meta);
  for (i = 0; i < count; ++i) {
    const CassColumnMeta *column =
        cass_materialized_view_meta_clustering_key(view->meta, i);
    if (column) {
      zval zcolumn = php_driver_create_column(view->schema, column );
      if (!Z_ISUNDEF(zcolumn)) {
        add_next_index_zval(result, &zcolumn);
      }
    }
  }
}

zval
php_driver_create_materialized_view(php_driver_ref* schema,
                                       const CassMaterializedViewMeta *meta )
{
  zval result;
  php_driver_materialized_view *view;
  const char *name;
  size_t name_length;

  ZVAL_UNDEF(&result);


  object_init_ex(&result, php_driver_default_materialized_view_ce);

  view = PHP_DRIVER_GET_MATERIALIZED_VIEW(&result);
  view->schema = php_driver_add_ref(schema);
  view->meta   = meta;

  cass_materialized_view_meta_name(meta, &name, &name_length);

  ZVAL_STRINGL(&view->name, name, name_length);

  return result;
}

void
php_driver_default_materialized_view_build_options(php_driver_materialized_view *view ) {
  CassIterator *iterator =
      cass_iterator_fields_from_materialized_view_meta(view->meta);
  view->options =
      php_driver_table_build_options(iterator );
  cass_iterator_free(iterator);
}

void
php_driver_materialized_view_get_option(php_driver_materialized_view *view,
                                           const char *name,
                                           zval *result ) {
  zval *zvalue;
  if (Z_ISUNDEF(view->options)) {
    php_driver_default_materialized_view_build_options(view );
  }

  if (!PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL(view->options),
                         name, strlen(name) + 1,
                         zvalue)) {
    ZVAL_FALSE(result);
    return;
  }

  ZVAL_COPY(result, zvalue);
}

PHP_METHOD(DefaultMaterializedView, name)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  RETURN_ZVAL(&self->name, 1, 0);
}

PHP_METHOD(DefaultMaterializedView, option)
{
  char *name;
  size_t name_len;
  php_driver_materialized_view *self;
  zval* result;

  if (zend_parse_parameters(ZEND_NUM_ARGS() , "s",
                            &name, &name_len) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_driver_default_materialized_view_build_options(self );
  }

  if (PHP5TO7_ZEND_HASH_FIND(Z_ARRVAL(self->options),
                         name, name_len + 1,
                         result)) {
    RETURN_ZVAL(result, 1, 0);
  }
  RETURN_FALSE;
}

PHP_METHOD(DefaultMaterializedView, options)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_driver_default_materialized_view_build_options(self );
  }

  RETURN_ZVAL(&self->options, 1, 0);
}

PHP_METHOD(DefaultMaterializedView, comment)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "comment", return_value );
}

PHP_METHOD(DefaultMaterializedView, readRepairChance)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "read_repair_chance", return_value );
}

PHP_METHOD(DefaultMaterializedView, localReadRepairChance)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "local_read_repair_chance", return_value );
}

PHP_METHOD(DefaultMaterializedView, gcGraceSeconds)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "gc_grace_seconds", return_value );
}

PHP_METHOD(DefaultMaterializedView, caching)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "caching", return_value );
}

PHP_METHOD(DefaultMaterializedView, bloomFilterFPChance)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "bloom_filter_fp_chance", return_value );
}

PHP_METHOD(DefaultMaterializedView, memtableFlushPeriodMs)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "memtable_flush_period_in_ms", return_value );
}

PHP_METHOD(DefaultMaterializedView, defaultTTL)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "default_time_to_live", return_value );
}

PHP_METHOD(DefaultMaterializedView, speculativeRetry)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "speculative_retry", return_value );
}

PHP_METHOD(DefaultMaterializedView, indexInterval)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "index_interval", return_value );
}

PHP_METHOD(DefaultMaterializedView, compactionStrategyClassName)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "compaction_strategy_class", return_value );
}

PHP_METHOD(DefaultMaterializedView, compactionStrategyOptions)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "compaction_strategy_options", return_value );
}

PHP_METHOD(DefaultMaterializedView, compressionParameters)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "compression_parameters", return_value );
}

PHP_METHOD(DefaultMaterializedView, populateIOCacheOnFlush)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "populate_io_cache_on_flush", return_value );
}

PHP_METHOD(DefaultMaterializedView, replicateOnWrite)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "replicate_on_write", return_value );
}

PHP_METHOD(DefaultMaterializedView, maxIndexInterval)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "max_index_interval", return_value );
}

PHP_METHOD(DefaultMaterializedView, minIndexInterval)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());

  php_driver_materialized_view_get_option(self, "min_index_interval", return_value );
}


PHP_METHOD(DefaultMaterializedView, column)
{
  php_driver_materialized_view *self;
  char *name;
  size_t name_len;
  zval column;
  const CassColumnMeta *meta;

  if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &name, &name_len) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  meta = cass_materialized_view_meta_column_by_name(self->meta, name);
  if (meta == NULL) {
    RETURN_FALSE;
  }

  column = php_driver_create_column(self->schema, meta );
  if (Z_ISUNDEF(column)) {
    return;
  }

  RETURN_ZVAL(&column, 0, 1);
}

PHP_METHOD(DefaultMaterializedView, columns)
{
  php_driver_materialized_view *self;
  CassIterator    *iterator;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self     = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  iterator = cass_iterator_columns_from_materialized_view_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassColumnMeta *meta;
    zval zcolumn;
    php_driver_column *column;

    meta    = cass_iterator_get_column_meta(iterator);
    zcolumn = php_driver_create_column(self->schema, meta );

    if (!Z_ISUNDEF(zcolumn)) {
      column = PHP_DRIVER_GET_COLUMN(&zcolumn);

      if (Z_TYPE(column->name) == IS_STRING) {
        PHP5TO7_ADD_ASSOC_ZVAL_EX(return_value,
                                  Z_STRVAL(column->name),
                                  Z_STRLEN(column->name) + 1,
                                  &zcolumn);
      } else {
        add_next_index_zval(return_value, &zcolumn);
      }
    }
  }

  cass_iterator_free(iterator);
}

PHP_METHOD(DefaultMaterializedView, partitionKey)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->partition_key)) {

    array_init(&self->partition_key);
    populate_partition_key(self, &self->partition_key );
  }

  RETURN_ZVAL(&self->partition_key, 1, 0);
}

PHP_METHOD(DefaultMaterializedView, primaryKey)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->primary_key)) {

    array_init(&self->primary_key);
    populate_partition_key(self, &self->primary_key );
    populate_clustering_key(self, &self->primary_key );
  }

  RETURN_ZVAL(&self->primary_key, 1, 0);
}

PHP_METHOD(DefaultMaterializedView, clusteringKey)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->clustering_key)) {

    array_init(&self->clustering_key);
    populate_clustering_key(self, &self->clustering_key );
  }

  RETURN_ZVAL(&self->clustering_key, 1, 0);
}

PHP_METHOD(DefaultMaterializedView, clusteringOrder)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->clustering_order)) {
    size_t i, count = cass_materialized_view_meta_clustering_key_count(self->meta);

    array_init(&self->clustering_order);
    for (i = 0; i < count; ++i) {
      CassClusteringOrder order =
          cass_materialized_view_meta_clustering_key_order(self->meta, i);
      switch (order) {
        case CASS_CLUSTERING_ORDER_ASC:
          add_next_index_string(&self->clustering_order, "asc");
          break;
        case CASS_CLUSTERING_ORDER_DESC:
          add_next_index_string(&self->clustering_order, "desc");
          break;
        case CASS_CLUSTERING_ORDER_NONE:
          add_next_index_string(&self->clustering_order, "none");
          break;
      }
    }
  }

  RETURN_ZVAL(&self->clustering_order, 1, 0);
}

PHP_METHOD(DefaultMaterializedView, baseTable)
{
  php_driver_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->base_table)) {
    const CassTableMeta *table =
        cass_materialized_view_meta_base_table(self->meta);
    if (!table) {
      return;
    }
    self->base_table = php_driver_create_table(self->schema,
                                                  table );
  }

  RETURN_ZVAL(&self->base_table, 1, 0);
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_name, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_default_materialized_view_methods[] = {
  PHP_ME(DefaultMaterializedView, name, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, option, arginfo_name, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, options, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, comment, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, readRepairChance, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, localReadRepairChance, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, gcGraceSeconds, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, caching, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, bloomFilterFPChance, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, memtableFlushPeriodMs, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, defaultTTL, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, speculativeRetry, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, indexInterval, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, compactionStrategyClassName, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, compactionStrategyOptions, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, compressionParameters, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, populateIOCacheOnFlush, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, replicateOnWrite, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, maxIndexInterval, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, minIndexInterval, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, column, arginfo_name, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, columns, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, partitionKey, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, primaryKey, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, clusteringKey, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, clusteringOrder, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultMaterializedView, baseTable, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static zend_object_handlers php_driver_default_materialized_view_handlers;

static HashTable *
php_driver_type_default_materialized_view_gc(
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
php_driver_default_materialized_view_properties(
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
php_driver_default_materialized_view_compare(zval *obj1, zval *obj2 )
{
#if PHP_MAJOR_VERSION >= 8
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
#endif
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj2);
}

static void
php_driver_default_materialized_view_free(zend_object *object )
{
  php_driver_materialized_view *self = PHP5TO7_ZEND_OBJECT_GET(materialized_view, object);

  zval_ptr_dtor(&self->name);
  zval_ptr_dtor(&self->options);
  zval_ptr_dtor(&self->partition_key);
  zval_ptr_dtor(&self->primary_key);
  zval_ptr_dtor(&self->clustering_key);
  zval_ptr_dtor(&self->clustering_order);
  zval_ptr_dtor(&self->base_table);

  if (self->schema) {
    php_driver_del_ref(&self->schema);
    self->schema = NULL;
  }
  self->meta = NULL;

  zend_object_std_dtor(&self->zendObject);

}

static zend_object*
php_driver_default_materialized_view_new(zend_class_entry *ce )
{
  php_driver_materialized_view *self =
      PHP5TO7_ZEND_OBJECT_ECALLOC(materialized_view, ce);

  ZVAL_UNDEF(&self->name);
  ZVAL_UNDEF(&self->options);
  ZVAL_UNDEF(&self->partition_key);
  ZVAL_UNDEF(&self->primary_key);
  ZVAL_UNDEF(&self->clustering_key);
  ZVAL_UNDEF(&self->clustering_order);
  ZVAL_UNDEF(&self->base_table);

  self->meta   = NULL;
  self->schema = NULL;

  PHP5TO7_ZEND_OBJECT_INIT_EX(materialized_view, default_materialized_view, self, ce);
}

void php_driver_define_DefaultMaterializedView()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\DefaultMaterializedView", php_driver_default_materialized_view_methods);
  php_driver_default_materialized_view_ce = zend_register_internal_class_ex(&ce, php_driver_materialized_view_ce);
  php_driver_default_materialized_view_ce->ce_flags     |= ZEND_ACC_FINAL;
  php_driver_default_materialized_view_ce->create_object = php_driver_default_materialized_view_new;

  memcpy(&php_driver_default_materialized_view_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_default_materialized_view_handlers.get_properties  = php_driver_default_materialized_view_properties;
#if PHP_VERSION_ID >= 50400
  php_driver_default_materialized_view_handlers.get_gc          = php_driver_type_default_materialized_view_gc;
#endif
#if PHP_MAJOR_VERSION >= 8
  php_driver_default_materialized_view_handlers.compare = php_driver_default_materialized_view_compare;
#else
  php_driver_default_materialized_view_handlers.compare_objects = php_driver_default_materialized_view_compare;
#endif
  php_driver_default_materialized_view_handlers.clone_obj = NULL;
}
END_EXTERN_C()