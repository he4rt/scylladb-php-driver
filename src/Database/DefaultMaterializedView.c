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

#include "DefaultColumn.h"
#include "DefaultIndex.h"
#include "DefaultTable.h"
#include "Table.h"

#include "DefaultMaterializedView_arginfo.h"

extern zend_object_handlers php_scylladb_default_materialized_view_handlers;
static void
populate_partition_key(php_scylladb_materialized_view *view, zval *result )
{
  size_t i, count = cass_materialized_view_meta_partition_key_count(view->meta);
  for (i = 0; i < count; ++i) {
    const CassColumnMeta *column =
        cass_materialized_view_meta_partition_key(view->meta, i);
    if (column) {
      zval zcolumn = php_scylladb_create_column(&view->schema, column );
      if (!Z_ISUNDEF(zcolumn)) {
        add_next_index_zval(result, &zcolumn);
      }
    }
  }
}

static void
populate_clustering_key(php_scylladb_materialized_view *view, zval *result )
{
  size_t i, count = cass_materialized_view_meta_clustering_key_count(view->meta);
  for (i = 0; i < count; ++i) {
    const CassColumnMeta *column =
        cass_materialized_view_meta_clustering_key(view->meta, i);
    if (column) {
      zval zcolumn = php_scylladb_create_column(&view->schema, column );
      if (!Z_ISUNDEF(zcolumn)) {
        add_next_index_zval(result, &zcolumn);
      }
    }
  }
}

zval
php_scylladb_create_materialized_view(zval *schema,
                                       const CassMaterializedViewMeta *meta )
{
  zval result;
  php_scylladb_materialized_view *view;
  const char *name;
  size_t name_length;

  ZVAL_UNDEF(&result);

  object_init_ex(&result, php_scylladb_default_materialized_view_ce);

  view = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(&result);
  ZVAL_COPY(&view->schema, schema);
  view->meta   = meta;

  cass_materialized_view_meta_name(meta, &name, &name_length);

  ZVAL_STRINGL(&view->name, name, name_length);

  return result;
}

void
php_scylladb_default_materialized_view_build_options(php_scylladb_materialized_view *view ) {
  CassIterator *iterator =
      cass_iterator_fields_from_materialized_view_meta(view->meta);
  view->options =
      php_scylladb_table_build_options(iterator );
  if (iterator) {
    cass_iterator_free(iterator);
  }
}

void
php_scylladb_materialized_view_get_option(php_scylladb_materialized_view *view,
                                           const char *name,
                                           zval *result ) {
  zval *zvalue;
  if (Z_ISUNDEF(view->options)) {
    php_scylladb_default_materialized_view_build_options(view );
  }

  if ((zvalue = zend_hash_str_find(Z_ARRVAL(view->options), name, strlen(name))) == nullptr) {
    ZVAL_FALSE(result);
    return;
  }

  ZVAL_COPY(result, zvalue);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, name)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  RETURN_ZVAL(&self->name, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, option)
{
  char *name;
  size_t name_len;
  php_scylladb_materialized_view *self;
  zval* result;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_default_materialized_view_build_options(self );
  }

  if ((result = zend_hash_str_find(Z_ARRVAL(self->options), name, name_len)) != nullptr) {
    RETURN_ZVAL(result, 1, 0);
  }
  RETURN_FALSE;
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, options)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_default_materialized_view_build_options(self );
  }

  RETURN_ZVAL(&self->options, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, comment)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "comment", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, readRepairChance)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "read_repair_chance", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, localReadRepairChance)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "local_read_repair_chance", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, gcGraceSeconds)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "gc_grace_seconds", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, caching)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "caching", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, bloomFilterFPChance)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "bloom_filter_fp_chance", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, memtableFlushPeriodMs)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "memtable_flush_period_in_ms", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, defaultTTL)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "default_time_to_live", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, speculativeRetry)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "speculative_retry", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, indexInterval)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "index_interval", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, compactionStrategyClassName)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "compaction_strategy_class", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, compactionStrategyOptions)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "compaction_strategy_options", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, compressionParameters)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "compression_parameters", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, populateIOCacheOnFlush)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "populate_io_cache_on_flush", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, replicateOnWrite)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "replicate_on_write", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, maxIndexInterval)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "max_index_interval", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, minIndexInterval)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());

  php_scylladb_materialized_view_get_option(self, "min_index_interval", return_value );
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, column)
{
  php_scylladb_materialized_view *self;
  char *name;
  size_t name_len;
  zval column;
  const CassColumnMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  meta = cass_materialized_view_meta_column_by_name(self->meta, name);
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  column = php_scylladb_create_column(&self->schema, meta );
  if (Z_ISUNDEF(column)) {
    return;
  }

  RETURN_ZVAL(&column, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, columns)
{
  php_scylladb_materialized_view *self;
  CassIterator    *iterator;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self     = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  iterator = cass_iterator_columns_from_materialized_view_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassColumnMeta *meta;
    zval zcolumn;
    php_scylladb_column *column;

    meta    = cass_iterator_get_column_meta(iterator);
    zcolumn = php_scylladb_create_column(&self->schema, meta );

    if (!Z_ISUNDEF(zcolumn)) {
      column = PHP_SCYLLADB_GET_COLUMN(&zcolumn);

      if (Z_TYPE(column->name) == IS_STRING) {
        add_assoc_zval_ex(return_value, Z_STRVAL(column->name), Z_STRLEN(column->name), &zcolumn);
      } else {
        add_next_index_zval(return_value, &zcolumn);
      }
    }
  }

  cass_iterator_free(iterator);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, partitionKey)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->partition_key)) {

    array_init(&self->partition_key);
    populate_partition_key(self, &self->partition_key );
  }

  RETURN_ZVAL(&self->partition_key, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, primaryKey)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->primary_key)) {

    array_init(&self->primary_key);
    populate_partition_key(self, &self->primary_key );
    populate_clustering_key(self, &self->primary_key );
  }

  RETURN_ZVAL(&self->primary_key, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, clusteringKey)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->clustering_key)) {

    array_init(&self->clustering_key);
    populate_clustering_key(self, &self->clustering_key );
  }

  RETURN_ZVAL(&self->clustering_key, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultMaterializedView, clusteringOrder)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
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

ZEND_METHOD(Cassandra_DefaultMaterializedView, baseTable)
{
  php_scylladb_materialized_view *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_MATERIALIZED_VIEW(getThis());
  if (Z_ISUNDEF(self->base_table)) {
    const CassTableMeta *table =
        cass_materialized_view_meta_base_table(self->meta);
    if (!table) {
      return;
    }
    self->base_table = php_scylladb_create_table(&self->schema,
                                                  table );
  }

  RETURN_ZVAL(&self->base_table, 1, 0);
}

HashTable *
php_scylladb_default_materialized_view_gc(zend_object *object, zval** table, int *n)
{
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *
php_scylladb_default_materialized_view_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

int
php_scylladb_default_materialized_view_compare(zval *obj1, zval *obj2 )
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_default_materialized_view_free(zend_object *object )
{
  auto self = php_scylladb_materialized_view_object_fetch(object);

  zval_ptr_dtor(&self->name);
  zval_ptr_dtor(&self->options);
  zval_ptr_dtor(&self->partition_key);
  zval_ptr_dtor(&self->primary_key);
  zval_ptr_dtor(&self->clustering_key);
  zval_ptr_dtor(&self->clustering_order);
  zval_ptr_dtor(&self->base_table);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = nullptr;

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_default_materialized_view_new(zend_class_entry *ce )
{
  php_scylladb_materialized_view *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_materialized_view, ce,
                                &php_scylladb_default_materialized_view_handlers);

  ZVAL_UNDEF(&self->name);
  ZVAL_UNDEF(&self->options);
  ZVAL_UNDEF(&self->partition_key);
  ZVAL_UNDEF(&self->primary_key);
  ZVAL_UNDEF(&self->clustering_key);
  ZVAL_UNDEF(&self->clustering_order);
  ZVAL_UNDEF(&self->base_table);

  self->meta   = nullptr;
  ZVAL_UNDEF(&self->schema);

  php_scylladb_default_materialized_view_handlers.offset = offsetof(php_scylladb_materialized_view, zendObject);
  php_scylladb_default_materialized_view_handlers.free_obj = php_scylladb_default_materialized_view_free;
  return &self->zendObject;
}
