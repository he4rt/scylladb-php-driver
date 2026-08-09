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

#include <src/DateTime/DateTimeInternal.h>
#include <Registry/Registry.h>
#include <Async/Reactor.h>

#include <fcntl.h>
#include <php_scylladb.h>
#include <php_scylladb_consistency.h>
#include <php_scylladb_globals.h>
#include <php_scylladb_types.h>
#include <php_ini.h>
#include <pthread.h>
#include <syslog.h>
#include <version.h>

#include <time.h>

#include <ext/standard/info.h>

#define PHP_SCYLLADB_SCALAR_TYPES_MAP(XX)    \
  XX(ascii, CASS_VALUE_TYPE_ASCII)         \
  XX(bigint, CASS_VALUE_TYPE_BIGINT)       \
  XX(smallint, CASS_VALUE_TYPE_SMALL_INT)  \
  XX(tinyint, CASS_VALUE_TYPE_TINY_INT)    \
  XX(blob, CASS_VALUE_TYPE_BLOB)           \
  XX(boolean, CASS_VALUE_TYPE_BOOLEAN)     \
  XX(counter, CASS_VALUE_TYPE_COUNTER)     \
  XX(decimal, CASS_VALUE_TYPE_DECIMAL)     \
  XX(double, CASS_VALUE_TYPE_DOUBLE)       \
  XX(duration, CASS_VALUE_TYPE_DURATION)   \
  XX(float, CASS_VALUE_TYPE_FLOAT)         \
  XX(int, CASS_VALUE_TYPE_INT)             \
  XX(text, CASS_VALUE_TYPE_TEXT)           \
  XX(timestamp, CASS_VALUE_TYPE_TIMESTAMP) \
  XX(date, CASS_VALUE_TYPE_DATE)           \
  XX(time, CASS_VALUE_TYPE_TIME)           \
  XX(uuid, CASS_VALUE_TYPE_UUID)           \
  XX(varchar, CASS_VALUE_TYPE_VARCHAR)     \
  XX(varint, CASS_VALUE_TYPE_VARINT)       \
  XX(timeuuid, CASS_VALUE_TYPE_TIMEUUID)   \
  XX(inet, CASS_VALUE_TYPE_INET)

/* Resources */
#define PHP_SCYLLADB_CLUSTER_RES_NAME PHP_SCYLLADB_NAMESPACE " Cluster"
#define PHP_SCYLLADB_SESSION_RES_NAME PHP_SCYLLADB_NAMESPACE " Session"
#define PHP_SCYLLADB_PREPARED_STATEMENT_RES_NAME PHP_SCYLLADB_NAMESPACE " PreparedStatement"

/* Statically initialised: the driver's log callback runs on the C/C++ driver's
 * event-loop threads, which can fire before MINIT finishes registering the INI
 * entries that write log_location. */
typedef enum : uint8_t {
  PHP_SCYLLADB_LOG_STDERR,
  PHP_SCYLLADB_LOG_FILE,
  PHP_SCYLLADB_LOG_SYSLOG,
} php_scylladb_log_target;

static char *log_location = nullptr;
static php_scylladb_log_target log_target = PHP_SCYLLADB_LOG_STDERR;
static bool log_syslog_opened = false;
static pthread_rwlock_t log_lock = PTHREAD_RWLOCK_INITIALIZER;

// cpp-rs-driver uses its own 1.x versioning track (it's a rewrite, not a
// successor of the 2.x cpp-driver line). Skip the legacy version check on
// the rust backend; required-version constants are about a different library.
#ifndef PHP_SCYLLADB_BACKEND_SCYLLA_RUST
#if CURRENT_CPP_DRIVER_VERSION < CPP_DRIVER_VERSION(2, 16, 2)
#error C/C++ driver version 2.16.2 or greater required
#endif
#endif

ZEND_DECLARE_MODULE_GLOBALS(php_scylladb)

static PHP_GINIT_FUNCTION(php_scylladb);
static PHP_GSHUTDOWN_FUNCTION(php_scylladb);

// clang-format off
const zend_function_entry php_scylladb_functions[] = {
    PHP_FE_END /* Must be the last line in php_scylladb_functions[] */
};
// clang-format on

// clang-format off
static zend_module_dep php_scylladb_deps[] = {
  ZEND_MOD_REQUIRED("spl")
  ZEND_MOD_REQUIRED("date")
  ZEND_MOD_END
};
// clang-format on

// clang-format off
zend_module_entry php_scylladb_module_entry = {
  STANDARD_MODULE_HEADER_EX,
  nullptr,
  php_scylladb_deps,
  PHP_SCYLLADB_NAME,
  php_scylladb_functions,      /* Functions */
  PHP_MINIT(php_scylladb),     /* MINIT */
  PHP_MSHUTDOWN(php_scylladb), /* MSHUTDOWN */
  PHP_RINIT(php_scylladb),     /* RINIT */
  PHP_RSHUTDOWN(php_scylladb), /* RSHUTDOWN */
  PHP_MINFO(php_scylladb),     /* MINFO */
  PHP_SCYLLADB_VERSION,
  PHP_MODULE_GLOBALS(php_scylladb),
  PHP_GINIT(php_scylladb),
  PHP_GSHUTDOWN(php_scylladb),
  nullptr,
  STANDARD_MODULE_PROPERTIES_EX
};
// clang-format on

#ifdef COMPILE_DL_CASSANDRA
ZEND_DLEXPORT zend_module_entry *get_module(void) { return &php_scylladb_module_entry; }
#endif

#define PHP_SCYLLADB_INI_BOOL(name, dflt, field)                                     \
  STD_PHP_INI_BOOLEAN(PHP_SCYLLADB_NAME "." name, dflt, PHP_INI_SYSTEM, OnUpdateBool, \
                      field, zend_php_scylladb_globals, php_scylladb_globals)

#define PHP_SCYLLADB_INI_LONG(name, dflt, handler, field)                        \
  STD_PHP_INI_ENTRY(PHP_SCYLLADB_NAME "." name, dflt, PHP_INI_SYSTEM, handler,   \
                    field, zend_php_scylladb_globals, php_scylladb_globals)

#define PHP_SCYLLADB_INI_STR(name, dflt, field)                                       \
  STD_PHP_INI_ENTRY(PHP_SCYLLADB_NAME "." name, dflt, PHP_INI_SYSTEM, OnUpdateString, \
                    field, zend_php_scylladb_globals, php_scylladb_globals)

/* Every entry here is PHP_INI_SYSTEM by design, not by oversight. The builder
 * seeds feed the persistent-cluster cache key in Cluster/Builder.c, so a
 * request-scoped ini_set() would allocate a fresh CassCluster per request and
 * grow EG(persistent_list) without bound. */
// clang-format off
PHP_INI_BEGIN()
  PHP_INI_ENTRY(PHP_SCYLLADB_NAME ".log", PHP_SCYLLADB_DEFAULT_LOG, PHP_INI_SYSTEM, OnUpdateLog)
  PHP_INI_ENTRY(PHP_SCYLLADB_NAME ".log_level", PHP_SCYLLADB_DEFAULT_LOG_LEVEL, PHP_INI_SYSTEM, OnUpdateLogLevel)
  /* PHP_INI_SYSTEM, so no ini_set() in a request can flip it: turning it on is
   * an operator decision, not something a library or a debug handler can do. */
  STD_PHP_INI_BOOLEAN(PHP_SCYLLADB_NAME ".expose_credentials", "0", PHP_INI_SYSTEM, OnUpdateBool,
                      expose_credentials, zend_php_scylladb_globals, php_scylladb_globals)

  /* Persistence policy. allow_persistent is the operator kill switch:
   * withPersistentSessions(true) cannot re-enable caching once it is off. */
  PHP_SCYLLADB_INI_BOOL("allow_persistent", "1", allow_persistent)
  PHP_SCYLLADB_INI_LONG("max_persistent_clusters", PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_CLUSTERS,
                        OnUpdatePersistentMax, max_persistent_clusters)
  PHP_SCYLLADB_INI_LONG("max_persistent_sessions", PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_SESSIONS,
                        OnUpdatePersistentMax, max_persistent_sessions)
  PHP_SCYLLADB_INI_LONG("max_persistent_prepared_statements", PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_PREPARED,
                        OnUpdatePersistentMax, max_persistent_prepared_statements)

  /* Cluster\Builder seeds. Each with*() method still overrides its seed. */
  STD_PHP_INI_ENTRY(PHP_SCYLLADB_NAME ".contact_points", PHP_SCYLLADB_DEFAULT_CONTACT_POINTS,
                    PHP_INI_SYSTEM, OnUpdateString, contact_points, zend_php_scylladb_globals,
                    php_scylladb_globals)
  PHP_SCYLLADB_INI_LONG("port", PHP_SCYLLADB_DEFAULT_PORT, OnUpdatePort, port)
  PHP_SCYLLADB_INI_LONG("connect_timeout", PHP_SCYLLADB_DEFAULT_CONNECT_TIMEOUT, OnUpdatePositiveLong,
                        connect_timeout)
  PHP_SCYLLADB_INI_LONG("request_timeout", PHP_SCYLLADB_DEFAULT_REQUEST_TIMEOUT, OnUpdatePositiveLong,
                        request_timeout)
  PHP_SCYLLADB_INI_LONG("default_consistency", PHP_SCYLLADB_DEFAULT_CONSISTENCY_NAME,
                        OnUpdateDefaultConsistency, default_consistency)
  PHP_SCYLLADB_INI_LONG("default_page_size", PHP_SCYLLADB_DEFAULT_PAGE_SIZE, OnUpdatePositiveLong,
                        default_page_size)
  PHP_SCYLLADB_INI_LONG("protocol_version", PHP_SCYLLADB_DEFAULT_PROTOCOL_VERSION,
                        OnUpdateProtocolVersion, protocol_version)
  PHP_SCYLLADB_INI_LONG("io_threads", PHP_SCYLLADB_DEFAULT_IO_THREADS, OnUpdatePositiveLong, io_threads)
  PHP_SCYLLADB_INI_LONG("core_connections_per_host", PHP_SCYLLADB_DEFAULT_CORE_CONNECTIONS_PER_HOST,
                        OnUpdatePositiveLong, core_connections_per_host)
  PHP_SCYLLADB_INI_LONG("max_connections_per_host", PHP_SCYLLADB_DEFAULT_MAX_CONNECTIONS_PER_HOST,
                        OnUpdatePositiveLong, max_connections_per_host)
  PHP_SCYLLADB_INI_LONG("reconnect_interval", PHP_SCYLLADB_DEFAULT_RECONNECT_INTERVAL,
                        OnUpdatePositiveLong, reconnect_interval)
  PHP_SCYLLADB_INI_LONG("connection_heartbeat_interval", PHP_SCYLLADB_DEFAULT_HEARTBEAT_INTERVAL,
                        OnUpdateLongGEZero, connection_heartbeat_interval)
  /* Seconds. 0 disables TCP keepalive, which is what withTCPKeepalive(null) does. */
  PHP_SCYLLADB_INI_LONG("tcp_keepalive_delay", PHP_SCYLLADB_DEFAULT_TCP_KEEPALIVE_DELAY,
                        OnUpdateLongGEZero, tcp_keepalive_delay)
  PHP_SCYLLADB_INI_BOOL("token_aware_routing", "1", token_aware_routing)
  PHP_SCYLLADB_INI_BOOL("latency_aware_routing", "1", latency_aware_routing)
  PHP_SCYLLADB_INI_BOOL("tcp_nodelay", "1", tcp_nodelay)
  PHP_SCYLLADB_INI_BOOL("schema_metadata", "1", schema_metadata)
  PHP_SCYLLADB_INI_BOOL("hostname_resolution", "0", hostname_resolution)
  PHP_SCYLLADB_INI_BOOL("randomized_contact_points", "1", randomized_contact_points)

  /* ScyllaDB rack-aware load balancing. Setting local_rack selects the policy;
   * an empty local_rack leaves the load balancing policy alone. */
  PHP_SCYLLADB_INI_STR("local_dc", "", local_dc)
  PHP_SCYLLADB_INI_STR("local_rack", "", local_rack)

  /* Reported to the server, visible in system.clients. */
  PHP_SCYLLADB_INI_STR("application_name", "", application_name)
  PHP_SCYLLADB_INI_STR("application_version", "", application_version)

  /* "constant" keeps reconnect_interval as a fixed delay. "exponential" uses it
   * as the base delay, backing off to reconnect_max_interval with jitter. */
  PHP_SCYLLADB_INI_STR("reconnect_policy", "constant", reconnect_policy)
  PHP_SCYLLADB_INI_LONG("reconnect_max_interval", PHP_SCYLLADB_DEFAULT_RECONNECT_MAX_INTERVAL,
                        OnUpdatePositiveLong, reconnect_max_interval)

  /* 0 delay disables speculative execution, which is the driver default. */
  PHP_SCYLLADB_INI_LONG("speculative_execution_delay", PHP_SCYLLADB_DEFAULT_SPECULATIVE_DELAY,
                        OnUpdateLongGEZero, speculative_execution_delay)
  PHP_SCYLLADB_INI_LONG("speculative_execution_max", PHP_SCYLLADB_DEFAULT_SPECULATIVE_MAX,
                        OnUpdateLongGEZero, speculative_execution_max)

  /* Event-loop tuning. Change these against a measurement, not a hunch. */
  PHP_SCYLLADB_INI_LONG("coalesce_delay", PHP_SCYLLADB_DEFAULT_COALESCE_DELAY, OnUpdatePositiveLong,
                        coalesce_delay)
  PHP_SCYLLADB_INI_LONG("new_request_ratio", PHP_SCYLLADB_DEFAULT_NEW_REQUEST_RATIO,
                        OnUpdateNewRequestRatio, new_request_ratio)

  /* Each default below is the C driver's own, so exposing the setting changes
   * nothing until an operator sets it. */
  PHP_SCYLLADB_INI_STR("local_address", "", local_address)
  PHP_SCYLLADB_INI_LONG("connection_idle_timeout", PHP_SCYLLADB_DEFAULT_CONNECTION_IDLE_TIMEOUT,
                        OnUpdatePositiveLong, connection_idle_timeout)
  PHP_SCYLLADB_INI_LONG("max_schema_wait_time", PHP_SCYLLADB_DEFAULT_MAX_SCHEMA_WAIT_TIME,
                        OnUpdatePositiveLong, max_schema_wait_time)
  PHP_SCYLLADB_INI_LONG("resolve_timeout", PHP_SCYLLADB_DEFAULT_RESOLVE_TIMEOUT, OnUpdatePositiveLong,
                        resolve_timeout)
  PHP_SCYLLADB_INI_LONG("monitor_reporting_interval", PHP_SCYLLADB_DEFAULT_MONITOR_REPORTING_INTERVAL,
                        OnUpdateLongGEZero, monitor_reporting_interval)
  PHP_SCYLLADB_INI_LONG("queue_size_io", PHP_SCYLLADB_DEFAULT_QUEUE_SIZE_IO, OnUpdatePositiveLong,
                        queue_size_io)
  PHP_SCYLLADB_INI_BOOL("prepare_on_all_hosts", "1", prepare_on_all_hosts)
  PHP_SCYLLADB_INI_BOOL("prepare_on_up_or_add_host", "1", prepare_on_up_or_add_host)
  PHP_SCYLLADB_INI_BOOL("shuffle_replicas", "1", shuffle_replicas)
  PHP_SCYLLADB_INI_BOOL("no_compact", "0", no_compact)
  PHP_SCYLLADB_INI_BOOL("beta_protocol", "0", beta_protocol)

  /* Request tracing. These only matter for statements that turn tracing on. */
  PHP_SCYLLADB_INI_LONG("tracing_consistency", PHP_SCYLLADB_DEFAULT_TRACING_CONSISTENCY_NAME,
                        OnUpdateTracingConsistency, tracing_consistency)
  PHP_SCYLLADB_INI_LONG("tracing_max_wait_time", PHP_SCYLLADB_DEFAULT_TRACING_MAX_WAIT,
                        OnUpdatePositiveLong, tracing_max_wait_time)
  PHP_SCYLLADB_INI_LONG("tracing_retry_wait_time", PHP_SCYLLADB_DEFAULT_TRACING_RETRY_WAIT,
                        OnUpdatePositiveLong, tracing_retry_wait_time)
PHP_INI_END()
// clang-format on

/* Persistent resources.
 *
 * Three resource types — cluster, session, prepared statement — are cached in
 * EG(persistent_list) under a process-keyed hash. The cache is per PHP-FPM
 * worker (or per CLI invocation): each worker has its own EG(persistent_list),
 * so a "persistent" CassCluster/CassSession is NOT shared across the worker
 * pool. Connection counts therefore scale with worker count.
 *
 * Entries survive across requests within the same worker. They are released
 * either when the worker terminates (PHP runs persistent_list_destructors), or
 * when a connect/prepare attempt observes a broken cached future and
 * zend_hash_str_del's the stale entry (see DefaultCluster::connect and
 * DefaultSession::prepare).
 */
static int le_php_scylladb_cluster_res;
int php_le_php_scylladb_cluster(void) { return le_php_scylladb_cluster_res; }
static void php_scylladb_cluster_dtor(zend_resource* rsrc) {
  CassCluster *cluster = (CassCluster *)rsrc->ptr;

  if (cluster) {
    cass_cluster_free(cluster);
    // clang-format off
    PHP_SCYLLADB_G(persistent_clusters)--;
    // clang-format on
    rsrc->ptr = nullptr;
  }
}

static int le_php_scylladb_session_res;
int php_le_php_scylladb_session(void) { return le_php_scylladb_session_res; }
static void php_scylladb_session_dtor(zend_resource* rsrc) {
  php_scylladb_psession *psession = (php_scylladb_psession *)rsrc->ptr;

  if (psession) {
    cass_future_free(psession->future);
    if (psession->session) {
      cass_session_free(psession->session);
      psession->session = nullptr;
    }
    pefree(psession, 1);
    // clang-format off
    PHP_SCYLLADB_G(persistent_sessions)--;
    // clang-format on

    rsrc->ptr = nullptr;
  }
}

static int le_php_scylladb_prepared_statement_res;
int php_le_php_scylladb_prepared_statement(void) { return le_php_scylladb_prepared_statement_res; }
static void php_scylladb_prepared_statement_dtor(zend_resource* rsrc) {
  php_scylladb_pprepared_statement *preparedStmt = (php_scylladb_pprepared_statement *)rsrc->ptr;

  if (preparedStmt) {
    cass_future_free(preparedStmt->future);
    pefree(preparedStmt, 1);

    // clang-format off
    PHP_SCYLLADB_G(persistent_prepared_statements)--;
    // clang-format on

    rsrc->ptr = nullptr;
  }
}

/* Per-request refcounted CassStatement* wrapper. Needed because a Rows
   object can outlive the FutureRows that produced it AND can itself
   spawn child FutureRows via nextPageAsync — all of which need to share
   the same CassStatement* for paging. */
static int le_cass_statement_res;
int php_le_cass_statement(void) { return le_cass_statement_res; }
static void php_scylladb_cass_statement_dtor(zend_resource* rsrc) {
  if (rsrc->ptr) {
    cass_statement_free((CassStatement *)rsrc->ptr);
    rsrc->ptr = nullptr;
  }
}

static void php_scylladb_log(const CassLogMessage *message, void *data);

static void php_scylladb_log_cleanup(void) {
  pthread_rwlock_wrlock(&log_lock);
  if (log_location) {
    free(log_location);
    log_location = nullptr;
  }
  log_target = PHP_SCYLLADB_LOG_STDERR;
  if (log_syslog_opened) {
    closelog();
    log_syslog_opened = false;
  }
  pthread_rwlock_unlock(&log_lock);
}

[[gnu::const]]
static int php_scylladb_syslog_priority(CassLogLevel severity) {
  switch (severity) {
    case CASS_LOG_CRITICAL:
      return LOG_CRIT;
    case CASS_LOG_ERROR:
      return LOG_ERR;
    case CASS_LOG_WARN:
      return LOG_WARNING;
    case CASS_LOG_INFO:
      return LOG_INFO;
    case CASS_LOG_DEBUG:
    case CASS_LOG_TRACE:
      return LOG_DEBUG;
    default:
      return LOG_NOTICE;
  }
}

static void php_scylladb_log_initialize(void) {
  cass_log_set_level(CASS_LOG_ERROR);
  cass_log_set_callback(php_scylladb_log, nullptr);
}

static void php_scylladb_log(const CassLogMessage *message, void *data) {
  char log[MAXPATHLEN + 1];
  uint log_length = 0;
  php_scylladb_log_target target;

  /* Making a copy here because location could be updated by a PHP thread. */
  pthread_rwlock_rdlock(&log_lock);
  target = log_target;
  if (log_location) {
    log_length = MIN(strlen(log_location), MAXPATHLEN);
    memcpy(log, log_location, log_length);
  }
  pthread_rwlock_unlock(&log_lock);

  log[log_length] = '\0';

  if (target == PHP_SCYLLADB_LOG_SYSLOG) {
    /* syslog(3) is thread-safe and adds its own timestamp and ident. */
    syslog(php_scylladb_syslog_priority(message->severity), "%s (%s:%d)", message->message,
           message->file, message->line);
    return;
  }

  if (target == PHP_SCYLLADB_LOG_FILE && log_length > 0) {
    FILE *fd = nullptr;
    fd = fopen(log, "a");
    if (fd) {
      time_t log_time;
      struct tm log_tm = {};
      char log_time_str[64];
      size_t needed = 0;
      char *tmp = nullptr;

      time(&log_time);
      localtime_r(&log_time, &log_tm);
      strftime(log_time_str, sizeof(log_time_str), "%d-%m-%Y %H:%M:%S %Z", &log_tm);

      needed = snprintf(nullptr, 0, "%s [%s] %s (%s:%d)\n", log_time_str,
                        cass_log_level_string(message->severity), message->message, message->file,
                        message->line);

      tmp = (char *)malloc(needed + 1);
      if (tmp) {
        snprintf(tmp, needed + 1, "%s [%s] %s (%s:%d)\n", log_time_str,
                 cass_log_level_string(message->severity),
                 message->message, message->file, message->line);

        fwrite(tmp, 1, needed, fd);
        free(tmp);
      }
      fclose(fd);
      return;
    }
  }

  /* This defaults to using "stderr" instead of "sapi_module.log_message"
   * because there are no guarantees that all implementations of the SAPI
   * logging function are thread-safe. Also: this callback runs on libuv
   * worker threads with no TSRM context, so PHP/Zend API access is forbidden.
   */

  fprintf(stderr, PHP_SCYLLADB_NAME " | [%s] %s (%s:%d)\n", cass_log_level_string(message->severity),
          message->message, message->file, message->line);
}

zend_class_entry *exception_class(CassError rc) {
  switch (rc) {
    case CASS_ERROR_LIB_BAD_PARAMS:
    case CASS_ERROR_LIB_INDEX_OUT_OF_BOUNDS:
    case CASS_ERROR_LIB_INVALID_ITEM_COUNT:
    case CASS_ERROR_LIB_INVALID_VALUE_TYPE:
    case CASS_ERROR_LIB_INVALID_STATEMENT_TYPE:
    case CASS_ERROR_LIB_NAME_DOES_NOT_EXIST:
    case CASS_ERROR_LIB_NULL_VALUE:
    case CASS_ERROR_SSL_INVALID_CERT:
    case CASS_ERROR_SSL_INVALID_PRIVATE_KEY:
    case CASS_ERROR_SSL_NO_PEER_CERT:
    case CASS_ERROR_SSL_INVALID_PEER_CERT:
    case CASS_ERROR_SSL_IDENTITY_MISMATCH:
      return php_scylladb_invalid_argument_exception_ce;
    case CASS_ERROR_LIB_NO_STREAMS:
    case CASS_ERROR_LIB_UNABLE_TO_INIT:
    case CASS_ERROR_LIB_MESSAGE_ENCODE:
    case CASS_ERROR_LIB_HOST_RESOLUTION:
    case CASS_ERROR_LIB_UNEXPECTED_RESPONSE:
    case CASS_ERROR_LIB_REQUEST_QUEUE_FULL:
    case CASS_ERROR_LIB_NO_AVAILABLE_IO_THREAD:
    case CASS_ERROR_LIB_WRITE_ERROR:
    case CASS_ERROR_LIB_NO_HOSTS_AVAILABLE:
    case CASS_ERROR_LIB_UNABLE_TO_SET_KEYSPACE:
    case CASS_ERROR_LIB_UNABLE_TO_DETERMINE_PROTOCOL:
    case CASS_ERROR_LIB_UNABLE_TO_CONNECT:
    case CASS_ERROR_LIB_UNABLE_TO_CLOSE:
      return php_scylladb_runtime_exception_ce;
    case CASS_ERROR_LIB_REQUEST_TIMED_OUT:
      return php_scylladb_timeout_exception_ce;
    case CASS_ERROR_LIB_CALLBACK_ALREADY_SET:
    case CASS_ERROR_LIB_NOT_IMPLEMENTED:
      return php_scylladb_logic_exception_ce;
    case CASS_ERROR_SERVER_SERVER_ERROR:
      return php_scylladb_server_exception_ce;
    case CASS_ERROR_SERVER_PROTOCOL_ERROR:
      return php_scylladb_protocol_exception_ce;
    case CASS_ERROR_SERVER_BAD_CREDENTIALS:
      return php_scylladb_authentication_exception_ce;
    case CASS_ERROR_SERVER_UNAVAILABLE:
      return php_scylladb_unavailable_exception_ce;
    case CASS_ERROR_SERVER_OVERLOADED:
      return php_scylladb_overloaded_exception_ce;
    case CASS_ERROR_SERVER_IS_BOOTSTRAPPING:
      return php_scylladb_is_bootstrapping_exception_ce;
    case CASS_ERROR_SERVER_TRUNCATE_ERROR:
      return php_scylladb_truncate_exception_ce;
    case CASS_ERROR_SERVER_WRITE_TIMEOUT:
      return php_scylladb_write_timeout_exception_ce;
    case CASS_ERROR_SERVER_READ_TIMEOUT:
      return php_scylladb_read_timeout_exception_ce;
    case CASS_ERROR_SERVER_SYNTAX_ERROR:
      return php_scylladb_invalid_syntax_exception_ce;
    case CASS_ERROR_SERVER_UNAUTHORIZED:
      return php_scylladb_unauthorized_exception_ce;
    case CASS_ERROR_SERVER_INVALID_QUERY:
      return php_scylladb_invalid_query_exception_ce;
    case CASS_ERROR_SERVER_CONFIG_ERROR:
      return php_scylladb_configuration_exception_ce;
    case CASS_ERROR_SERVER_ALREADY_EXISTS:
      return php_scylladb_already_exists_exception_ce;
    case CASS_ERROR_SERVER_UNPREPARED:
      return php_scylladb_unprepared_exception_ce;
    default:
      return php_scylladb_runtime_exception_ce;
  }
}

void throw_invalid_argument(const zval *object, const char *object_name, const char *expected_type) {
  if (Z_TYPE_P(object) == IS_OBJECT) {
    zend_string *str = Z_OBJ_HANDLER_P(object, get_class_name)(Z_OBJ_P(object));
   const char * cls_name = str->val;
    size_t cls_len = str->len;
    if (cls_name) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "%s must be %s, an instance of %.*s given", object_name,
                              expected_type, (int)cls_len, cls_name);
      zend_string_release(str);
    } else {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "%s must be %s, an instance of Unknown Class given", object_name,
                              expected_type);
    }
  } else if (Z_TYPE_P(object) == IS_STRING) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                            "%s must be %s, %Z given", object_name, expected_type, object);
  } else {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0, "%s must be %s, %Z given",
                            object_name, expected_type, object);
  }
}

PHP_INI_MH(OnUpdateLogLevel) {
  /* If TSRM is enabled then the last thread to update this wins */

  if (!new_value) {
    return SUCCESS;
  }

  if (strncasecmp(ZSTR_VAL(new_value), ZEND_STRS("critical")) == 0) {
    cass_log_set_level(CASS_LOG_CRITICAL);
  } else if (strncasecmp(ZSTR_VAL(new_value), ZEND_STRS("error")) == 0) {
    cass_log_set_level(CASS_LOG_ERROR);
  } else if (strncasecmp(ZSTR_VAL(new_value), ZEND_STRS("warn")) == 0) {
    cass_log_set_level(CASS_LOG_WARN);
  } else if (strncasecmp(ZSTR_VAL(new_value), ZEND_STRS("info")) == 0) {
    cass_log_set_level(CASS_LOG_INFO);
  } else if (strncasecmp(ZSTR_VAL(new_value), ZEND_STRS("debug")) == 0) {
    cass_log_set_level(CASS_LOG_DEBUG);
  } else if (strncasecmp(ZSTR_VAL(new_value), ZEND_STRS("trace")) == 0) {
    cass_log_set_level(CASS_LOG_TRACE);
  } else {
    php_error_docref(nullptr, E_NOTICE, PHP_SCYLLADB_NAME " | Unknown log level '%s', using 'ERROR'",
                     ZSTR_VAL(new_value));
    cass_log_set_level(CASS_LOG_ERROR);
  }

  return SUCCESS;
}

PHP_INI_MH(OnUpdateLog) {
  /* If TSRM is enabled then the last thread to update this wins */

  pthread_rwlock_wrlock(&log_lock);
  if (log_location) {
    free(log_location);
    log_location = nullptr;
  }
  log_target = PHP_SCYLLADB_LOG_STDERR;

  if (new_value && ZSTR_LEN(new_value) > 0) {
    if (zend_string_equals_literal_ci(new_value, "syslog")) {
      log_target = PHP_SCYLLADB_LOG_SYSLOG;
      if (!log_syslog_opened) {
        /* LOG_NDELAY so the socket exists before a libuv thread first logs. */
        openlog(PHP_SCYLLADB_NAME, LOG_PID | LOG_NDELAY, LOG_USER);
        log_syslog_opened = true;
      }
    } else if (!zend_string_equals_literal_ci(new_value, "stderr")) {
      char realpath[MAXPATHLEN + 1];
      if (VCWD_REALPATH(ZSTR_VAL(new_value), realpath)) {
        log_location = strdup(realpath);
      } else {
        log_location = strdup(ZSTR_VAL(new_value));
      }
      if (log_location) {
        log_target = PHP_SCYLLADB_LOG_FILE;
      }
    }
  }
  pthread_rwlock_unlock(&log_lock);

  return SUCCESS;
}

/* Reject-and-keep-default rather than clamp. Returning FAILURE makes the Zend
 * INI machinery restore the entry's default string, so ini_get() and phpinfo()
 * report the value the driver actually uses. Leaving `out` untouched keeps the
 * default that GINIT seeded. Startup is not aborted either way. */
static zend_result php_scylladb_ini_bounded_long(const zend_ini_entry *entry,
                                                 const zend_string *new_value, zend_long *out,
                                                 zend_long min, zend_long max) {
  if (new_value == nullptr) {
    return SUCCESS;
  }

  zend_long value = ZEND_STRTOL(ZSTR_VAL(new_value), nullptr, 10);

  if (value < min || value > max) {
    php_error_docref(nullptr, E_WARNING,
                     PHP_SCYLLADB_NAME " | %s must be between " ZEND_LONG_FMT " and " ZEND_LONG_FMT
                                       ", ignoring '%s' and using the default",
                     ZSTR_VAL(entry->name), min, max, ZSTR_VAL(new_value));
    return FAILURE;
  }

  *out = value;
  return SUCCESS;
}

PHP_INI_MH(OnUpdatePositiveLong) {
  return php_scylladb_ini_bounded_long(entry, new_value, (zend_long *)ZEND_INI_GET_ADDR(), 1,
                                       ZEND_LONG_MAX);
}

PHP_INI_MH(OnUpdatePort) {
  return php_scylladb_ini_bounded_long(entry, new_value, (zend_long *)ZEND_INI_GET_ADDR(), 1, 65535);
}

PHP_INI_MH(OnUpdateProtocolVersion) {
  return php_scylladb_ini_bounded_long(entry, new_value, (zend_long *)ZEND_INI_GET_ADDR(), 1, 5);
}

PHP_INI_MH(OnUpdateTracingConsistency) {
  if (!new_value) {
    return SUCCESS;
  }

  int32_t consistency = php_scylladb_consistency_from_name(ZSTR_VAL(new_value));

  if (consistency < 0) {
    php_error_docref(nullptr, E_WARNING,
                     PHP_SCYLLADB_NAME " | Unknown tracing consistency '%s', using '%s'",
                     ZSTR_VAL(new_value), PHP_SCYLLADB_DEFAULT_TRACING_CONSISTENCY_NAME);
    return FAILURE;
  }

  *(zend_long *)ZEND_INI_GET_ADDR() = consistency;
  return SUCCESS;
}

PHP_INI_MH(OnUpdateNewRequestRatio) {
  /* cass_cluster_set_new_request_ratio documents a range of 1 to 100. */
  return php_scylladb_ini_bounded_long(entry, new_value, (zend_long *)ZEND_INI_GET_ADDR(), 1, 100);
}

PHP_INI_MH(OnUpdatePersistentMax) {
  /* -1 means unlimited, 0 disables caching for that resource kind. */
  return php_scylladb_ini_bounded_long(entry, new_value, (zend_long *)ZEND_INI_GET_ADDR(), -1,
                                       ZEND_LONG_MAX);
}

PHP_INI_MH(OnUpdateDefaultConsistency) {
  if (!new_value) {
    return SUCCESS;
  }

  int32_t consistency = php_scylladb_consistency_from_name(ZSTR_VAL(new_value));

  if (consistency < 0) {
    /* FAILURE restores the default string on the entry, so ini_get() agrees
     * with the value GINIT left in the globals. */
    php_error_docref(nullptr, E_WARNING,
                     PHP_SCYLLADB_NAME " | Unknown consistency '%s', using '%s'",
                     ZSTR_VAL(new_value), PHP_SCYLLADB_DEFAULT_CONSISTENCY_NAME);
    return FAILURE;
  }

  /* ZEND_INI_GET_ADDR, not PHP_SCYLLADB_G: under ZTS the latter resolves the
   * thread-local pointer through TSRMG, which is the wrong way to reach module
   * globals from an INI handler. Every other handler here uses the same idiom. */
  *(zend_long *)ZEND_INI_GET_ADDR() = consistency;
  return SUCCESS;
}

bool php_scylladb_persistent_can_cache(php_scylladb_persistent_kind kind) {
  zend_long limit;
  unsigned int used;
  bool *warned;
  const char *what;

  switch (kind) {
    case PHP_SCYLLADB_PERSISTENT_CLUSTERS:
      limit = PHP_SCYLLADB_G(max_persistent_clusters);
      used = PHP_SCYLLADB_G(persistent_clusters);
      warned = &PHP_SCYLLADB_G(warned_cluster_cap);
      what = "clusters";
      break;
    case PHP_SCYLLADB_PERSISTENT_SESSIONS:
      limit = PHP_SCYLLADB_G(max_persistent_sessions);
      used = PHP_SCYLLADB_G(persistent_sessions);
      warned = &PHP_SCYLLADB_G(warned_session_cap);
      what = "sessions";
      break;
    case PHP_SCYLLADB_PERSISTENT_PREPARED_STATEMENTS:
      limit = PHP_SCYLLADB_G(max_persistent_prepared_statements);
      used = PHP_SCYLLADB_G(persistent_prepared_statements);
      warned = &PHP_SCYLLADB_G(warned_prepared_statement_cap);
      what = "prepared_statements";
      break;
    default:
      return true;
  }

  if (limit < 0 || (zend_long)used < limit) {
    return true;
  }

  if (!*warned) {
    *warned = true;
    php_error_docref(nullptr, E_WARNING,
                     PHP_SCYLLADB_NAME " | " PHP_SCYLLADB_NAME
                                       ".max_persistent_%s reached (" ZEND_LONG_FMT
                                       "); further %s are rebuilt on every request",
                     what, limit, what);
  }

  return false;
}

static PHP_GINIT_FUNCTION(php_scylladb) {
  php_scylladb_globals->reactor = nullptr;
  php_scylladb_globals->uuid_gen = nullptr;
  php_scylladb_globals->uuid_gen_pid = 0;
  php_scylladb_globals->persistent_clusters = 0;
  php_scylladb_globals->persistent_sessions = 0;
  php_scylladb_globals->persistent_prepared_statements = 0;
  php_scylladb_globals->expose_credentials = false;
  php_scylladb_globals->warned_cluster_cap = false;
  php_scylladb_globals->warned_session_cap = false;
  php_scylladb_globals->warned_prepared_statement_cap = false;

  /* GINIT runs before REGISTER_INI_ENTRIES, so a directive the handler rejects
   * leaves its default here. These must match the PHP_SCYLLADB_DEFAULT_* strings
   * in the INI table; the shared _N macros are what guarantees that. */
  php_scylladb_globals->max_persistent_clusters = PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_CLUSTERS_N;
  php_scylladb_globals->max_persistent_sessions = PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_SESSIONS_N;
  php_scylladb_globals->max_persistent_prepared_statements =
      PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_PREPARED_N;
  php_scylladb_globals->port = PHP_SCYLLADB_DEFAULT_PORT_N;
  php_scylladb_globals->connect_timeout = PHP_SCYLLADB_DEFAULT_CONNECT_TIMEOUT_N;
  php_scylladb_globals->request_timeout = PHP_SCYLLADB_DEFAULT_REQUEST_TIMEOUT_N;
  php_scylladb_globals->default_consistency = PHP_SCYLLADB_DEFAULT_CONSISTENCY;
  php_scylladb_globals->default_page_size = PHP_SCYLLADB_DEFAULT_PAGE_SIZE_N;
  php_scylladb_globals->protocol_version = PHP_SCYLLADB_DEFAULT_PROTOCOL_VERSION_N;
  php_scylladb_globals->io_threads = PHP_SCYLLADB_DEFAULT_IO_THREADS_N;
  php_scylladb_globals->core_connections_per_host = PHP_SCYLLADB_DEFAULT_CORE_CONNECTIONS_PER_HOST_N;
  php_scylladb_globals->max_connections_per_host = PHP_SCYLLADB_DEFAULT_MAX_CONNECTIONS_PER_HOST_N;
  php_scylladb_globals->reconnect_interval = PHP_SCYLLADB_DEFAULT_RECONNECT_INTERVAL_N;
  php_scylladb_globals->connection_heartbeat_interval = PHP_SCYLLADB_DEFAULT_HEARTBEAT_INTERVAL_N;
  php_scylladb_globals->tcp_keepalive_delay = PHP_SCYLLADB_DEFAULT_TCP_KEEPALIVE_DELAY_N;
  php_scylladb_globals->reconnect_max_interval = PHP_SCYLLADB_DEFAULT_RECONNECT_MAX_INTERVAL_N;
  php_scylladb_globals->speculative_execution_delay = PHP_SCYLLADB_DEFAULT_SPECULATIVE_DELAY_N;
  php_scylladb_globals->speculative_execution_max = PHP_SCYLLADB_DEFAULT_SPECULATIVE_MAX_N;
  php_scylladb_globals->coalesce_delay = PHP_SCYLLADB_DEFAULT_COALESCE_DELAY_N;
  php_scylladb_globals->new_request_ratio = PHP_SCYLLADB_DEFAULT_NEW_REQUEST_RATIO_N;
  php_scylladb_globals->connection_idle_timeout = PHP_SCYLLADB_DEFAULT_CONNECTION_IDLE_TIMEOUT_N;
  php_scylladb_globals->max_schema_wait_time = PHP_SCYLLADB_DEFAULT_MAX_SCHEMA_WAIT_TIME_N;
  php_scylladb_globals->resolve_timeout = PHP_SCYLLADB_DEFAULT_RESOLVE_TIMEOUT_N;
  php_scylladb_globals->monitor_reporting_interval = PHP_SCYLLADB_DEFAULT_MONITOR_REPORTING_INTERVAL_N;
  php_scylladb_globals->queue_size_io = PHP_SCYLLADB_DEFAULT_QUEUE_SIZE_IO_N;
  php_scylladb_globals->tracing_consistency = PHP_SCYLLADB_DEFAULT_TRACING_CONSISTENCY;
  php_scylladb_globals->tracing_max_wait_time = PHP_SCYLLADB_DEFAULT_TRACING_MAX_WAIT_N;
  php_scylladb_globals->tracing_retry_wait_time = PHP_SCYLLADB_DEFAULT_TRACING_RETRY_WAIT_N;
  ZVAL_UNDEF(&php_scylladb_globals->type_varchar);
  ZVAL_UNDEF(&php_scylladb_globals->type_text);
  ZVAL_UNDEF(&php_scylladb_globals->type_blob);
  ZVAL_UNDEF(&php_scylladb_globals->type_ascii);
  ZVAL_UNDEF(&php_scylladb_globals->type_bigint);
  ZVAL_UNDEF(&php_scylladb_globals->type_smallint);
  ZVAL_UNDEF(&php_scylladb_globals->type_counter);
  ZVAL_UNDEF(&php_scylladb_globals->type_int);
  ZVAL_UNDEF(&php_scylladb_globals->type_varint);
  ZVAL_UNDEF(&php_scylladb_globals->type_boolean);
  ZVAL_UNDEF(&php_scylladb_globals->type_decimal);
  ZVAL_UNDEF(&php_scylladb_globals->type_double);
  ZVAL_UNDEF(&php_scylladb_globals->type_float);
  ZVAL_UNDEF(&php_scylladb_globals->type_inet);
  ZVAL_UNDEF(&php_scylladb_globals->type_timestamp);
  ZVAL_UNDEF(&php_scylladb_globals->type_uuid);
  ZVAL_UNDEF(&php_scylladb_globals->type_timeuuid);
}

static PHP_GSHUTDOWN_FUNCTION(php_scylladb) {
  /* Process/thread end: free the reactor's eventfd + mutex (kept alive across
     requests). RSHUTDOWN already drained its request-scoped state. */
  if (php_scylladb_globals->reactor) {
    php_scylladb_reactor_destroy(php_scylladb_globals->reactor);
    php_scylladb_globals->reactor = nullptr;
  }

  if (php_scylladb_globals->uuid_gen) {
    cass_uuid_gen_free(php_scylladb_globals->uuid_gen);
  }
}


PHP_MINIT_FUNCTION(php_scylladb) {
  php_scylladb_log_initialize();

  REGISTER_INI_ENTRIES();

  le_php_scylladb_cluster_res = zend_register_list_destructors_ex(
      nullptr, php_scylladb_cluster_dtor, PHP_SCYLLADB_CLUSTER_RES_NAME, module_number);
  le_php_scylladb_session_res = zend_register_list_destructors_ex(
      nullptr, php_scylladb_session_dtor, PHP_SCYLLADB_SESSION_RES_NAME, module_number);

  le_php_scylladb_prepared_statement_res =
      zend_register_list_destructors_ex(nullptr, php_scylladb_prepared_statement_dtor,
                                        PHP_SCYLLADB_PREPARED_STATEMENT_RES_NAME, module_number);

  le_cass_statement_res = zend_register_list_destructors_ex(
      php_scylladb_cass_statement_dtor, nullptr, "CassStatement", module_number);

  /* All PHP classes register themselves at file scope via
   * PHP_SCYLLADB_REGISTER_CLASS(...) in their respective .cpp/.c files. This
   * walks the resulting list once, topo-sorts by parent FQN, and registers
   * each class in dependency order. Adding a class no longer requires
   * editing this file. */
  php_scylladb_class_registry_minit();

  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(php_scylladb) {
  /* Detach the libuv-thread log callback before tearing down the lock so that
   * a late log message from a worker thread cannot trample on freed memory. */
  cass_log_set_callback(nullptr, nullptr);

  UNREGISTER_INI_ENTRIES();
  php_scylladb_log_cleanup();
  return SUCCESS;
}

PHP_RINIT_FUNCTION(php_scylladb) {
#define XX_SCALAR(name, value) ZVAL_UNDEF(&PHP_SCYLLADB_G(type_## name));
  PHP_SCYLLADB_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR

  PHP_SCYLLADB_G(warned_cluster_cap) = false;
  PHP_SCYLLADB_G(warned_session_cap) = false;
  PHP_SCYLLADB_G(warned_prepared_statement_cap) = false;

  return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(php_scylladb) {
  /* Reset the shared async reactor: drain request-scoped futures (so no driver
     callback fires against freed request memory) and drop the cached stream,
     but keep its eventfd + mutex for the next request. */
  if (PHP_SCYLLADB_G(reactor)) {
    php_scylladb_reactor_reset(PHP_SCYLLADB_G(reactor));
  }

#define XX_SCALAR(name, value) zval_ptr_dtor(&PHP_SCYLLADB_G(type_## name));
  PHP_SCYLLADB_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR

  return SUCCESS;
}

PHP_MINFO_FUNCTION(php_scylladb) {
  char buf[256];
  php_info_print_table_start();

  php_info_print_table_row(2, PHP_SCYLLADB_NAMESPACE " support", "enabled");

  snprintf(buf, sizeof(buf), "%d.%d.%d", CASS_VERSION_MAJOR, CASS_VERSION_MINOR,
           CASS_VERSION_PATCH);
  php_info_print_table_row(2, "C/C++ driver version", buf);

  php_info_print_table_row(2, "PHP driver extension",
                           "customized for persistent prepared statements");

  snprintf(buf, sizeof(buf), "%d", PHP_SCYLLADB_G(persistent_clusters));
  php_info_print_table_row(2, "Persistent Clusters", buf);

  snprintf(buf, sizeof(buf), "%d", PHP_SCYLLADB_G(persistent_sessions));
  php_info_print_table_row(2, "Persistent Sessions", buf);

  snprintf(buf, sizeof(buf), "%d", PHP_SCYLLADB_G(persistent_prepared_statements));
  php_info_print_table_row(2, "Persistent Prepared Statements", buf);

  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
