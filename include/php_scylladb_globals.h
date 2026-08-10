#ifndef PHP_SCYLLADB_GLOBALS_H
#define PHP_SCYLLADB_GLOBALS_H

BEGIN_EXTERN_C()

/* Shared async reactor (see src/Async/Reactor.c); per-request, per-thread under
   ZTS. Forward-declared to avoid a hard include here. */
typedef struct php_scylladb_reactor_ php_scylladb_reactor;

ZEND_BEGIN_MODULE_GLOBALS(php_scylladb)
  php_scylladb_reactor *reactor;
  CassUuidGen  *uuid_gen;
  pid_t         uuid_gen_pid;
  unsigned int  persistent_clusters;
  unsigned int  persistent_sessions;
  unsigned int  persistent_prepared_statements;
  bool          expose_credentials;

  /* Persistence policy (INI-backed). A negative max means unlimited. */
  bool          allow_persistent;
  zend_long     max_persistent_clusters;
  zend_long     max_persistent_sessions;
  zend_long     max_persistent_prepared_statements;

  /* One warning per request per resource kind, not one per call. */
  bool          warned_cluster_cap;
  bool          warned_session_cap;
  bool          warned_prepared_statement_cap;

  /* Cluster\Builder seed values (INI-backed). Every one of these is mixed
   * into the persistent-cluster cache key, so all are PHP_INI_SYSTEM: a
   * per-request ini_set() would mint a new CassCluster on every request. */
  char         *contact_points;
  zend_long     port;
  zend_long     connect_timeout;
  zend_long     request_timeout;
  zend_long     default_consistency;
  zend_long     default_page_size;
  zend_long     protocol_version;
  zend_long     io_threads;
  zend_long     core_connections_per_host;
  zend_long     max_connections_per_host;
  zend_long     reconnect_interval;
  zend_long     connection_heartbeat_interval;
  zend_long     tcp_keepalive_delay;
  bool          token_aware_routing;
  bool          latency_aware_routing;
  bool          tcp_nodelay;
  bool          schema_metadata;
  bool          hostname_resolution;
  bool          randomized_contact_points;

  /* ScyllaDB rack-aware routing. A non-empty local_rack selects it. */
  char         *local_dc;
  char         *local_rack;

  /* Reported in the server's system.clients table. */
  char         *application_name;
  char         *application_version;

  char         *reconnect_policy;
  zend_long     reconnect_max_interval;
  zend_long     speculative_execution_delay;
  zend_long     speculative_execution_max;
  zend_long     coalesce_delay;
  zend_long     new_request_ratio;

  char         *local_address;
  zend_long     connection_idle_timeout;
  zend_long     max_schema_wait_time;
  zend_long     resolve_timeout;
  zend_long     monitor_reporting_interval;
  zend_long     queue_size_io;
  zend_long     tracing_consistency;
  zend_long     tracing_max_wait_time;
  zend_long     tracing_retry_wait_time;
  bool          prepare_on_all_hosts;
  bool          prepare_on_up_or_add_host;
  bool          shuffle_replicas;
  bool          no_compact;
  bool          beta_protocol;

  zval  type_varchar;
  zval  type_text;
  zval  type_blob;
  zval  type_ascii;
  zval  type_bigint;
  zval  type_counter;
  zval  type_int;
  zval  type_varint;
  zval  type_boolean;
  zval  type_decimal;
  zval  type_double;
  zval  type_float;
  zval  type_inet;
  zval  type_timestamp;
  zval  type_date;
  zval  type_time;
  zval  type_uuid;
  zval  type_timeuuid;
  zval  type_smallint;
  zval  type_tinyint;
  zval  type_duration;
  zend_resource stmt;
ZEND_END_MODULE_GLOBALS(php_scylladb)

ZEND_EXTERN_MODULE_GLOBALS(php_scylladb)
END_EXTERN_C()

#endif /* PHP_SCYLLADB_GLOBALS_H */

