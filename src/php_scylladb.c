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

#include <fcntl.h>
#include <php_scylladb.h>
#include <php_scylladb_globals.h>
#include <php_scylladb_types.h>
#include <php_ini.h>
#include <uv.h>
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

static char *log_location = nullptr;
static uv_rwlock_t log_lock;

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

// clang-format off
PHP_INI_BEGIN()
  PHP_INI_ENTRY(PHP_SCYLLADB_NAME ".log", PHP_SCYLLADB_DEFAULT_LOG, PHP_INI_SYSTEM, OnUpdateLog)
  PHP_INI_ENTRY(PHP_SCYLLADB_NAME ".log_level", PHP_SCYLLADB_DEFAULT_LOG_LEVEL, PHP_INI_SYSTEM, OnUpdateLogLevel)
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
  uv_rwlock_destroy(&log_lock);
  if (log_location) {
    free(log_location);
    log_location = nullptr;
  }
}

static void php_scylladb_log_initialize(void) {
  uv_rwlock_init(&log_lock);
  cass_log_set_level(CASS_LOG_ERROR);
  cass_log_set_callback(php_scylladb_log, nullptr);
}

static void php_scylladb_log(const CassLogMessage *message, void *data) {
  char log[MAXPATHLEN + 1];
  uint log_length = 0;

  /* Making a copy here because location could be updated by a PHP thread. */
  uv_rwlock_rdlock(&log_lock);
  if (log_location) {
    log_length = MIN(strlen(log_location), MAXPATHLEN);
    memcpy(log, log_location, log_length);
  }
  uv_rwlock_rdunlock(&log_lock);

  log[log_length] = '\0';

  if (log_length > 0) {
    FILE *fd = nullptr;
    fd = fopen(log, "a");
    if (fd) {
      time_t log_time;
      struct tm log_tm = {0};
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

  uv_rwlock_wrlock(&log_lock);
  if (log_location) {
    free(log_location);
    log_location = nullptr;
  }
  if (new_value) {
    if (strcmp(ZSTR_VAL(new_value), "syslog") != 0) {
      char realpath[MAXPATHLEN + 1];
      if (VCWD_REALPATH(ZSTR_VAL(new_value), realpath)) {
        log_location = strdup(realpath);
      } else {
        log_location = strdup(ZSTR_VAL(new_value));
      }
    } else {
      log_location = strdup(ZSTR_VAL(new_value));
    }
  }
  uv_rwlock_wrunlock(&log_lock);

  return SUCCESS;
}

static PHP_GINIT_FUNCTION(php_scylladb) {
  php_scylladb_globals->uuid_gen = nullptr;
  php_scylladb_globals->uuid_gen_pid = 0;
  php_scylladb_globals->persistent_clusters = 0;
  php_scylladb_globals->persistent_sessions = 0;
  php_scylladb_globals->persistent_prepared_statements = 0;
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

  return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(php_scylladb) {
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
