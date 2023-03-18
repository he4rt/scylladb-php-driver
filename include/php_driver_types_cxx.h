#pragma once

#include <cstdint>
#include <gmp.h>
#include <util/hash.h>
#include <zend_types.h>

#include <ZendCPP/String/String.h>

#include <functional>
#include <optional>

namespace php_driver {

enum class numeric_type { BIGINT, DECIMAL, FLOAT, VARINT, SMALLINT, TINYINT };

struct numeric {
  numeric_type type;
  union {
    struct {
      int8_t value;
    } tinyint;
    struct {
      int16_t value;
    } smallint;
    struct {
      int64_t value;
    } bigint;
    struct {
      float value;
    } floating;
    struct {
      mpz_t value;
    } varint;
    struct {
      mpz_t value;
      long scale;
    } decimal;
  } data;
  zend_object zval;
};

struct timestamp {
  int64_t timestamp;
  zend_object zval;
};

struct date {
  uint32_t date;
  zend_object zval;
};

struct time {
  int64_t time;
  zend_object zval;
};

struct blob {
  uint8_t *data;
  size_t size;
  zend_object zval;
};

struct uuid {
  CassUuid uuid;
  zend_object zval;
};

struct inet {
  CassInet inet;
  zend_object zval;
};

struct duration {
  int32_t months;
  int32_t days;
  int64_t nanos;
  zend_object zval;
};

struct collection {
  zval type;
  HashTable values;
  unsigned hashv;
  int dirty;
  zend_object zval;
};

struct map {
  zval type;
  php_driver_map_entry_ *entries;
  unsigned hashv;
  int dirty;
  php_driver_map_entry_ *iter_curr;
  php_driver_map_entry_ *iter_temp;
  zend_object zval;
} php_driver_map;

struct set {
  zval type;
  php_driver_set_entry_ *entries;
  unsigned hashv;
  int dirty;
  php_driver_set_entry_ *iter_curr;
  php_driver_set_entry_ *iter_temp;
  int iter_index;
  zend_object zval;
};

struct tuple {
  zval type;
  HashTable values;
  HashPosition pos;
  unsigned hashv;
  int dirty;
  zend_object zval;
};

struct value {
  zval type;
  HashTable values;
  HashPosition pos;
  unsigned hashv;
  int dirty;
  zend_object zval;
};

struct cluster {
  uint8_t *data{};
  CassCluster *cluster{};
  uint32_t default_consistency{};
  uint32_t default_page_size{};
  std::optional<uint32_t> default_timeout;
  bool persist{};
  uint8_t _padding_[3]{};
  zend_string *hash_key{};
  zend_object zval{};
};

enum class statement_type { SIMPLE, PREPARED, BATCH };

struct statement {
  statement_type type;
  union {
    struct {
      char *cql;
    } simple;
    struct {
      const CassPrepared *prepared;
    } prepared;
    struct {
      CassBatchType type;
      HashTable statements;
    } batch;
  } data;
  zend_object zval;
};

struct batch_statement_entry {
  zval statement;
  zval arguments;
};

struct execution_options {
  int64_t consistency;
  int64_t serial_consistency;
  int32_t page_size;
  zend_string *paging_state_token;
  zval timeout;
  zval arguments;
  zval retry_policy;
  int64_t timestamp;
  zend_object zval;
};

enum class load_balancing { DEFAULT = 0, ROUND_ROBIN, DC_AWARE_ROUND_ROBIN };

struct ref_count {
  size_t count;
  std::function<void(void *)> destruct;
  void *data;
};

struct rows {
  ref_count *statement;
  ref_count *session;
  zval rows;
  zval next_rows;
  ref_count *result;
  ref_count *next_result;
  zval future_next_page;
  zend_object zval;
};

struct future_rows {
  ref_count *statement;
  ref_count *session;
  php5to7_zval rows;
  ref_count *result;
  CassFuture *future;
  zend_object zval;
};

struct retry_policy {
  CassRetryPolicy *policy;
  zend_object zval;
};

struct ssl {
  CassSsl *ssl;
  zend_object zval;
};

struct timestamp_generator {
  CassTimestampGen *gen;
  zend_object zval;
};

struct cluster_builder {
  ZendCPP::String contact_points{};
  ZendCPP::String username{};
  ZendCPP::String password{};
  ZendCPP::String local_dc{};
  ZendCPP::String blacklist_hosts{};
  ZendCPP::String whitelist_hosts{};
  ZendCPP::String blacklist_dcs{};
  ZendCPP::String whitelist_dcs{};

  uint32_t used_hosts_per_remote_dc{};
  uint32_t connect_timeout{};
  uint32_t default_consistency{};
  uint32_t default_page_size{};
  uint32_t protocol_version{};
  uint32_t reconnect_interval{};
  uint32_t tcp_keepalive_delay{};
  bool enable_latency_aware_routing{};
  bool enable_tcp_nodelay{};
  bool enable_tcp_keepalive{};
  bool enable_schema{};
  bool enable_hostname_resolution{};
  bool enable_randomized_contact_points{};
  bool allow_remote_dcs_for_local_cl{};
  bool use_token_aware_routing{};
  uint32_t connection_heartbeat_interval{};
  uint32_t request_timeout{};
  uint16_t port{};
  bool persist{};
  uint8_t io_threads{};
  load_balancing load_balancing_policy;
  retry_policy *retry_policy{};
  timestamp_generator *timestamp_gen{};
  ssl *ssl_options{};
  std::optional<uint32_t> default_timeout;
  uint8_t core_connections_per_host{};
  uint8_t max_connections_per_host{};
  uint8_t _padding_{};
  zend_object zval;
};

struct future_prepared_statement {
  CassFuture *future;
  zval prepared_statement;
  zend_object zval;
};

struct future_value {
  zval value;
  zend_object zval;
};

struct future_close {
  CassFuture *future;
  zend_object zval;
};

struct future_session {
  CassFuture *future;
  ref_count *session;
  zval default_session;
  cass_bool_t persist;
  CassError exception_code;
  ZendCPP::String hash_key;
  ZendCPP::String exception_message;
  ZendCPP::String session_keyspace;
  ZendCPP::String session_hash_key;
  zend_object zval;
};

struct persistent_session {
  CassFuture *future;
  ref_count *session;
};

struct persistent_prepared_statement {
  CassFuture *future;
  ref_count *ref;
};

struct session {
  ref_count *session;
  int64_t default_consistency;
  int32_t default_page_size;
  ZendCPP::String keyspace;
  ZendCPP::String hash_key;
  zval default_timeout;
  cass_bool_t persist;
  zend_object zval;
};

struct ssl_builder {
  int32_t flags;
  int32_t trusted_certs_cnt;
  char **trusted_certs;
  ZendCPP::String client_cert;
  ZendCPP::String private_key;
  ZendCPP::String passphrase;
  zend_object zval;
};

struct schema {
  ref_count *schema;
  zend_object zval;
};

struct keyspace {
  ref_count *schema;
  const CassKeyspaceMeta *meta;
  zend_object zval;
};

struct table {
  zval name;
  zval options;
  zval partition_key;
  zval primary_key;
  zval clustering_key;
  zval clustering_order;
  ref_count *schema;
  const CassTableMeta *meta;
  zend_object zval;
};

struct materialized_view {
  zval name;
  zval options;
  zval partition_key;
  zval primary_key;
  zval clustering_key;
  zval clustering_order;
  zval base_table;
  ref_count *schema;
  const CassMaterializedViewMeta *meta;
  zend_object zval;
};

struct column {
  zval name;
  zval type;
  int32_t reversed;
  int32_t frozen;
  ref_count *schema;
  const CassColumnMeta *meta;
  zend_object zval;
};

struct index {
  zval name;
  zval kind;
  zval target;
  zval options;
  ref_count *schema;
  const CassIndexMeta *meta;
  zend_object zval;
};

struct function {
  zval simple_name;
  zval arguments;
  zval return_type;
  zval signature;
  zval language;
  zval body;
  ref_count *schema;
  const CassFunctionMeta *meta;
  zend_object zval;
};

struct aggregate {
  zval simple_name;
  zval argument_types;
  zval state_function;
  zval final_function;
  zval initial_condition;
  zval state_type;
  zval return_type;
  zval signature;
  ref_count *schema;
  const CassAggregateMeta *meta;
  zend_object zval;
};

struct type {
  CassValueType type;
  CassDataType *data_type;
  union {
    struct {
      zval value_type;
    } collection;
    struct {
      zval value_type;
    } set;
    struct {
      zval key_type;
      zval value_type;
    } map;
    struct {
      char *class_name;
    } custom;
    struct {
      char *keyspace;
      char *type_name;
      HashTable types;
    } udt;
    struct {
      HashTable types;
    } tuple;
  } data;
  zend_object zval;
};

struct value_handlers {
  zend_object_handlers std{};
  std::function<void(zval *)> hash_value;
};

} // namespace php_driver