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

#pragma once

#include <php.h>
#include <php_scylladb.h>
#include <php_scylladb_object.h>

#include <DateTime/Date.h>
#include <RetryPolicy/RetryPolicy.h>
#include "SSLOptions/SSLOptions.h"

BEGIN_EXTERN_C()

/* Event-loop completion notifier (see FutureNotifier.h). Forward-declared here
   so the Future structs can hold a pointer without a hard include.

   MUST stay a heap pointer (never embed by value), regardless of how small the
   struct is: it is reference-counted and persistent-malloc'd so it can OUTLIVE
   the Future object. The cpp-driver completion callback runs on an IO thread and
   may fire after this (emalloc'd, main-thread-freed) Future is destroyed;
   cass_future_free() does not cancel it. Embedding the notifier would let that
   late callback write into freed memory — a cross-thread use-after-free. */
typedef struct php_scylladb_notifier_ php_scylladb_notifier;

/* Shared reactor registration (see src/Async/Reactor.c). Forward-declared so
   FutureRows can hold a pointer without a hard include. */
typedef struct php_scylladb_reg_ php_scylladb_reg;

#define PHP_SCYLLADB_GET_NUMERIC(obj) php_scylladb_numeric_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_BLOB(obj) php_scylladb_blob_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_UUID(obj) php_scylladb_uuid_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_INET(obj) php_scylladb_inet_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_COLLECTION(obj) php_scylladb_collection_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_MAP(obj) php_scylladb_map_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_SET(obj) php_scylladb_set_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_TUPLE(obj) php_scylladb_tuple_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_USER_TYPE_VALUE(obj) php_scylladb_user_type_value_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_CLUSTER(obj) php_scylladb_cluster_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_STATEMENT(obj) php_scylladb_statement_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_EXECUTION_OPTIONS(obj) php_scylladb_execution_options_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_ROWS(obj) php_scylladb_rows_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_FUTURE_ROWS(obj) php_scylladb_future_rows_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_CLUSTER_BUILDER(obj) php_scylladb_cluster_builder_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_FUTURE_PREPARED_STATEMENT(obj) php_scylladb_future_prepared_statement_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_FUTURE_VALUE(obj) php_scylladb_future_value_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_FUTURE_CLOSE(obj) php_scylladb_future_close_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_FUTURE_SESSION(obj) php_scylladb_future_session_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_SESSION(obj) php_scylladb_session_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_SCHEMA(obj) php_scylladb_schema_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_KEYSPACE(obj) php_scylladb_keyspace_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_TABLE(obj) php_scylladb_table_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_COLUMN(obj) php_scylladb_column_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_INDEX(obj) php_scylladb_index_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_MATERIALIZED_VIEW(obj) php_scylladb_materialized_view_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_FUNCTION(obj) php_scylladb_function_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_AGGREGATE(obj) php_scylladb_aggregate_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_TYPE(obj) php_scylladb_type_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_TIMESTAMP_GEN(obj) php_scylladb_timestamp_gen_object_fetch(Z_OBJ_P(obj))
#define PHP_SCYLLADB_GET_DURATION(obj) php_scylladb_duration_object_fetch(Z_OBJ_P(obj))

typedef enum : uint8_t
{
    PHP_SCYLLADB_BIGINT,
    PHP_SCYLLADB_DECIMAL,
    PHP_SCYLLADB_FLOAT,
    PHP_SCYLLADB_VARINT,
    PHP_SCYLLADB_SMALLINT,
    PHP_SCYLLADB_TINYINT
} php_scylladb_numeric_type;

typedef struct php_scylladb_numeric_
{
    php_scylladb_numeric_type type;
    union {
        struct
        {
            cass_int8_t value;
        } tinyint;
        struct
        {
            cass_int16_t value;
        } smallint;
        struct
        {
            cass_int64_t value;
        } bigint;
        struct
        {
            cass_float_t value;
        } floating;
        struct
        {
            mpz_t value;
        } varint;
        struct
        {
            mpz_t value;
            long scale;
        } decimal;
    } data;
    zend_object zendObject;
} php_scylladb_numeric;
static zend_always_inline php_scylladb_numeric *php_scylladb_numeric_object_fetch(zend_object *obj)
{
    return (php_scylladb_numeric *)((char *)obj - ((size_t)(&(((php_scylladb_numeric *)0)->zendObject))));
}


typedef struct php_scylladb_blob_
{
    cass_byte_t *data;
    size_t size;
    zend_object zendObject;
} php_scylladb_blob;
static zend_always_inline php_scylladb_blob *php_scylladb_blob_object_fetch(zend_object *obj)
{
    return (php_scylladb_blob *)((char *)obj - ((size_t)(&(((php_scylladb_blob *)0)->zendObject))));
}

typedef struct php_scylladb_uuid_
{
    CassUuid uuid;
    zend_object zendObject;
} php_scylladb_uuid;
static zend_always_inline php_scylladb_uuid *php_scylladb_uuid_object_fetch(zend_object *obj)
{
    return (php_scylladb_uuid *)((char *)obj - ((size_t)(&(((php_scylladb_uuid *)0)->zendObject))));
}

typedef struct php_scylladb_inet_
{
    CassInet inet;
    zend_object zendObject;
} php_scylladb_inet;
static zend_always_inline php_scylladb_inet *php_scylladb_inet_object_fetch(zend_object *obj)
{
    return (php_scylladb_inet *)((char *)obj - ((size_t)(&(((php_scylladb_inet *)0)->zendObject))));
}

typedef struct php_scylladb_duration_
{
    cass_int32_t months;
    cass_int32_t days;
    cass_int64_t nanos;
    zend_object zendObject;
} php_scylladb_duration;
static zend_always_inline php_scylladb_duration *php_scylladb_duration_object_fetch(zend_object *obj)
{
    return (php_scylladb_duration *)((char *)obj - ((size_t)(&(((php_scylladb_duration *)0)->zendObject))));
}

typedef struct php_scylladb_collection_
{
    zval type;
    HashTable values;
    unsigned hashv;
    int dirty;
    zend_object zendObject;
} php_scylladb_collection;
static zend_always_inline php_scylladb_collection *php_scylladb_collection_object_fetch(zend_object *obj)
{
    return (php_scylladb_collection *)((char *)obj - ((size_t)(&(((php_scylladb_collection *)0)->zendObject))));
}

typedef struct php_scylladb_map_entry_ php_scylladb_map_entry;

typedef struct php_scylladb_map_
{
    zval type;
    php_scylladb_map_entry *entries;
    unsigned hashv;
    int dirty;
    php_scylladb_map_entry *iter_curr;
    php_scylladb_map_entry *iter_temp;
    zend_object zendObject;
} php_scylladb_map;
static zend_always_inline php_scylladb_map *php_scylladb_map_object_fetch(zend_object *obj)
{
    return (php_scylladb_map *)((char *)obj - ((size_t)(&(((php_scylladb_map *)0)->zendObject))));
}

typedef struct php_scylladb_set_entry_ php_scylladb_set_entry;

typedef struct php_scylladb_set_
{
    zval type;
    php_scylladb_set_entry *entries;
    unsigned hashv;
    int dirty;
    php_scylladb_set_entry *iter_curr;
    php_scylladb_set_entry *iter_temp;
    int iter_index;
    zend_object zendObject;
} php_scylladb_set;
static zend_always_inline php_scylladb_set *php_scylladb_set_object_fetch(zend_object *obj)
{
    return (php_scylladb_set *)((char *)obj - ((size_t)(&(((php_scylladb_set *)0)->zendObject))));
}

typedef struct php_scylladb_tuple_
{
    zval type;
    HashTable values;
    HashPosition pos;
    unsigned hashv;
    int dirty;
    zend_object zendObject;
} php_scylladb_tuple;
static zend_always_inline php_scylladb_tuple *php_scylladb_tuple_object_fetch(zend_object *obj)
{
    return (php_scylladb_tuple *)((char *)obj - ((size_t)(&(((php_scylladb_tuple *)0)->zendObject))));
}

typedef struct php_scylladb_user_type_value_
{
    zval type;
    HashTable values;
    HashPosition pos;
    unsigned hashv;
    int dirty;
    zend_object zendObject;
} php_scylladb_user_type_value;
static zend_always_inline php_scylladb_user_type_value *php_scylladb_user_type_value_object_fetch(zend_object *obj)
{
    return (php_scylladb_user_type_value *)((char *)obj - ((size_t)(&(((php_scylladb_user_type_value *)0)->zendObject))));
}

typedef struct php_scylladb_cluster_
{
    cass_byte_t *data;
    CassCluster *cluster;
    uint32_t default_consistency;
    uint32_t default_page_size;
    zval default_timeout;
    cass_bool_t persist;
    zend_ulong cache_key;
    zend_object zendObject;
} php_scylladb_cluster;
static zend_always_inline php_scylladb_cluster *php_scylladb_cluster_object_fetch(zend_object *obj)
{
    return (php_scylladb_cluster *)((char *)obj - offsetof(php_scylladb_cluster, zendObject));
}

typedef enum
{
    PHP_SCYLLADB_SIMPLE_STATEMENT,
    PHP_SCYLLADB_PREPARED_STATEMENT,
    PHP_SCYLLADB_BATCH_STATEMENT
} php_scylladb_statement_type;

typedef struct php_scylladb_statement_
{
    php_scylladb_statement_type type;
    union {
        struct
        {
            char *cql;
        } simple;
        struct
        {
            const CassPrepared *prepared;
        } prepared;
        struct
        {
            CassBatchType type;
            HashTable statements;
        } batch;
    } data;
    zend_object zendObject;
} php_scylladb_statement;
static zend_always_inline php_scylladb_statement *php_scylladb_statement_object_fetch(zend_object *obj)
{
    return (php_scylladb_statement *)((char *)obj - ((size_t)(&(((php_scylladb_statement *)0)->zendObject))));
}

typedef struct
{
    zval statement;
    zval arguments;
} php_scylladb_batch_statement_entry;

typedef struct php_scylladb_execution_options_
{
    long consistency;
    long serial_consistency;
    int page_size;
    char *paging_state_token;
    size_t paging_state_token_size;
    zval timeout;
    zval arguments;
    zval retry_policy;
    cass_int64_t timestamp;
    zend_object zendObject;
} php_scylladb_execution_options;
static zend_always_inline php_scylladb_execution_options *php_scylladb_execution_options_object_fetch(zend_object *obj)
{
    return (php_scylladb_execution_options *)((char *)obj - ((size_t)(&(((php_scylladb_execution_options *)0)->zendObject))));
}

typedef enum /* : uint8_t */
{
    LOAD_BALANCING_DEFAULT = 0,
    LOAD_BALANCING_ROUND_ROBIN,
    LOAD_BALANCING_DC_AWARE_ROUND_ROBIN
} php_scylladb_load_balancing;

typedef struct php_scylladb_rows_
{
    zend_resource *statement;       /* le_cass_statement; refcounted */
    zval session;                   /* zval to DefaultSession PHP object */
    zval rows;
    zval next_rows;
    const CassResult *result;       /* owned; freed in free_obj */
    const CassResult *next_result;  /* owned; freed in free_obj */
    zval future_next_page;
    /* Iterator cursor. Private to this object: `rows` is a refcounted copy of
       the array the producing FutureRows caches, so the array's own internal
       pointer is shared between every Rows built from one future. */
    HashPosition pos;
    zend_object zendObject;
} php_scylladb_rows;
static zend_always_inline php_scylladb_rows *php_scylladb_rows_object_fetch(zend_object *obj)
{
    return (php_scylladb_rows *)((char *)obj - ((size_t)(&(((php_scylladb_rows *)0)->zendObject))));
}

typedef struct php_scylladb_future_rows_
{
    zend_resource *statement;       /* le_cass_statement; refcounted */
    zval session;                   /* zval to DefaultSession PHP object */
    zval rows;
    const CassResult *result;       /* owned; freed in free_obj */
    CassFuture *future;
    php_scylladb_notifier *notifier; /* async: owns write fd; lazy */
    zval notify_stream;              /* async: cached readable php_stream */
    php_scylladb_reg *reactor_reg;   /* async: shared-reactor registration; null unless added */
    zend_object zendObject;
} php_scylladb_future_rows;
static zend_always_inline php_scylladb_future_rows *php_scylladb_future_rows_object_fetch(zend_object *obj)
{
    return (php_scylladb_future_rows *)((char *)obj - ((size_t)(&(((php_scylladb_future_rows *)0)->zendObject))));
}


typedef struct php_scylladb_timestamp_gen_
{
    CassTimestampGen *gen;
    zend_object zendObject;
} php_scylladb_timestamp_gen;
static zend_always_inline php_scylladb_timestamp_gen *php_scylladb_timestamp_gen_object_fetch(zend_object *obj)
{
    return (php_scylladb_timestamp_gen *)((char *)obj - ((size_t)(&(((php_scylladb_timestamp_gen *)0)->zendObject))));
}

typedef struct php_scylladb_cluster_builder_
{
    zend_string *contact_points;
    zend_string *username;
    zend_string *password;
    zend_string *local_dc;
    zend_string *blacklist_hosts;
    zend_string *whitelist_hosts;
    zend_string *blacklist_dcs;
    zend_string *whitelist_dcs;

    uint32_t used_hosts_per_remote_dc;
    uint32_t connect_timeout;
    uint32_t default_consistency;
    uint32_t default_page_size;
    uint32_t protocol_version;
    uint32_t io_threads;
    uint32_t core_connections_per_host;
    uint32_t max_connections_per_host;
    uint32_t reconnect_interval;
    uint32_t tcp_keepalive_delay;
    cass_bool_t enable_latency_aware_routing;
    cass_bool_t enable_tcp_nodelay;
    cass_bool_t enable_tcp_keepalive;
    cass_bool_t enable_schema;
    cass_bool_t enable_hostname_resolution;
    cass_bool_t enable_randomized_contact_points;
    cass_bool_t allow_remote_dcs_for_local_cl;
    cass_bool_t use_token_aware_routing;
    uint32_t connection_heartbeat_interval;
    uint32_t request_timeout;
    uint16_t port;
    php_scylladb_load_balancing load_balancing_policy;
    cass_bool_t persist;
    php_scylladb_retry_policy *retry_policy;
    php_scylladb_timestamp_gen *timestamp_gen;
    php_scylladb_ssl *ssl_options;
    zval default_timeout;

    zend_object zendObject;

} php_scylladb_cluster_builder;
static zend_always_inline php_scylladb_cluster_builder *php_scylladb_cluster_builder_object_fetch(zend_object *obj)
{
    return (php_scylladb_cluster_builder *)((char *)obj - offsetof(php_scylladb_cluster_builder, zendObject));
}

typedef struct php_scylladb_future_prepared_statement_
{
    CassFuture *future;
    zval prepared_statement;
    php_scylladb_notifier *notifier; /* async: owns write fd; lazy */
    zval notify_stream;              /* async: cached readable php_stream */
    zend_object zendObject;
} php_scylladb_future_prepared_statement;
static zend_always_inline php_scylladb_future_prepared_statement *php_scylladb_future_prepared_statement_object_fetch(
    zend_object *obj)
{
    return (php_scylladb_future_prepared_statement *)((char *)obj -
                                                    ((size_t)(&(((php_scylladb_future_prepared_statement *)0)->zendObject))));
}

typedef struct php_scylladb_future_value_
{
    zval value;
    php_scylladb_notifier *notifier; /* async: owns write fd; lazy */
    zval notify_stream;              /* async: cached eagerly-readable php_stream */
    zend_object zendObject;
} php_scylladb_future_value;
static zend_always_inline php_scylladb_future_value *php_scylladb_future_value_object_fetch(zend_object *obj)
{
    return (php_scylladb_future_value *)((char *)obj - ((size_t)(&(((php_scylladb_future_value *)0)->zendObject))));
}

typedef struct php_scylladb_future_close_
{
    CassFuture *future;
    php_scylladb_notifier *notifier; /* async: owns write fd; lazy */
    zval notify_stream;              /* async: cached readable php_stream */
    zend_object zendObject;
} php_scylladb_future_close;
static zend_always_inline php_scylladb_future_close *php_scylladb_future_close_object_fetch(zend_object *obj)
{
    return (php_scylladb_future_close *)((char *)obj - ((size_t)(&(((php_scylladb_future_close *)0)->zendObject))));
}

typedef struct php_scylladb_future_session_
{
    CassFuture *future;
    CassSession *session;     /* owned until get() transfers to DefaultSession */
    zval default_session;
    cass_bool_t persist;
    zend_ulong cache_key;
    char *exception_message;
    CassError exception_code;
    char *session_keyspace;
    php_scylladb_notifier *notifier; /* async: owns write fd; lazy */
    zval notify_stream;              /* async: cached readable php_stream */
    zend_object zendObject;
} php_scylladb_future_session;
static zend_always_inline php_scylladb_future_session *php_scylladb_future_session_object_fetch(zend_object *obj)
{
    return (php_scylladb_future_session *)((char *)obj - ((size_t)(&(((php_scylladb_future_session *)0)->zendObject))));
}

typedef struct
{
    CassFuture *future;
    CassSession *session;   /* owns CassSession across requests (persistent_list) */
} php_scylladb_psession;

typedef struct
{
    CassFuture *future;
    /* No back-ref to session: persistent_list cleanup is LIFO so the
       prepared-statement entry is destroyed before the psession it
       depends on; CassFuture holds an internal CassSession ref. */
} php_scylladb_pprepared_statement;

typedef struct php_scylladb_session_
{
    CassSession *session;        /* owned if !persist; borrowed (psession owns) otherwise */
    long default_consistency;
    int default_page_size;
    char *keyspace;
    zend_ulong cache_key;
    zval default_timeout;
    cass_bool_t persist;
    zend_object zendObject;
} php_scylladb_session;
static zend_always_inline php_scylladb_session *php_scylladb_session_object_fetch(zend_object *obj)
{
    return (php_scylladb_session *)((char *)obj - offsetof(php_scylladb_session, zendObject));
}

typedef struct php_scylladb_schema_
{
    const CassSchemaMeta *schema_meta;
    zend_object zendObject;
} php_scylladb_schema;
static zend_always_inline php_scylladb_schema *php_scylladb_schema_object_fetch(zend_object *obj)
{
    return (php_scylladb_schema *)((char *)obj - ((size_t)(&(((php_scylladb_schema *)0)->zendObject))));
}

typedef struct php_scylladb_keyspace_
{
    zval schema;
    const CassKeyspaceMeta *meta;
    zend_object zendObject;
} php_scylladb_keyspace;
static zend_always_inline php_scylladb_keyspace *php_scylladb_keyspace_object_fetch(zend_object *obj)
{
    return (php_scylladb_keyspace *)((char *)obj - ((size_t)(&(((php_scylladb_keyspace *)0)->zendObject))));
}

typedef struct php_scylladb_table_
{
    zval name;
    zval options;
    zval partition_key;
    zval primary_key;
    zval clustering_key;
    zval clustering_order;
    zval schema;
    const CassTableMeta *meta;
    zend_object zendObject;
} php_scylladb_table;
static zend_always_inline php_scylladb_table *php_scylladb_table_object_fetch(zend_object *obj)
{
    return (php_scylladb_table *)((char *)obj - ((size_t)(&(((php_scylladb_table *)0)->zendObject))));
}

typedef struct php_scylladb_materialized_view_
{
    zval name;
    zval options;
    zval partition_key;
    zval primary_key;
    zval clustering_key;
    zval clustering_order;
    zval base_table;
    zval schema;
    const CassMaterializedViewMeta *meta;
    zend_object zendObject;
} php_scylladb_materialized_view;
static zend_always_inline php_scylladb_materialized_view *php_scylladb_materialized_view_object_fetch(zend_object *obj)
{
    return (php_scylladb_materialized_view *)((char *)obj - ((size_t)(&(((php_scylladb_materialized_view *)0)->zendObject))));
}

typedef struct php_scylladb_column_
{
    zval name;
    zval type;
    int reversed;
    int frozen;
    zval schema;
    const CassColumnMeta *meta;
    zend_object zendObject;
} php_scylladb_column;
static zend_always_inline php_scylladb_column *php_scylladb_column_object_fetch(zend_object *obj)
{
    return (php_scylladb_column *)((char *)obj - ((size_t)(&(((php_scylladb_column *)0)->zendObject))));
}

typedef struct php_scylladb_index_
{
    zval name;
    zval kind;
    zval target;
    zval options;
    zval schema;
    const CassIndexMeta *meta;
    zend_object zendObject;
} php_scylladb_index;
static zend_always_inline php_scylladb_index *php_scylladb_index_object_fetch(zend_object *obj)
{
    return (php_scylladb_index *)((char *)obj - ((size_t)(&(((php_scylladb_index *)0)->zendObject))));
}

typedef struct php_scylladb_function_
{
    zval simple_name;
    zval arguments;
    zval return_type;
    zval signature;
    zval language;
    zval body;
    zval schema;
    const CassFunctionMeta *meta;
    zend_object zendObject;
} php_scylladb_function;
static zend_always_inline php_scylladb_function *php_scylladb_function_object_fetch(zend_object *obj)
{
    return (php_scylladb_function *)((char *)obj - ((size_t)(&(((php_scylladb_function *)0)->zendObject))));
}

typedef struct php_scylladb_aggregate_
{
    zval simple_name;
    zval argument_types;
    zval state_function;
    zval final_function;
    zval initial_condition;
    zval state_type;
    zval return_type;
    zval signature;
    zval schema;
    const CassAggregateMeta *meta;
    zend_object zendObject;
} php_scylladb_aggregate;
static zend_always_inline php_scylladb_aggregate *php_scylladb_aggregate_object_fetch(zend_object *obj)
{
    return (php_scylladb_aggregate *)((char *)obj - ((size_t)(&(((php_scylladb_aggregate *)0)->zendObject))));
}

typedef struct php_scylladb_type_
{
    CassValueType type;
    CassDataType *data_type;
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
            char *class_name;
        } custom;
        struct
        {
            char *keyspace;
            char *type_name;
            HashTable types;
        } udt;
        struct
        {
            HashTable types;
        } tuple;
    } data;
    zend_object zendObject;
} php_scylladb_type;
static zend_always_inline php_scylladb_type *php_scylladb_type_object_fetch(zend_object *obj)
{
    return (php_scylladb_type *)((char *)obj - ((size_t)(&(((php_scylladb_type *)0)->zendObject))));
}

typedef unsigned (*php_scylladb_value_hash_t)(zval *obj);

typedef struct
{
    zend_object_handlers std;
    php_scylladb_value_hash_t hash_value;
} php_scylladb_value_handlers;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_value_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_numeric_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_bigint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_smallint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_tinyint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_blob_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_decimal_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_float_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_inet_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timestamp_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_uuid_interface_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_uuid_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timeuuid_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_varint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_custom_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_duration_ce;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_set_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_map_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_collection_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_tuple_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_user_type_value_ce;

/*
 * Cassandra\Async\Poll — an Io\Poll\Context plus the futures watched on it (see
 * src/Async/Poll.c). Declared unconditionally (it names no Io\Poll type), but
 * only defined in a build with HAVE_PHP_POLL_API.
 */
typedef struct php_scylladb_async_poll_
{
    zval        context; /* the Io\Poll\Context this loop owns */
    HashTable  *watched; /* watcher object handle → php_scylladb_poll_watch */
    zend_object zendObject;
} php_scylladb_async_poll;
static zend_always_inline php_scylladb_async_poll *php_scylladb_async_poll_object_fetch(zend_object *obj)
{
    return (php_scylladb_async_poll *)((char *)obj - offsetof(php_scylladb_async_poll, zendObject));
}
#define PHP_SCYLLADB_GET_ASYNC_POLL(zv) php_scylladb_async_poll_object_fetch(Z_OBJ_P(zv))

/* Exceptions */

/* Types */

/* Classes */
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_async_poll_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_core_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_cluster_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_cluster_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_cluster_builder_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_ssl_options_builder_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_future_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_future_prepared_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_future_rows_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_future_session_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_future_value_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_future_close_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_session_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_session_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_runtime_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timeout_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_logic_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_domain_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_invalid_argument_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_server_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_overloaded_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_is_bootstrapping_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_execution_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_truncate_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_write_timeout_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_read_timeout_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_unavailable_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_validation_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_invalid_syntax_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_unauthorized_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_invalid_query_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_configuration_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_already_exists_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_unprepared_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_protocol_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_authentication_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_divide_by_zero_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_range_exception_ce;

extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_simple_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_prepared_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_batch_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_execution_options_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_rows_ce;


extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_schema_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_schema_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_keyspace_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_keyspace_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_table_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_table_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_column_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_column_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_index_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_index_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_materialized_view_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_materialized_view_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_function__ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_function_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_aggregate_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_default_aggregate_ce;


extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_scalar_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_collection_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_set_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_map_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_tuple_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_user_type_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_type_custom_ce;


extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timestamp_generator_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timestamp_generator_monotonic_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timestamp_generator_server_side_ce;


extern int php_le_php_scylladb_cluster();
extern int php_le_php_scylladb_session();
extern int php_le_php_scylladb_prepared_statement();
extern int php_le_cass_statement();
END_EXTERN_C()