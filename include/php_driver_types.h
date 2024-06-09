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

#include <RetryPolicy/RetryPolicy.h>
#include <php.h>
#include <php_driver.h>

#include "SSLOptions/SSLOptions.h"

BEGIN_EXTERN_C()

#define PHP_DRIVER_GET_NUMERIC(obj) php_driver_numeric_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_BLOB(obj) php_driver_blob_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_UUID(obj) php_driver_uuid_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_INET(obj) php_driver_inet_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_COLLECTION(obj) php_driver_collection_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_MAP(obj) php_driver_map_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SET(obj) php_driver_set_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TUPLE(obj) php_driver_tuple_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_USER_TYPE_VALUE(obj) php_driver_user_type_value_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_CLUSTER(obj) php_driver_cluster_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_STATEMENT(obj) php_driver_statement_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_EXECUTION_OPTIONS(obj) php_driver_execution_options_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_ROWS(obj) php_driver_rows_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUTURE_ROWS(obj) php_driver_future_rows_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_CLUSTER_BUILDER(obj) php_driver_cluster_builder_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUTURE_PREPARED_STATEMENT(obj) php_driver_future_prepared_statement_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUTURE_VALUE(obj) php_driver_future_value_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUTURE_CLOSE(obj) php_driver_future_close_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUTURE_SESSION(obj) php_driver_future_session_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SESSION(obj) php_driver_session_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_SCHEMA(obj) php_driver_schema_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_KEYSPACE(obj) php_driver_keyspace_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TABLE(obj) php_driver_table_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_COLUMN(obj) php_driver_column_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_INDEX(obj) php_driver_index_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_MATERIALIZED_VIEW(obj) php_driver_materialized_view_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_FUNCTION(obj) php_driver_function_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_AGGREGATE(obj) php_driver_aggregate_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TYPE(obj) php_driver_type_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_TIMESTAMP_GEN(obj) php_driver_timestamp_gen_object_fetch(Z_OBJ_P(obj))
#define PHP_DRIVER_GET_DURATION(obj) php_driver_duration_object_fetch(Z_OBJ_P(obj))

typedef enum
{
    PHP_DRIVER_BIGINT,
    PHP_DRIVER_DECIMAL,
    PHP_DRIVER_FLOAT,
    PHP_DRIVER_VARINT,
    PHP_DRIVER_SMALLINT,
    PHP_DRIVER_TINYINT
} php_driver_numeric_type;

typedef struct php_driver_numeric_
{
    php_driver_numeric_type type;
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
} php_driver_numeric;
static zend_always_inline php_driver_numeric *php_driver_numeric_object_fetch(zend_object *obj)
{
    return (php_driver_numeric *)((char *)obj - ((size_t)(&(((php_driver_numeric *)0)->zendObject))));
}


typedef struct php_driver_blob_
{
    cass_byte_t *data;
    size_t size;
    zend_object zendObject;
} php_driver_blob;
static zend_always_inline php_driver_blob *php_driver_blob_object_fetch(zend_object *obj)
{
    return (php_driver_blob *)((char *)obj - ((size_t)(&(((php_driver_blob *)0)->zendObject))));
}

typedef struct php_driver_uuid_
{
    CassUuid uuid;
    zend_object zendObject;
} php_driver_uuid;
static zend_always_inline php_driver_uuid *php_driver_uuid_object_fetch(zend_object *obj)
{
    return (php_driver_uuid *)((char *)obj - ((size_t)(&(((php_driver_uuid *)0)->zendObject))));
}

typedef struct php_driver_inet_
{
    CassInet inet;
    zend_object zendObject;
} php_driver_inet;
static zend_always_inline php_driver_inet *php_driver_inet_object_fetch(zend_object *obj)
{
    return (php_driver_inet *)((char *)obj - ((size_t)(&(((php_driver_inet *)0)->zendObject))));
}

typedef struct php_driver_duration_
{
    cass_int32_t months;
    cass_int32_t days;
    cass_int64_t nanos;
    zend_object zendObject;
} php_driver_duration;
static zend_always_inline php_driver_duration *php_driver_duration_object_fetch(zend_object *obj)
{
    return (php_driver_duration *)((char *)obj - ((size_t)(&(((php_driver_duration *)0)->zendObject))));
}

typedef struct php_driver_collection_
{
    zval type;
    HashTable values;
    unsigned hashv;
    int dirty;
    zend_object zendObject;
} php_driver_collection;
static zend_always_inline php_driver_collection *php_driver_collection_object_fetch(zend_object *obj)
{
    return (php_driver_collection *)((char *)obj - ((size_t)(&(((php_driver_collection *)0)->zendObject))));
}

typedef struct php_driver_map_entry_ php_driver_map_entry;

typedef struct php_driver_map_
{
    zval type;
    php_driver_map_entry *entries;
    unsigned hashv;
    int dirty;
    php_driver_map_entry *iter_curr;
    php_driver_map_entry *iter_temp;
    zend_object zendObject;
} php_driver_map;
static zend_always_inline php_driver_map *php_driver_map_object_fetch(zend_object *obj)
{
    return (php_driver_map *)((char *)obj - ((size_t)(&(((php_driver_map *)0)->zendObject))));
}

typedef struct php_driver_set_entry_ php_driver_set_entry;

typedef struct php_driver_set_
{
    zval type;
    php_driver_set_entry *entries;
    unsigned hashv;
    int dirty;
    php_driver_set_entry *iter_curr;
    php_driver_set_entry *iter_temp;
    int iter_index;
    zend_object zendObject;
} php_driver_set;
static zend_always_inline php_driver_set *php_driver_set_object_fetch(zend_object *obj)
{
    return (php_driver_set *)((char *)obj - ((size_t)(&(((php_driver_set *)0)->zendObject))));
}

typedef struct php_driver_tuple_
{
    zval type;
    HashTable values;
    HashPosition pos;
    unsigned hashv;
    int dirty;
    zend_object zendObject;
} php_driver_tuple;
static zend_always_inline php_driver_tuple *php_driver_tuple_object_fetch(zend_object *obj)
{
    return (php_driver_tuple *)((char *)obj - ((size_t)(&(((php_driver_tuple *)0)->zendObject))));
}

typedef struct php_driver_user_type_value_
{
    zval type;
    HashTable values;
    HashPosition pos;
    unsigned hashv;
    int dirty;
    zend_object zendObject;
} php_driver_user_type_value;
static zend_always_inline php_driver_user_type_value *php_driver_user_type_value_object_fetch(zend_object *obj)
{
    return (php_driver_user_type_value *)((char *)obj - ((size_t)(&(((php_driver_user_type_value *)0)->zendObject))));
}

typedef struct php_driver_cluster_
{
    cass_byte_t *data;
    CassCluster *cluster;
    uint32_t default_consistency;
    uint32_t default_page_size;
    zval default_timeout;
    cass_bool_t persist;
    char *hash_key;
    int hash_key_len;
    zend_object zendObject;
} php_driver_cluster;
static zend_always_inline php_driver_cluster *php_driver_cluster_object_fetch(zend_object *obj)
{
    return (php_driver_cluster *)((char *)obj - offsetof(php_driver_cluster, zendObject));
}

typedef enum
{
    PHP_DRIVER_SIMPLE_STATEMENT,
    PHP_DRIVER_PREPARED_STATEMENT,
    PHP_DRIVER_BATCH_STATEMENT
} php_driver_statement_type;

typedef struct php_driver_statement_
{
    php_driver_statement_type type;
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
} php_driver_statement;
static zend_always_inline php_driver_statement *php_driver_statement_object_fetch(zend_object *obj)
{
    return (php_driver_statement *)((char *)obj - ((size_t)(&(((php_driver_statement *)0)->zendObject))));
}

typedef struct
{
    zval statement;
    zval arguments;
} php_driver_batch_statement_entry;

typedef struct php_driver_execution_options_
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
} php_driver_execution_options;
static zend_always_inline php_driver_execution_options *php_driver_execution_options_object_fetch(zend_object *obj)
{
    return (php_driver_execution_options *)((char *)obj - ((size_t)(&(((php_driver_execution_options *)0)->zendObject))));
}

typedef enum /* : uint8_t */
{
    LOAD_BALANCING_DEFAULT = 0,
    LOAD_BALANCING_ROUND_ROBIN,
    LOAD_BALANCING_DC_AWARE_ROUND_ROBIN
} php_driver_load_balancing;

typedef void (*php_driver_free_function)(void *data);

typedef struct
{
    size_t count;
    php_driver_free_function destruct;
    void *data;
} php_driver_ref;

typedef struct php_driver_rows_
{
    php_driver_ref *statement;
    php_driver_ref *session;
    zval rows;
    zval next_rows;
    php_driver_ref *result;
    php_driver_ref *next_result;
    zval future_next_page;
    zend_object zendObject;
} php_driver_rows;
static zend_always_inline php_driver_rows *php_driver_rows_object_fetch(zend_object *obj)
{
    return (php_driver_rows *)((char *)obj - ((size_t)(&(((php_driver_rows *)0)->zendObject))));
}

typedef struct php_driver_future_rows_
{
    php_driver_ref *statement;
    php_driver_ref *session;
    zval rows;
    php_driver_ref *result;
    CassFuture *future;
    zend_object zendObject;
} php_driver_future_rows;
static zend_always_inline php_driver_future_rows *php_driver_future_rows_object_fetch(zend_object *obj)
{
    return (php_driver_future_rows *)((char *)obj - ((size_t)(&(((php_driver_future_rows *)0)->zendObject))));
}


typedef struct php_driver_timestamp_gen_
{
    CassTimestampGen *gen;
    zend_object zendObject;
} php_driver_timestamp_gen;
static zend_always_inline php_driver_timestamp_gen *php_driver_timestamp_gen_object_fetch(zend_object *obj)
{
    return (php_driver_timestamp_gen *)((char *)obj - ((size_t)(&(((php_driver_timestamp_gen *)0)->zendObject))));
}

typedef struct php_driver_cluster_builder_
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
    php_driver_load_balancing load_balancing_policy;
    cass_bool_t persist;
    php_driver_retry_policy *retry_policy;
    php_driver_timestamp_gen *timestamp_gen;
    php_scylladb_ssl *ssl_options;
    zval default_timeout;

    zend_object zendObject;

} php_driver_cluster_builder;
static zend_always_inline php_driver_cluster_builder *php_driver_cluster_builder_object_fetch(zend_object *obj)
{
    return (php_driver_cluster_builder *)((char *)obj - offsetof(php_driver_cluster_builder, zendObject));
}

typedef struct php_driver_future_prepared_statement_
{
    CassFuture *future;
    zval prepared_statement;
    zend_object zendObject;
} php_driver_future_prepared_statement;
static zend_always_inline php_driver_future_prepared_statement *php_driver_future_prepared_statement_object_fetch(
    zend_object *obj)
{
    return (php_driver_future_prepared_statement *)((char *)obj -
                                                    ((size_t)(&(((php_driver_future_prepared_statement *)0)->zendObject))));
}

typedef struct php_driver_future_value_
{
    zval value;
    zend_object zendObject;
} php_driver_future_value;
static zend_always_inline php_driver_future_value *php_driver_future_value_object_fetch(zend_object *obj)
{
    return (php_driver_future_value *)((char *)obj - ((size_t)(&(((php_driver_future_value *)0)->zendObject))));
}

typedef struct php_driver_future_close_
{
    CassFuture *future;
    zend_object zendObject;
} php_driver_future_close;
static zend_always_inline php_driver_future_close *php_driver_future_close_object_fetch(zend_object *obj)
{
    return (php_driver_future_close *)((char *)obj - ((size_t)(&(((php_driver_future_close *)0)->zendObject))));
}

typedef struct php_driver_future_session_
{
    CassFuture *future;
    php_driver_ref *session;
    zval default_session;
    cass_bool_t persist;
    char *hash_key;
    int hash_key_len;
    char *exception_message;
    CassError exception_code;
    char *session_keyspace;
    char *session_hash_key;
    zend_object zendObject;
} php_driver_future_session;
static zend_always_inline php_driver_future_session *php_driver_future_session_object_fetch(zend_object *obj)
{
    return (php_driver_future_session *)((char *)obj - ((size_t)(&(((php_driver_future_session *)0)->zendObject))));
}

typedef struct
{
    CassFuture *future;
    php_driver_ref *session;
} php_driver_psession;

typedef struct
{
    CassFuture *future;
    php_driver_ref *ref;
} php_driver_pprepared_statement;

typedef struct php_driver_session_
{
    php_driver_ref *session;
    long default_consistency;
    int default_page_size;
    char *keyspace;
    char *hash_key;
    zval default_timeout;
    cass_bool_t persist;
    zend_object zendObject;
} php_driver_session;
static zend_always_inline php_driver_session *php_driver_session_object_fetch(zend_object *obj)
{
    return (php_driver_session *)((char *)obj - offsetof(php_driver_session, zendObject));
}

typedef struct php_driver_schema_
{
    php_driver_ref *schema;
    zend_object zendObject;
} php_driver_schema;
static zend_always_inline php_driver_schema *php_driver_schema_object_fetch(zend_object *obj)
{
    return (php_driver_schema *)((char *)obj - ((size_t)(&(((php_driver_schema *)0)->zendObject))));
}

typedef struct php_driver_keyspace_
{
    php_driver_ref *schema;
    const CassKeyspaceMeta *meta;
    zend_object zendObject;
} php_driver_keyspace;
static zend_always_inline php_driver_keyspace *php_driver_keyspace_object_fetch(zend_object *obj)
{
    return (php_driver_keyspace *)((char *)obj - ((size_t)(&(((php_driver_keyspace *)0)->zendObject))));
}

typedef struct php_driver_table_
{
    zval name;
    zval options;
    zval partition_key;
    zval primary_key;
    zval clustering_key;
    zval clustering_order;
    php_driver_ref *schema;
    const CassTableMeta *meta;
    zend_object zendObject;
} php_driver_table;
static zend_always_inline php_driver_table *php_driver_table_object_fetch(zend_object *obj)
{
    return (php_driver_table *)((char *)obj - ((size_t)(&(((php_driver_table *)0)->zendObject))));
}

typedef struct php_driver_materialized_view_
{
    zval name;
    zval options;
    zval partition_key;
    zval primary_key;
    zval clustering_key;
    zval clustering_order;
    zval base_table;
    php_driver_ref *schema;
    const CassMaterializedViewMeta *meta;
    zend_object zendObject;
} php_driver_materialized_view;
static zend_always_inline php_driver_materialized_view *php_driver_materialized_view_object_fetch(zend_object *obj)
{
    return (php_driver_materialized_view *)((char *)obj - ((size_t)(&(((php_driver_materialized_view *)0)->zendObject))));
}

typedef struct php_driver_column_
{
    zval name;
    zval type;
    int reversed;
    int frozen;
    php_driver_ref *schema;
    const CassColumnMeta *meta;
    zend_object zendObject;
} php_driver_column;
static zend_always_inline php_driver_column *php_driver_column_object_fetch(zend_object *obj)
{
    return (php_driver_column *)((char *)obj - ((size_t)(&(((php_driver_column *)0)->zendObject))));
}

typedef struct php_driver_index_
{
    zval name;
    zval kind;
    zval target;
    zval options;
    php_driver_ref *schema;
    const CassIndexMeta *meta;
    zend_object zendObject;
} php_driver_index;
static zend_always_inline php_driver_index *php_driver_index_object_fetch(zend_object *obj)
{
    return (php_driver_index *)((char *)obj - ((size_t)(&(((php_driver_index *)0)->zendObject))));
}

typedef struct php_driver_function_
{
    zval simple_name;
    zval arguments;
    zval return_type;
    zval signature;
    zval language;
    zval body;
    php_driver_ref *schema;
    const CassFunctionMeta *meta;
    zend_object zendObject;
} php_driver_function;
static zend_always_inline php_driver_function *php_driver_function_object_fetch(zend_object *obj)
{
    return (php_driver_function *)((char *)obj - ((size_t)(&(((php_driver_function *)0)->zendObject))));
}

typedef struct php_driver_aggregate_
{
    zval simple_name;
    zval argument_types;
    zval state_function;
    zval final_function;
    zval initial_condition;
    zval state_type;
    zval return_type;
    zval signature;
    php_driver_ref *schema;
    const CassAggregateMeta *meta;
    zend_object zendObject;
} php_driver_aggregate;
static zend_always_inline php_driver_aggregate *php_driver_aggregate_object_fetch(zend_object *obj)
{
    return (php_driver_aggregate *)((char *)obj - ((size_t)(&(((php_driver_aggregate *)0)->zendObject))));
}

typedef struct php_driver_type_
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
} php_driver_type;
static zend_always_inline php_driver_type *php_driver_type_object_fetch(zend_object *obj)
{
    return (php_driver_type *)((char *)obj - ((size_t)(&(((php_driver_type *)0)->zendObject))));
}

typedef unsigned (*php_driver_value_hash_t)(zval *obj);

typedef struct
{
    zend_object_handlers std;
    php_driver_value_hash_t hash_value;
} php_driver_value_handlers;

extern PHP_SCYLLADB_API zend_class_entry *php_driver_value_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_numeric_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_bigint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_smallint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_tinyint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_blob_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_decimal_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_float_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_inet_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_timestamp_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_uuid_interface_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_uuid_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_timeuuid_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_varint_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_custom_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_duration_ce;

extern PHP_SCYLLADB_API zend_class_entry *php_driver_set_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_map_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_collection_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_tuple_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_user_type_value_ce;

/* Exceptions */
void php_driver_define_Exception();
void php_driver_define_InvalidArgumentException();
void php_driver_define_DomainException();
void php_driver_define_LogicException();
void php_driver_define_RuntimeException();
void php_driver_define_TimeoutException();
void php_driver_define_ExecutionException();
void php_driver_define_ReadTimeoutException();
void php_driver_define_WriteTimeoutException();
void php_driver_define_UnavailableException();
void php_driver_define_TruncateException();
void php_driver_define_ValidationException();
void php_driver_define_InvalidQueryException();
void php_driver_define_InvalidSyntaxException();
void php_driver_define_UnauthorizedException();
void php_driver_define_UnpreparedException();
void php_driver_define_ConfigurationException();
void php_driver_define_AlreadyExistsException();
void php_driver_define_AuthenticationException();
void php_driver_define_ProtocolException();
void php_driver_define_ServerException();
void php_driver_define_IsBootstrappingException();
void php_driver_define_OverloadedException();
void php_driver_define_DivideByZeroException();
void php_driver_define_RangeException();

/* Types */
void php_driver_define_Value();
void php_driver_define_Numeric();
void php_driver_define_Bigint();
void php_driver_define_Smallint();
void php_driver_define_Tinyint();
void php_driver_define_Blob();
void php_driver_define_Collection();
void php_driver_define_Decimal();
void php_driver_define_Float();
void php_driver_define_Inet();
void php_driver_define_Map();
void php_driver_define_Set();
void php_driver_define_Tuple();
void php_driver_define_UserTypeValue();
void php_driver_define_UuidInterface();
void php_driver_define_Uuid();
void php_driver_define_Varint();
void php_driver_define_Custom();
void php_driver_define_Duration();

/* Classes */
extern PHP_SCYLLADB_API zend_class_entry *php_driver_core_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_cluster_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_cluster_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_cluster_builder_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_cluster_builder_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_scylladb_ssl_builder_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_future_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_future_prepared_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_future_rows_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_future_session_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_future_value_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_future_close_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_session_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_session_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_runtime_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_timeout_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_logic_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_domain_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_invalid_argument_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_server_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_overloaded_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_is_bootstrapping_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_execution_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_truncate_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_write_timeout_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_read_timeout_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_truncate_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_unavailable_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_validation_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_invalid_syntax_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_unauthorized_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_invalid_query_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_configuration_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_already_exists_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_unprepared_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_protocol_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_authentication_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_divide_by_zero_exception_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_range_exception_ce;

extern PHP_SCYLLADB_API zend_class_entry *php_driver_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_simple_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_prepared_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_batch_statement_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_execution_options_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_rows_ce;

void php_driver_define_Core();
void php_driver_define_Future();
void php_driver_define_FuturePreparedStatement();
void php_driver_define_FutureRows();
void php_driver_define_FutureSession();
void php_driver_define_FutureValue();
void php_driver_define_FutureClose();
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

extern PHP_SCYLLADB_API zend_class_entry *php_driver_schema_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_schema_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_keyspace_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_keyspace_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_table_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_table_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_column_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_column_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_index_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_index_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_materialized_view_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_materialized_view_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_function_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_function_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_aggregate_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_default_aggregate_ce;

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

extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_scalar_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_collection_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_set_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_map_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_tuple_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_user_type_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_type_custom_ce;

void php_driver_define_Type();
void php_driver_define_TypeScalar();
void php_driver_define_TypeCollection();
void php_driver_define_TypeSet();
void php_driver_define_TypeMap();
void php_driver_define_TypeTuple();
void php_driver_define_TypeUserType();
void php_driver_define_TypeCustom();

zend_class_entry*php_scylladb_define_RetryPolicy(void);
void php_scylladb_define_RetryPolicyDefault(zend_class_entry* retry_policy_interface);
void php_driver_define_RetryPolicyDowngradingConsistency(zend_class_entry* retry_policy_interface);
void php_driver_define_RetryPolicyFallthrough(zend_class_entry* retry_policy_interface);
void php_driver_define_RetryPolicyLogging(zend_class_entry* retry_policy_interface);

extern PHP_SCYLLADB_API zend_class_entry *php_driver_timestamp_gen_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_timestamp_gen_monotonic_ce;
extern PHP_SCYLLADB_API zend_class_entry *php_driver_timestamp_gen_server_side_ce;

void php_driver_define_TimestampGenerator();
void php_driver_define_TimestampGeneratorMonotonic();
void php_driver_define_TimestampGeneratorServerSide();

extern int php_le_php_driver_cluster();
extern int php_le_php_driver_session();
extern int php_le_php_driver_prepared_statement();
END_EXTERN_C()