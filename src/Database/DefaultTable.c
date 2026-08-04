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
#include "DefaultMaterializedView.h"
#include "DefaultIndex.h"
#include "Table.h"

#include "DefaultTable_arginfo.h"

extern zend_object_handlers php_scylladb_default_table_handlers;
static void
populate_partition_key(php_scylladb_table *table, zval *result )
{
  size_t i, count = cass_table_meta_partition_key_count(table->meta);
  for (i = 0; i < count; ++i) {
    const CassColumnMeta *column =
      cass_table_meta_partition_key(table->meta, i);
    if (column) {
      zval zcolumn = php_scylladb_create_column(&table->schema, column );
      if (!Z_ISUNDEF(zcolumn)) {
        add_next_index_zval(result, &zcolumn);
      }
    }
  }
}

static void
populate_clustering_key(php_scylladb_table *table, zval *result )
{
  size_t i, count = cass_table_meta_clustering_key_count(table->meta);
  for (i = 0; i < count; ++i) {
    const CassColumnMeta *column =
        cass_table_meta_clustering_key(table->meta, i);
    if (column) {
      zval zcolumn = php_scylladb_create_column(&table->schema, column );
      if (!Z_ISUNDEF(zcolumn)) {
        add_next_index_zval(result, &zcolumn);
      }
    }
  }
}

zval
php_scylladb_create_table(zval *schema,
                           const CassTableMeta *meta )
{
  zval result;
  php_scylladb_table *table;
  const char *name;
  size_t name_length;

  ZVAL_UNDEF(&result);

  object_init_ex(&result, php_scylladb_default_table_ce);

  table = PHP_SCYLLADB_GET_TABLE(&result);
  ZVAL_COPY(&table->schema, schema);
  table->meta   = meta;

  cass_table_meta_name(meta, &name, &name_length);

  ZVAL_STRINGL(&table->name, name, name_length);

  return result;
}

void
php_scylladb_default_table_build_options(php_scylladb_table *table ) {
  CassIterator *iterator =
      cass_iterator_fields_from_table_meta(table->meta);
  table->options = php_scylladb_table_build_options(iterator );
  if (iterator) {
    cass_iterator_free(iterator);
  }
}

void
php_scylladb_table_get_option(php_scylladb_table *table,
                               const char *name,
                               zval *result ) {
  zval *zvalue;
  if (Z_ISUNDEF(table->options)) {
    php_scylladb_default_table_build_options(table );
  }

  if ((zvalue = zend_hash_str_find(Z_ARRVAL(table->options), name, strlen(name))) == nullptr) {
    ZVAL_FALSE(result);
    return;
  }

  ZVAL_COPY(result, zvalue);
}

ZEND_METHOD(Cassandra_DefaultTable, name)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  RETURN_ZVAL(&self->name, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultTable, option)
{
  char *name;
  size_t name_len;
  php_scylladb_table *self;
  zval* result;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_default_table_build_options(self );
  }

  if ((result = zend_hash_str_find(Z_ARRVAL(self->options), name, name_len)) != nullptr) {
    RETURN_ZVAL(result, 1, 0);
  }
  RETURN_FALSE;
}

ZEND_METHOD(Cassandra_DefaultTable, options)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  if (Z_ISUNDEF(self->options)) {
    php_scylladb_default_table_build_options(self );
  }

  RETURN_ZVAL(&self->options, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultTable, comment)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "comment", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, readRepairChance)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "read_repair_chance", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, localReadRepairChance)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "local_read_repair_chance", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, gcGraceSeconds)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "gc_grace_seconds", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, caching)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "caching", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, bloomFilterFPChance)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "bloom_filter_fp_chance", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, memtableFlushPeriodMs)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "memtable_flush_period_in_ms", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, defaultTTL)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "default_time_to_live", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, speculativeRetry)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "speculative_retry", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, indexInterval)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "index_interval", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, compactionStrategyClassName)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "compaction_strategy_class", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, compactionStrategyOptions)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "compaction_strategy_options", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, compressionParameters)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "compression_parameters", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, populateIOCacheOnFlush)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "populate_io_cache_on_flush", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, replicateOnWrite)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "replicate_on_write", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, maxIndexInterval)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "max_index_interval", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, minIndexInterval)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());

  php_scylladb_table_get_option(self, "min_index_interval", return_value );
}

ZEND_METHOD(Cassandra_DefaultTable, column)
{
  php_scylladb_table *self;
  char *name;
  size_t name_len;
  zval column;
  const CassColumnMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  meta = cass_table_meta_column_by_name(self->meta, name);
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  column = php_scylladb_create_column(&self->schema, meta );

  if (Z_ISUNDEF(column)) {
    return;
  }

  RETURN_ZVAL(&column, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultTable, columns)
{
  php_scylladb_table *self;
  CassIterator    *iterator;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self     = PHP_SCYLLADB_GET_TABLE(getThis());
  iterator = cass_iterator_columns_from_table_meta(self->meta);

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

ZEND_METHOD(Cassandra_DefaultTable, partitionKey)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  if (Z_ISUNDEF(self->partition_key)) {

    array_init(&self->partition_key);
    populate_partition_key(self, &self->partition_key );
  }

  RETURN_ZVAL(&self->partition_key, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultTable, primaryKey)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  if (Z_ISUNDEF(self->primary_key)) {

    array_init(&self->primary_key);
    populate_partition_key(self, &self->primary_key );
    populate_clustering_key(self, &self->primary_key );
  }

  RETURN_ZVAL(&self->primary_key, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultTable, clusteringKey)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  if (Z_ISUNDEF(self->clustering_key)) {

    array_init(&self->clustering_key);
    populate_clustering_key(self, &self->clustering_key );
  }

  RETURN_ZVAL(&self->clustering_key, 1, 0);
}

ZEND_METHOD(Cassandra_DefaultTable, clusteringOrder)
{
  php_scylladb_table *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  if (Z_ISUNDEF(self->clustering_order)) {
    size_t i, count = cass_table_meta_clustering_key_count(self->meta);

    array_init(&self->clustering_order);
    for (i = 0; i < count; ++i) {
      CassClusteringOrder order =
          cass_table_meta_clustering_key_order(self->meta, i);
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

ZEND_METHOD(Cassandra_DefaultTable, index)
{
  php_scylladb_table *self;
  char *name;
  size_t name_len;
  zval index;
  const CassIndexMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  meta = cass_table_meta_index_by_name(self->meta, name);
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  index = php_scylladb_create_index(&self->schema, meta );
  if (Z_ISUNDEF(index)) {
    return;
  }

  RETURN_ZVAL(&index, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultTable, indexes)
{
  php_scylladb_table *self;
  CassIterator *iterator;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self     = PHP_SCYLLADB_GET_TABLE(getThis());
  iterator = cass_iterator_indexes_from_table_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassIndexMeta *meta;
    zval zindex;

    meta   = cass_iterator_get_index_meta(iterator);
    zindex = php_scylladb_create_index(&self->schema, meta );

    if (!Z_ISUNDEF(zindex)) {
      auto index = PHP_SCYLLADB_GET_INDEX(&zindex);

      if (Z_TYPE(index->name) == IS_STRING) {
        add_assoc_zval_ex(return_value, Z_STRVAL(index->name), Z_STRLEN(index->name), &zindex);
      } else {
        add_next_index_zval(return_value, &zindex);
      }
    }
  }

  cass_iterator_free(iterator);
}

ZEND_METHOD(Cassandra_DefaultTable, materializedView)
{
  php_scylladb_table *self;
  char *name;
  size_t name_len;
  zval zview;
  const CassMaterializedViewMeta *meta;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, name_len)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_TABLE(getThis());
  meta = cass_table_meta_materialized_view_by_name_n(self->meta,
                                                     name, name_len);
  if (meta == nullptr) {
    RETURN_FALSE;
  }

  zview = php_scylladb_create_materialized_view(&self->schema, meta );
  if (Z_ISUNDEF(zview)) {
    return;
  }

  RETURN_ZVAL(&zview, 0, 1);
}

ZEND_METHOD(Cassandra_DefaultTable, materializedViews)
{
  php_scylladb_table *self;
  CassIterator *iterator;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self     = PHP_SCYLLADB_GET_TABLE(getThis());
  iterator = cass_iterator_materialized_views_from_table_meta(self->meta);

  array_init(return_value);
  while (cass_iterator_next(iterator)) {
    const CassMaterializedViewMeta *meta;
    zval zview;
    php_scylladb_materialized_view *view;

    meta  = cass_iterator_get_materialized_view_meta(iterator);
    zview = php_scylladb_create_materialized_view(&self->schema, meta );

    if (!Z_ISUNDEF(zview)) {
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

HashTable *
php_scylladb_default_table_gc(zend_object *object, zval** table, int *n)
{
  *table = nullptr;
  *n = 0;
  return nullptr;
}

HashTable *
php_scylladb_default_table_properties(zend_object *object)
{
  HashTable *props = zend_std_get_properties(object );

  return props;
}

int
php_scylladb_default_table_compare(zval *obj1, zval *obj2 )
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void
php_scylladb_default_table_free(zend_object *object )
{
  auto self = php_scylladb_table_object_fetch(object);

  zval_ptr_dtor(&self->name);
  zval_ptr_dtor(&self->options);
  zval_ptr_dtor(&self->partition_key);
  zval_ptr_dtor(&self->primary_key);
  zval_ptr_dtor(&self->clustering_key);
  zval_ptr_dtor(&self->clustering_order);

  if (!Z_ISUNDEF(self->schema)) {
    zval_ptr_dtor(&self->schema);
    ZVAL_UNDEF(&self->schema);
  }
  self->meta = nullptr;

  zend_object_std_dtor(&self->zendObject);

}

zend_object*
php_scylladb_default_table_new(zend_class_entry *ce )
{
  php_scylladb_table *self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_table, ce, &php_scylladb_default_table_handlers);

  ZVAL_UNDEF(&self->name);
  ZVAL_UNDEF(&self->options);
  ZVAL_UNDEF(&self->partition_key);
  ZVAL_UNDEF(&self->primary_key);
  ZVAL_UNDEF(&self->clustering_key);
  ZVAL_UNDEF(&self->clustering_order);

  self->meta   = nullptr;
  ZVAL_UNDEF(&self->schema);

  php_scylladb_default_table_handlers.offset = offsetof(php_scylladb_table, zendObject);
  php_scylladb_default_table_handlers.free_obj = php_scylladb_default_table_free;
  return &self->zendObject;
}
