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

#ifndef PHP_DRIVER_TYPES_H
#define PHP_DRIVER_TYPES_H

#include <CassandraDriver.h>
#include <cassandra.h>

#include <TimestampGenerators/TimestampGenerators.h>

#include "src/Types/Numeric/Numeric.h"

#define PHP_DRIVER_GET_BLOB(obj) php_driver_blob_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TIMESTAMP(obj) php_driver_timestamp_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_DATE(obj) php_driver_date_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TIME(obj) php_driver_time_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_UUID(obj) php_driver_uuid_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_INET(obj) php_driver_inet_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_COLLECTION(obj) php_driver_collection_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_MAP(obj) php_driver_map_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SET(obj) php_driver_set_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TUPLE(obj) php_driver_tuple_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_USER_TYPE_VALUE(obj) php_driver_user_type_value_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_STATEMENT(obj) php_driver_statement_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_EXECUTION_OPTIONS(obj) php_driver_execution_options_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_ROWS(obj) php_driver_rows_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SESSION(obj) php_driver_session_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SSL(obj) php_driver_ssl_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SSL_BUILDER(obj) php_driver_ssl_builder_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SCHEMA(obj) php_driver_schema_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_KEYSPACE(obj) php_driver_keyspace_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TABLE(obj) php_driver_table_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_COLUMN(obj) php_driver_column_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_INDEX(obj) php_driver_index_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_MATERIALIZED_VIEW(obj) php_driver_materialized_view_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUNCTION(obj) php_driver_function_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_AGGREGATE(obj) php_driver_aggregate_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TYPE(obj) php_driver_type_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_DURATION(obj) php_driver_duration_object_fetch(Z_OBJ_P(obj))

typedef struct php_driver_timestamp_ {
  cass_int64_t timestamp;
  zend_object zval;
} php_driver_timestamp;
static zend_always_inline
  php_driver_timestamp*
  php_driver_timestamp_object_fetch(zend_object* obj)
{
  return (php_driver_timestamp*) ((char*) obj - XtOffsetOf(php_driver_timestamp, zval));
}

typedef struct php_driver_date_ {
  cass_uint32_t date;
  zend_object zval;
} php_driver_date;
static zend_always_inline php_driver_date*
php_driver_date_object_fetch(zend_object* obj)
{
  return (php_driver_date*) ((char*) obj - XtOffsetOf(php_driver_date, zval));
}

typedef struct php_driver_time_ {
  cass_int64_t time;
  zend_object zval;
} php_driver_time;

static zend_always_inline
  php_driver_time*
  php_driver_time_object_fetch(zend_object* obj)
{
  return (php_driver_time*) ((char*) obj - XtOffsetOf(php_driver_time, zval));
}

typedef struct php_driver_blob_ {
  cass_byte_t* data;
  size_t size;
  zend_object zval;
} php_driver_blob;
static zend_always_inline
  php_driver_blob*
  php_driver_blob_object_fetch(zend_object* obj)
{
  return (php_driver_blob*) ((char*) obj - XtOffsetOf(php_driver_blob, zval));
}

typedef struct php_driver_uuid_ {
  CassUuid uuid;
  zend_object zval;
} php_driver_uuid;
static zend_always_inline
  php_driver_uuid*
  php_driver_uuid_object_fetch(zend_object* obj)
{
  return (php_driver_uuid*) ((char*) obj - XtOffsetOf(php_driver_uuid, zval));
}

typedef struct php_driver_inet_ {
  CassInet inet;
  zend_object zval;
} php_driver_inet;
static zend_always_inline
  php_driver_inet*
  php_driver_inet_object_fetch(zend_object* obj)
{
  return (php_driver_inet*) ((char*) obj - XtOffsetOf(php_driver_inet, zval));
}

typedef struct php_driver_duration_ {
  cass_int32_t months;
  cass_int32_t days;
  cass_int64_t nanos;
  zend_object zval;
} php_driver_duration;
static zend_always_inline
  php_driver_duration*
  php_driver_duration_object_fetch(zend_object* obj)
{
  return (php_driver_duration*) ((char*) obj - XtOffsetOf(php_driver_duration, zval));
}

typedef struct php_driver_collection_ {
  zval type;
  HashTable values;
  unsigned hashv;
  int dirty;
  zend_object zval;
} php_driver_collection;
static zend_always_inline
  php_driver_collection*
  php_driver_collection_object_fetch(zend_object* obj)
{
  return (php_driver_collection*) ((char*) obj - XtOffsetOf(php_driver_collection, zval));
}

typedef struct php_driver_map_entry_ php_driver_map_entry;

typedef struct php_driver_map_ {
  zval type;
  php_driver_map_entry* entries;
  unsigned hashv;
  int dirty;
  php_driver_map_entry* iter_curr;
  php_driver_map_entry* iter_temp;
  zend_object zval;
} php_driver_map;
static zend_always_inline
  php_driver_map*
  php_driver_map_object_fetch(zend_object* obj)
{
  return (php_driver_map*) ((char*) obj - XtOffsetOf(php_driver_map, zval));
}

typedef struct php_driver_set_entry_ php_driver_set_entry;

typedef struct php_driver_set_ {
  zval type;
  php_driver_set_entry* entries;
  unsigned hashv;
  int dirty;
  php_driver_set_entry* iter_curr;
  php_driver_set_entry* iter_temp;
  int iter_index;
  zend_object zval;
} php_driver_set;
static zend_always_inline
  php_driver_set*
  php_driver_set_object_fetch(zend_object* obj)
{
  return (php_driver_set*) ((char*) obj - XtOffsetOf(php_driver_set, zval));
}

typedef struct php_driver_tuple_ {
  zval type;
  HashTable values;
  HashPosition pos;
  unsigned hashv;
  int dirty;
  zend_object zval;
} php_driver_tuple;
static zend_always_inline
  php_driver_tuple*
  php_driver_tuple_object_fetch(zend_object* obj)
{
  return (php_driver_tuple*) ((char*) obj - XtOffsetOf(php_driver_tuple, zval));
}

typedef struct php_driver_user_type_value_ {
  zval type;
  HashTable values;
  HashPosition pos;
  unsigned hashv;
  int dirty;
  zend_object zval;
} php_driver_user_type_value;
static zend_always_inline
  php_driver_user_type_value*
  php_driver_user_type_value_object_fetch(zend_object* obj)
{
  return (php_driver_user_type_value*) ((char*) obj - XtOffsetOf(php_driver_user_type_value, zval));
}

typedef enum {
  PHP_DRIVER_SIMPLE_STATEMENT,
  PHP_DRIVER_PREPARED_STATEMENT,
  PHP_DRIVER_BATCH_STATEMENT
} php_driver_statement_type;

typedef struct php_driver_statement_ {
  php_driver_statement_type type;
  union {
    struct
    {
      char* cql;
    } simple;
    struct
    {
      const CassPrepared* prepared;
    } prepared;
    struct
    {
      CassBatchType type;
      HashTable statements;
    } batch;
  } data;
  zend_object zval;
} php_driver_statement;
static zend_always_inline
  php_driver_statement*
  php_driver_statement_object_fetch(zend_object* obj)
{
  return (php_driver_statement*) ((char*) obj - XtOffsetOf(php_driver_statement, zval));
}

typedef struct
{
  zval statement;
  zval arguments;
} php_driver_batch_statement_entry;

typedef struct php_driver_execution_options_ {
  long consistency;
  long serial_consistency;
  int page_size;
  char* paging_state_token;
  size_t paging_state_token_size;
  zval timeout;
  zval arguments;
  zval retry_policy;
  cass_int64_t timestamp;
  zend_object zval;
} php_driver_execution_options;
static zend_always_inline
  php_driver_execution_options*
  php_driver_execution_options_object_fetch(zend_object* obj)
{
  return (php_driver_execution_options*) ((char*) obj - XtOffsetOf(php_driver_execution_options, zval));
}

typedef struct php_driver_rows_ {
  php_driver_ref* statement;
  php_driver_ref* session;
  zval rows;
  zval next_rows;
  php_driver_ref* result;
  php_driver_ref* next_result;
  zval future_next_page;
  zend_object zval;
} php_driver_rows;

static zend_always_inline
  php_driver_rows*
  php_driver_rows_object_fetch(zend_object* obj)
{
  return (php_driver_rows*) ((char*) obj - XtOffsetOf(php_driver_rows, zval));
}

typedef struct
{
  CassFuture* future;
  php_driver_ref* session;
} php_driver_psession;

typedef struct
{
  CassFuture* future;
  php_driver_ref* ref;
} php_driver_pprepared_statement;

typedef struct php_driver_session_ {
  php_driver_ref* session;
  long default_consistency;
  int default_page_size;
  char* keyspace;
  char* hash_key;
  zval default_timeout;
  cass_bool_t persist;
  zend_object zval;
} php_driver_session;
static zend_always_inline
  php_driver_session*
  php_driver_session_object_fetch(zend_object* obj)
{
  return (php_driver_session*) ((char*) obj - XtOffsetOf(php_driver_session, zval));
}

typedef struct php_driver_ssl_ {
  CassSsl* ssl;
  zend_object zval;
} php_driver_ssl;
static zend_always_inline
  php_driver_ssl*
  php_driver_ssl_object_fetch(zend_object* obj)
{
  return (php_driver_ssl*) ((char*) obj - XtOffsetOf(php_driver_ssl, zval));
}

typedef struct php_driver_ssl_builder_ {
  int flags;
  char** trusted_certs;
  int trusted_certs_cnt;
  char* client_cert;
  char* private_key;
  char* passphrase;
  zend_object zval;
} php_driver_ssl_builder;
static zend_always_inline
  php_driver_ssl_builder*
  php_driver_ssl_builder_object_fetch(zend_object* obj)
{
  return (php_driver_ssl_builder*) ((char*) obj - XtOffsetOf(php_driver_ssl_builder, zval));
}

typedef struct php_driver_schema_ {
  php_driver_ref* schema;
  zend_object zval;
} php_driver_schema;
static zend_always_inline
  php_driver_schema*
  php_driver_schema_object_fetch(zend_object* obj)
{
  return (php_driver_schema*) ((char*) obj - XtOffsetOf(php_driver_schema, zval));
}

typedef struct php_driver_keyspace_ {
  php_driver_ref* schema;
  const CassKeyspaceMeta* meta;
  zend_object zval;
} php_driver_keyspace;
static zend_always_inline
  php_driver_keyspace*
  php_driver_keyspace_object_fetch(zend_object* obj)
{
  return (php_driver_keyspace*) ((char*) obj - XtOffsetOf(php_driver_keyspace, zval));
}

typedef struct php_driver_table_ {
  zval name;
  zval options;
  zval partition_key;
  zval primary_key;
  zval clustering_key;
  zval clustering_order;
  php_driver_ref* schema;
  const CassTableMeta* meta;
  zend_object zval;
} php_driver_table;
static zend_always_inline
  php_driver_table*
  php_driver_table_object_fetch(zend_object* obj)
{
  return (php_driver_table*) ((char*) obj - XtOffsetOf(php_driver_table, zval));
}

typedef struct php_driver_materialized_view_ {
  zval name;
  zval options;
  zval partition_key;
  zval primary_key;
  zval clustering_key;
  zval clustering_order;
  zval base_table;
  php_driver_ref* schema;
  const CassMaterializedViewMeta* meta;
  zend_object zval;
} php_driver_materialized_view;
static zend_always_inline
  php_driver_materialized_view*
  php_driver_materialized_view_object_fetch(zend_object* obj)
{
  return (php_driver_materialized_view*) ((char*) obj - XtOffsetOf(php_driver_materialized_view, zval));
}

typedef struct php_driver_column_ {
  zval name;
  zval type;
  int reversed;
  int frozen;
  php_driver_ref* schema;
  const CassColumnMeta* meta;
  zend_object zval;
} php_driver_column;
static zend_always_inline
  php_driver_column*
  php_driver_column_object_fetch(zend_object* obj)
{
  return (php_driver_column*) ((char*) obj - XtOffsetOf(php_driver_column, zval));
}

typedef struct php_driver_index_ {
  zval name;
  zval kind;
  zval target;
  zval options;
  php_driver_ref* schema;
  const CassIndexMeta* meta;
  zend_object zval;
} php_driver_index;
static zend_always_inline
  php_driver_index*
  php_driver_index_object_fetch(zend_object* obj)
{
  return (php_driver_index*) ((char*) obj - XtOffsetOf(php_driver_index, zval));
}

typedef struct php_driver_function_ {
  zval simple_name;
  zval arguments;
  zval return_type;
  zval signature;
  zval language;
  zval body;
  php_driver_ref* schema;
  const CassFunctionMeta* meta;
  zend_object zval;
} php_driver_function;
static zend_always_inline
  php_driver_function*
  php_driver_function_object_fetch(zend_object* obj)
{
  return (php_driver_function*) ((char*) obj - XtOffsetOf(php_driver_function, zval));
}

typedef struct php_driver_aggregate_ {
  zval simple_name;
  zval argument_types;
  zval state_function;
  zval final_function;
  zval initial_condition;
  zval state_type;
  zval return_type;
  zval signature;
  php_driver_ref* schema;
  const CassAggregateMeta* meta;
  zend_object zval;
} php_driver_aggregate;
static zend_always_inline
  php_driver_aggregate*
  php_driver_aggregate_object_fetch(zend_object* obj)
{
  return (php_driver_aggregate*) ((char*) obj - XtOffsetOf(php_driver_aggregate, zval));
}

typedef struct php_driver_type_ {
  CassValueType type;
  CassDataType* data_type;
  union {
    struct
    {
      zval value_type;
    } collection;
    struct
    {
      zval value_type;
    } set;
    struct
    {
      zval key_type;
      zval value_type;
    } map;
    struct
    {
      char* class_name;
    } custom;
    struct
    {
      char* keyspace;
      char* type_name;
      HashTable types;
    } udt;
    struct
    {
      HashTable types;
    } tuple;
  } data;
  zend_object zval;
} php_driver_type;
static zend_always_inline
  php_driver_type*
  php_driver_type_object_fetch(zend_object* obj)
{
  return (php_driver_type*) ((char*) obj - XtOffsetOf(php_driver_type, zval));
}

typedef unsigned (*php_driver_value_hash_t)(zval* obj);

typedef struct
{
  zend_object_handlers std;
  php_driver_value_hash_t hash_value;
} php_driver_value_handlers;

extern PHP_DRIVER_API zend_class_entry* php_driver_value_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_blob_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_inet_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_timestamp_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_date_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_time_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_uuid_interface_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_uuid_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_timeuuid_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_duration_ce;

extern PHP_DRIVER_API zend_class_entry* php_driver_set_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_map_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_collection_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_tuple_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_user_type_value_ce;

/* Types */

void php_driver_define_Blob();
void php_driver_define_Collection();

void php_driver_define_Inet();
void php_driver_define_Map();
void php_driver_define_Set();
void php_driver_define_Timestamp();
void php_driver_define_Date();
void php_driver_define_Time();
void php_driver_define_Tuple();
void php_driver_define_UserTypeValue();
void php_driver_define_UuidInterface();
void php_driver_define_Uuid();
void php_driver_define_Timeuuid();
void php_driver_define_Duration();

/* Classes */
extern PHP_DRIVER_API zend_class_entry* php_driver_core_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_ssl_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_ssl_builder_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_session_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_session_ce;

extern PHP_DRIVER_API zend_class_entry* php_driver_statement_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_simple_statement_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_prepared_statement_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_batch_statement_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_execution_options_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_rows_ce;

void php_driver_define_Core();
void php_driver_define_Session();
void php_driver_define_DefaultSession();
void php_driver_define_SSLOptions();
void php_driver_define_SSLOptionsBuilder();
void php_driver_define_Statement();
void php_driver_define_SimpleStatement();
void php_driver_define_PreparedStatement();
void php_driver_define_BatchStatement();
void php_driver_define_ExecutionOptions();
void php_driver_define_Rows();

extern PHP_DRIVER_API zend_class_entry* php_driver_schema_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_schema_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_keyspace_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_keyspace_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_table_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_table_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_column_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_column_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_index_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_index_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_materialized_view_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_materialized_view_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_function_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_function_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_aggregate_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_aggregate_ce;

void php_driver_define_Schema();
void php_driver_define_DefaultSchema();
void php_driver_define_Keyspace();
void php_driver_define_DefaultKeyspace();
void php_driver_define_Table();
void php_driver_define_DefaultTable();
void php_driver_define_Column();
void php_driver_define_DefaultColumn();
void php_driver_define_Index();
void php_driver_define_DefaultIndex();
void php_driver_define_MaterializedView();
void php_driver_define_DefaultMaterializedView();
void php_driver_define_Function();
void php_driver_define_DefaultFunction();
void php_driver_define_Aggregate();
void php_driver_define_DefaultAggregate();

extern PHP_DRIVER_API zend_class_entry* php_driver_type_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_scalar_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_collection_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_set_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_map_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_tuple_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_user_type_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_type_custom_ce;

void php_driver_define_Type();
void php_driver_define_TypeScalar();
void php_driver_define_TypeCollection();
void php_driver_define_TypeSet();
void php_driver_define_TypeMap();
void php_driver_define_TypeTuple();
void php_driver_define_TypeUserType();
void php_driver_define_TypeCustom();

void PhpDriverDefineRetryPolicy();

extern int php_le_php_driver_cluster();
extern int php_le_php_driver_session();
extern int php_le_php_driver_prepared_statement();

#endif /* PHP_DRIVER_TYPES_H */
