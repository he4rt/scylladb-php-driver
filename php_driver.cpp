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

#include <src/Cluster/Cluster.h>
#include <src/DateTime/DateTimeInternal.h>

#include <fcntl.h>
#include <php_driver.h>
#include <php_driver_globals.h>
#include <php_driver_types.h>
#include <php_ini.h>
#include <util/ref.h>
#include <uv.h>
#include <version.h>

#include <ctime>


BEGIN_EXTERN_C()
#include <ext/standard/info.h>

#define PHP_DRIVER_SCALAR_TYPES_MAP(XX)    \
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
#define PHP_DRIVER_CLUSTER_RES_NAME PHP_DRIVER_NAMESPACE " Cluster"
#define PHP_DRIVER_SESSION_RES_NAME PHP_DRIVER_NAMESPACE " Session"
#define PHP_DRIVER_PREPARED_STATEMENT_RES_NAME PHP_DRIVER_NAMESPACE " PreparedStatement"

static uv_once_t log_once = UV_ONCE_INIT;
static char *log_location = nullptr;
static uv_rwlock_t log_lock;

#if CURRENT_CPP_DRIVER_VERSION < CPP_DRIVER_VERSION(2, 16, 2)
#error C/C++ driver version 2.16.2 or greater required
#endif

ZEND_DECLARE_MODULE_GLOBALS(php_driver)

static PHP_GINIT_FUNCTION(php_driver);
static PHP_GSHUTDOWN_FUNCTION(php_driver);

// clang-format off
const zend_function_entry php_driver_functions[] = {
    PHP_FE_END /* Must be the last line in php_driver_functions[] */
};
// clang-format on

// clang-format off
static zend_module_dep php_driver_deps[] = {
  ZEND_MOD_REQUIRED("spl")
  ZEND_MOD_REQUIRED("date")
  ZEND_MOD_END
};
// clang-format on

// clang-format off
zend_module_entry php_driver_module_entry = {
  STANDARD_MODULE_HEADER_EX,
  nullptr,
  php_driver_deps,
  PHP_DRIVER_NAME,
  php_driver_functions,      /* Functions */
  PHP_MINIT(php_driver),     /* MINIT */
  PHP_MSHUTDOWN(php_driver), /* MSHUTDOWN */
  PHP_RINIT(php_driver),     /* RINIT */
  PHP_RSHUTDOWN(php_driver), /* RSHUTDOWN */
  PHP_MINFO(php_driver),     /* MINFO */
  PHP_DRIVER_VERSION,
  PHP_MODULE_GLOBALS(php_driver),
  PHP_GINIT(php_driver),
  PHP_GSHUTDOWN(php_driver),
  nullptr,
  STANDARD_MODULE_PROPERTIES_EX
};
// clang-format on

#ifdef COMPILE_DL_CASSANDRA
BEGIN_EXTERN_C()

ZEND_DLEXPORT zend_module_entry *get_module() { return &php_driver_module_entry; }
END_EXTERN_C()

#endif

// clang-format off
PHP_INI_BEGIN()
  PHP_INI_ENTRY(PHP_DRIVER_NAME ".log", PHP_DRIVER_DEFAULT_LOG, PHP_INI_ALL, OnUpdateLog)
  PHP_INI_ENTRY(PHP_DRIVER_NAME ".log_level", PHP_DRIVER_DEFAULT_LOG_LEVEL, PHP_INI_ALL, OnUpdateLogLevel)
PHP_INI_END()
// clang-format on

static int le_php_driver_cluster_res;
int php_le_php_driver_cluster() { return le_php_driver_cluster_res; }
static void php_driver_cluster_dtor(zend_resource* rsrc) {
  auto *cluster = (CassCluster *)rsrc->ptr;

  if (cluster) {
    cass_cluster_free(cluster);
    // clang-format off
    PHP_DRIVER_G(persistent_clusters)--;
    // clang-format on
    rsrc->ptr = nullptr;
  }
}

static int le_php_driver_session_res;
int php_le_php_driver_session() { return le_php_driver_session_res; }
static void php_driver_session_dtor(zend_resource* rsrc) {
  auto *psession = (php_driver_psession *)rsrc->ptr;

  if (psession) {
    cass_future_free(psession->future);
    php_driver_del_peref(&psession->session, 1);
    pefree(psession, 1);
    // clang-format off
    PHP_DRIVER_G(persistent_sessions)--;
    // clang-format on

    rsrc->ptr = nullptr;
  }
}

static int le_php_driver_prepared_statement_res;
int php_le_php_driver_prepared_statement() { return le_php_driver_prepared_statement_res; }
static void php_driver_prepared_statement_dtor(zend_resource* rsrc) {
  auto *preparedStmt = (php_driver_pprepared_statement *)rsrc->ptr;

  if (preparedStmt) {
    cass_future_free(preparedStmt->future);
    php_driver_del_peref(&preparedStmt->ref, 1);
    pefree(preparedStmt, 1);

    // clang-format off
    PHP_DRIVER_G(persistent_prepared_statements)--;
    // clang-format on

    rsrc->ptr = nullptr;
  }
}

static void php_driver_log(const CassLogMessage *message, void *data);

static void php_driver_log_cleanup() {
  uv_rwlock_destroy(&log_lock);
  if (log_location) {
    free(log_location);
    log_location = nullptr;
  }
}

static void php_driver_log_initialize() {
  uv_rwlock_init(&log_lock);
  cass_log_set_level(CASS_LOG_ERROR);
  cass_log_set_callback(php_driver_log, nullptr);
}

static void php_driver_log(const CassLogMessage *message, void *data) {
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
      struct tm log_tm {};
      char log_time_str[64];
      size_t needed = 0;
      char *tmp = nullptr;

      time(&log_time);
      php_localtime_r(&log_time, &log_tm);
      strftime(log_time_str, sizeof(log_time_str), "%d-%m-%Y %H:%M:%S %Z", &log_tm);

      needed = snprintf(nullptr, 0, "%s [%s] %s (%s:%d)%s", log_time_str,
                        cass_log_level_string(message->severity), message->message, message->file,
                        message->line, PHP_EOL);

      tmp = (char *)malloc(needed + 1);
      sprintf(tmp, "%s [%s] %s (%s:%d)%s", log_time_str, cass_log_level_string(message->severity),
              message->message, message->file, message->line, PHP_EOL);

      fwrite(tmp, 1, needed, fd);
      free(tmp);
      fclose(fd);
      return;
    }
  }

  /* This defaults to using "stderr" instead of "sapi_module.log_message"
   * because there are no guarantees that all implementations of the SAPI
   * logging function are thread-safe.
   */

  fprintf(stderr, PHP_DRIVER_NAME " | [%s] %s (%s:%d)%s", cass_log_level_string(message->severity),
          message->message, message->file, message->line, PHP_EOL);
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
      return php_driver_invalid_argument_exception_ce;
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
      return php_driver_runtime_exception_ce;
    case CASS_ERROR_LIB_REQUEST_TIMED_OUT:
      return php_driver_timeout_exception_ce;
    case CASS_ERROR_LIB_CALLBACK_ALREADY_SET:
    case CASS_ERROR_LIB_NOT_IMPLEMENTED:
      return php_driver_logic_exception_ce;
    case CASS_ERROR_SERVER_SERVER_ERROR:
      return php_driver_server_exception_ce;
    case CASS_ERROR_SERVER_PROTOCOL_ERROR:
      return php_driver_protocol_exception_ce;
    case CASS_ERROR_SERVER_BAD_CREDENTIALS:
      return php_driver_authentication_exception_ce;
    case CASS_ERROR_SERVER_UNAVAILABLE:
      return php_driver_unavailable_exception_ce;
    case CASS_ERROR_SERVER_OVERLOADED:
      return php_driver_overloaded_exception_ce;
    case CASS_ERROR_SERVER_IS_BOOTSTRAPPING:
      return php_driver_is_bootstrapping_exception_ce;
    case CASS_ERROR_SERVER_TRUNCATE_ERROR:
      return php_driver_truncate_exception_ce;
    case CASS_ERROR_SERVER_WRITE_TIMEOUT:
      return php_driver_write_timeout_exception_ce;
    case CASS_ERROR_SERVER_READ_TIMEOUT:
      return php_driver_read_timeout_exception_ce;
    case CASS_ERROR_SERVER_SYNTAX_ERROR:
      return php_driver_invalid_syntax_exception_ce;
    case CASS_ERROR_SERVER_UNAUTHORIZED:
      return php_driver_unauthorized_exception_ce;
    case CASS_ERROR_SERVER_INVALID_QUERY:
      return php_driver_invalid_query_exception_ce;
    case CASS_ERROR_SERVER_CONFIG_ERROR:
      return php_driver_configuration_exception_ce;
    case CASS_ERROR_SERVER_ALREADY_EXISTS:
      return php_driver_already_exists_exception_ce;
    case CASS_ERROR_SERVER_UNPREPARED:
      return php_driver_unprepared_exception_ce;
    default:
      return php_driver_runtime_exception_ce;
  }
}

void throw_invalid_argument(const zval *object, const char *object_name, const char *expected_type) {
  if (Z_TYPE_P(object) == IS_OBJECT) {
    zend_string *str = Z_OBJ_HANDLER_P(object, get_class_name)(Z_OBJ_P(object));
   const char * cls_name = str->val;
    size_t cls_len = str->len;
    if (cls_name) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "%s must be %s, an instance of %.*s given", object_name,
                              expected_type, static_cast<int>(cls_len), cls_name);
      zend_string_release(str);
    } else {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "%s must be %s, an instance of Unknown Class given", object_name,
                              expected_type);
    }
  } else if (Z_TYPE_P(object) == IS_STRING) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "%s must be %s, %Z given", object_name, expected_type, object);
  } else {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0, "%s must be %s, %Z given",
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
    php_error_docref(nullptr, E_NOTICE, PHP_DRIVER_NAME " | Unknown log level '%s', using 'ERROR'",
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

static PHP_GINIT_FUNCTION(php_driver) {
  uv_once(&log_once, php_driver_log_initialize);

  php_driver_globals->uuid_gen = nullptr;
  php_driver_globals->uuid_gen_pid = 0;
  php_driver_globals->persistent_clusters = 0;
  php_driver_globals->persistent_sessions = 0;
  php_driver_globals->persistent_prepared_statements = 0;
  ZVAL_UNDEF(&php_driver_globals->type_varchar);
  ZVAL_UNDEF(&php_driver_globals->type_text);
  ZVAL_UNDEF(&php_driver_globals->type_blob);
  ZVAL_UNDEF(&php_driver_globals->type_ascii);
  ZVAL_UNDEF(&php_driver_globals->type_bigint);
  ZVAL_UNDEF(&php_driver_globals->type_smallint);
  ZVAL_UNDEF(&php_driver_globals->type_counter);
  ZVAL_UNDEF(&php_driver_globals->type_int);
  ZVAL_UNDEF(&php_driver_globals->type_varint);
  ZVAL_UNDEF(&php_driver_globals->type_boolean);
  ZVAL_UNDEF(&php_driver_globals->type_decimal);
  ZVAL_UNDEF(&php_driver_globals->type_double);
  ZVAL_UNDEF(&php_driver_globals->type_float);
  ZVAL_UNDEF(&php_driver_globals->type_inet);
  ZVAL_UNDEF(&php_driver_globals->type_timestamp);
  ZVAL_UNDEF(&php_driver_globals->type_uuid);
  ZVAL_UNDEF(&php_driver_globals->type_timeuuid);
}

static PHP_GSHUTDOWN_FUNCTION(php_driver) {
  if (php_driver_globals->uuid_gen) {
    cass_uuid_gen_free(php_driver_globals->uuid_gen);
  }
  php_driver_log_cleanup();
}


PHP_MINIT_FUNCTION(php_driver) {
  REGISTER_INI_ENTRIES();

  le_php_driver_cluster_res = zend_register_list_destructors_ex(
      nullptr, php_driver_cluster_dtor, PHP_DRIVER_CLUSTER_RES_NAME, module_number);
  le_php_driver_session_res = zend_register_list_destructors_ex(
      nullptr, php_driver_session_dtor, PHP_DRIVER_SESSION_RES_NAME, module_number);

  le_php_driver_prepared_statement_res =
      zend_register_list_destructors_ex(nullptr, php_driver_prepared_statement_dtor,
                                        PHP_DRIVER_PREPARED_STATEMENT_RES_NAME, module_number);

  php_driver_define_Exception();
  php_driver_define_InvalidArgumentException();
  php_driver_define_DomainException();
  php_driver_define_RuntimeException();
  php_driver_define_TimeoutException();
  php_driver_define_LogicException();
  php_driver_define_ExecutionException();
  php_driver_define_ReadTimeoutException();
  php_driver_define_WriteTimeoutException();
  php_driver_define_UnavailableException();
  php_driver_define_TruncateException();
  php_driver_define_ValidationException();
  php_driver_define_InvalidQueryException();
  php_driver_define_InvalidSyntaxException();
  php_driver_define_UnauthorizedException();
  php_driver_define_UnpreparedException();
  php_driver_define_ConfigurationException();
  php_driver_define_AlreadyExistsException();
  php_driver_define_AuthenticationException();
  php_driver_define_ProtocolException();
  php_driver_define_ServerException();
  php_driver_define_IsBootstrappingException();
  php_driver_define_OverloadedException();
  php_driver_define_RangeException();
  php_driver_define_DivideByZeroException();

  php_driver_define_Value();
  php_driver_define_Numeric();
  php_driver_define_Bigint();
  php_driver_define_Smallint();
  php_driver_define_Tinyint();
  php_driver_define_Blob();
  php_driver_define_Decimal();
  php_driver_define_Float();
  php_driver_define_Inet();
  php_driver_define_Timestamp();
  php_driver_define_Date();
  php_driver_define_Time();
  php_driver_define_UuidInterface();
  php_driver_define_Timeuuid();
  php_driver_define_Uuid();
  php_driver_define_Varint();
  php_driver_define_Custom();
  php_driver_define_Duration();

  php_driver_define_Set();
  php_driver_define_Map();
  php_driver_define_Collection();
  php_driver_define_Tuple();
  php_driver_define_UserTypeValue();

  php_driver_define_Core();
  php_driver_define_Cluster();
  php_driver_define_DefaultCluster();
  php_driver_define_ClusterBuilder();
  php_driver_define_Future();
  php_driver_define_FuturePreparedStatement();
  php_driver_define_FutureRows();
  php_driver_define_FutureSession();
  php_driver_define_FutureValue();
  php_driver_define_FutureClose();
  php_driver_define_Session();
  php_driver_define_DefaultSession();
  php_driver_define_SSLOptions();
  php_driver_define_SSLOptionsBuilder();
  php_driver_define_Statement();
  php_driver_define_SimpleStatement();
  php_driver_define_PreparedStatement();
  php_driver_define_BatchStatement();
  php_driver_define_ExecutionOptions();
  php_driver_define_Rows();

  php_driver_define_Schema();
  php_driver_define_DefaultSchema();
  php_driver_define_Keyspace();
  php_driver_define_DefaultKeyspace();
  php_driver_define_Table();
  php_driver_define_DefaultTable();
  php_driver_define_Column();
  php_driver_define_DefaultColumn();
  php_driver_define_Index();
  php_driver_define_DefaultIndex();
  php_driver_define_MaterializedView();
  php_driver_define_DefaultMaterializedView();
  php_driver_define_Function();
  php_driver_define_DefaultFunction();
  php_driver_define_Aggregate();
  php_driver_define_DefaultAggregate();

  php_driver_define_Type();
  php_driver_define_TypeScalar();
  php_driver_define_TypeCollection();
  php_driver_define_TypeSet();
  php_driver_define_TypeMap();
  php_driver_define_TypeTuple();
  php_driver_define_TypeUserType();
  php_driver_define_TypeCustom();

  auto * retry_policy_interface = php_scylladb_define_RetryPolicy();
  php_scylladb_define_RetryPolicyDefault(retry_policy_interface);
  php_driver_define_RetryPolicyDowngradingConsistency(retry_policy_interface);
  php_driver_define_RetryPolicyFallthrough(retry_policy_interface);
  php_driver_define_RetryPolicyLogging(retry_policy_interface);

  php_driver_define_TimestampGenerator();
  php_driver_define_TimestampGeneratorMonotonic();
  php_driver_define_TimestampGeneratorServerSide();

  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(php_driver) { return SUCCESS; }

PHP_RINIT_FUNCTION(php_driver) {
#define XX_SCALAR(name, value) ZVAL_UNDEF(&PHP_DRIVER_G(type_## name));
  PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR

  return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(php_driver) {
#define XX_SCALAR(name, value) PHP5TO7_ZVAL_MAYBE_DESTROY(PHP_DRIVER_G(type_## name));
  PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR)
#undef XX_SCALAR

  return SUCCESS;
}

PHP_MINFO_FUNCTION(php_driver) {
  char buf[256];
  php_info_print_table_start();

  php_info_print_table_row(2, PHP_DRIVER_NAMESPACE " support", "enabled");

  snprintf(buf, sizeof(buf), "%d.%d.%d", CASS_VERSION_MAJOR, CASS_VERSION_MINOR,
           CASS_VERSION_PATCH);
  php_info_print_table_row(2, "C/C++ driver version", buf);

  php_info_print_table_row(2, "PHP driver extension",
                           "customized for persistent prepared statements");

  snprintf(buf, sizeof(buf), "%d", PHP_DRIVER_G(persistent_clusters));
  php_info_print_table_row(2, "Persistent Clusters", buf);

  snprintf(buf, sizeof(buf), "%d", PHP_DRIVER_G(persistent_sessions));
  php_info_print_table_row(2, "Persistent Sessions", buf);

  snprintf(buf, sizeof(buf), "%d", PHP_DRIVER_G(persistent_prepared_statements));
  php_info_print_table_row(2, "Persistent Prepared Statements", buf);

  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
END_EXTERN_C()