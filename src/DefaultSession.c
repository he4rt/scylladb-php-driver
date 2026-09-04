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

#include "DateTime/Date.h"
#include "php_scylladb.h"
#include "php_scylladb_cache_key.h"
#include "php_scylladb_globals.h"
#include "php_scylladb_types.h"
#include "src/ExecutionOptions.h"
#include "Type/Conversions.h"
#include "FutureUtil.h"
#include "Numbers/NumberParser.h"
#include "Database/ResultDecoder.h"

#include "DefaultSession_arginfo.h"

extern zend_object_handlers php_scylladb_default_session_handlers;

#define CHECK_RESULT(rc)               \
  do {                                 \
    ASSERT_SUCCESS_VALUE(rc, FAILURE); \
    return SUCCESS;                    \
  } while (0)

/* Cleanup helpers for PHP_SCYLLADB_CLEANUP — destructor signature is `T **`
 * because the cleanup attribute passes a pointer to the local variable. */
static inline void php_scylladb_free_bytes(cass_byte_t **p) {
  if (*p) free(*p);
}
static inline void php_scylladb_zs_release(zend_string **p) {
  if (*p) zend_string_release(*p);
}
static inline void php_scylladb_cass_collection_free(CassCollection **p) {
  if (*p) cass_collection_free(*p);
}
static inline void php_scylladb_cass_tuple_free(CassTuple **p) {
  if (*p) cass_tuple_free(*p);
}
static inline void php_scylladb_cass_user_type_free(CassUserType **p) {
  if (*p) cass_user_type_free(*p);
}

static int bind_argument_by_index(CassStatement* statement, size_t index, zval* value) {
  /* Primitive dispatch: one Z_TYPE_P read, compiler builds a jump table.
   * CHECK_RESULT always returns, so each case is terminal. IS_OBJECT and
   * any unsupported type fall through to the instanceof chain / error. */
  switch (Z_TYPE_P(value)) {
    case IS_NULL:   CHECK_RESULT(cass_statement_bind_null(statement, index));
    case IS_STRING: CHECK_RESULT(cass_statement_bind_string(statement, index, Z_STRVAL_P(value)));
    case IS_DOUBLE: CHECK_RESULT(cass_statement_bind_double(statement, index, Z_DVAL_P(value)));
    case IS_LONG:   CHECK_RESULT(cass_statement_bind_int32(statement, index, Z_LVAL_P(value)));
    case IS_TRUE:   CHECK_RESULT(cass_statement_bind_bool(statement, index, cass_true));
    case IS_FALSE:  CHECK_RESULT(cass_statement_bind_bool(statement, index, cass_false));
    case IS_OBJECT: break;       /* dispatched on class entry below */
    default:        break;       /* fall through to unsupported throw */
  }

  if (Z_TYPE_P(value) == IS_OBJECT) {
    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_float_ce)) {
      auto float_number = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(cass_statement_bind_float(statement, index, float_number->data.floating.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_bigint_ce)) {
      auto bigint = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(cass_statement_bind_int64(statement, index, bigint->data.bigint.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_smallint_ce)) {
      auto smallint = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(cass_statement_bind_int16(statement, index, smallint->data.smallint.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_tinyint_ce)) {
      auto tinyint = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(cass_statement_bind_int8(statement, index, tinyint->data.tinyint.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_timestamp_ce)) {
      auto timestamp = Z_SCYLLADB_TIMESTAMP_P(value);
      CHECK_RESULT(cass_statement_bind_int64(statement, index, timestamp->timestamp));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_date_ce)) {
      auto date = Z_SCYLLADB_DATE_P(value);
      CHECK_RESULT(cass_statement_bind_uint32(statement, index, date->date));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_time_ce)) {
      auto time = Z_SCYLLADB_TIME_P(value);
      CHECK_RESULT(cass_statement_bind_int64(statement, index, time->time));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_blob_ce)) {
      auto blob = PHP_SCYLLADB_GET_BLOB(value);
      CHECK_RESULT(cass_statement_bind_bytes(statement, index, blob->data, blob->size));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_varint_ce)) {
      auto varint = PHP_SCYLLADB_GET_NUMERIC(value);
      size_t size;
      PHP_SCYLLADB_CLEANUP(php_scylladb_free_bytes)
      cass_byte_t* data = export_twos_complement(varint->data.varint.value, &size);
      CHECK_RESULT(cass_statement_bind_bytes(statement, index, data, size));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_decimal_ce)) {
      auto decimal = PHP_SCYLLADB_GET_NUMERIC(value);
      size_t size;
      PHP_SCYLLADB_CLEANUP(php_scylladb_free_bytes)
      cass_byte_t* data = export_twos_complement(decimal->data.decimal.value, &size);
      CHECK_RESULT(cass_statement_bind_decimal(statement, index, data, size, decimal->data.decimal.scale));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_uuid_interface_ce)) {
      auto uuid = PHP_SCYLLADB_GET_UUID(value);
      CHECK_RESULT(cass_statement_bind_uuid(statement, index, uuid->uuid));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_inet_ce)) {
      auto inet = PHP_SCYLLADB_GET_INET(value);
      CHECK_RESULT(cass_statement_bind_inet(statement, index, inet->inet));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_duration_ce)) {
      auto duration = PHP_SCYLLADB_GET_DURATION(value);
      CHECK_RESULT(cass_statement_bind_duration(statement, index, duration->months, duration->days,
                                                duration->nanos));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_set_ce)) {
      auto set = PHP_SCYLLADB_GET_SET(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_collection_free) CassCollection* collection = nullptr;
      if (!php_scylladb_collection_from_set(set, &collection)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_collection(statement, index, collection));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_map_ce)) {
      auto map = PHP_SCYLLADB_GET_MAP(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_collection_free) CassCollection* collection = nullptr;
      if (!php_scylladb_collection_from_map(map, &collection)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_collection(statement, index, collection));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_collection_ce)) {
      auto coll = PHP_SCYLLADB_GET_COLLECTION(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_collection_free) CassCollection* collection = nullptr;
      if (!php_scylladb_collection_from_collection(coll, &collection)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_collection(statement, index, collection));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_tuple_ce)) {
      auto tuple = PHP_SCYLLADB_GET_TUPLE(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_tuple_free) CassTuple* tup = nullptr;
      if (!php_scylladb_tuple_from_tuple(tuple, &tup)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_tuple(statement, index, tup));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_user_type_value_ce)) {
      auto user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_user_type_free) CassUserType* ut = nullptr;
      if (!php_scylladb_user_type_from_user_type_value(user_type_value, &ut)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_user_type(statement, index, ut));
    }
  }

  zend_throw_exception_ex(
      php_scylladb_invalid_argument_exception_ce, 0,
      "Unsupported argument type at index %zu", index);
  return FAILURE;
}

static int bind_argument_by_name(CassStatement* statement, const char* name, zval* value) {
  switch (Z_TYPE_P(value)) {
    case IS_NULL:   CHECK_RESULT(cass_statement_bind_null_by_name(statement, name));
    case IS_STRING: CHECK_RESULT(cass_statement_bind_string_by_name(statement, name, Z_STRVAL_P(value)));
    case IS_DOUBLE: CHECK_RESULT(cass_statement_bind_double_by_name(statement, name, Z_DVAL_P(value)));
    case IS_LONG:   CHECK_RESULT(cass_statement_bind_int32_by_name(statement, name, Z_LVAL_P(value)));
    case IS_TRUE:   CHECK_RESULT(cass_statement_bind_bool_by_name(statement, name, cass_true));
    case IS_FALSE:  CHECK_RESULT(cass_statement_bind_bool_by_name(statement, name, cass_false));
    case IS_OBJECT: break;
    default:        break;
  }

  if (Z_TYPE_P(value) == IS_OBJECT) {
    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_float_ce)) {
      auto float_number = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(
          cass_statement_bind_float_by_name(statement, name, float_number->data.floating.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_bigint_ce)) {
      auto bigint = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(cass_statement_bind_int64_by_name(statement, name, bigint->data.bigint.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_smallint_ce)) {
      auto smallint = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(
          cass_statement_bind_int16_by_name(statement, name, smallint->data.smallint.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_tinyint_ce)) {
      auto tinyint = PHP_SCYLLADB_GET_NUMERIC(value);
      CHECK_RESULT(cass_statement_bind_int8_by_name(statement, name, tinyint->data.tinyint.value));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_timestamp_ce)) {
      auto timestamp = Z_SCYLLADB_TIMESTAMP_P(value);
      CHECK_RESULT(cass_statement_bind_int64_by_name(statement, name, timestamp->timestamp));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_date_ce)) {
      auto date = Z_SCYLLADB_DATE_P(value);
      CHECK_RESULT(cass_statement_bind_uint32_by_name(statement, name, date->date));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_time_ce)) {
      auto time = Z_SCYLLADB_TIME_P(value);
      CHECK_RESULT(cass_statement_bind_int64_by_name(statement, name, time->time));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_blob_ce)) {
      auto blob = PHP_SCYLLADB_GET_BLOB(value);
      CHECK_RESULT(cass_statement_bind_bytes_by_name(statement, name, blob->data, blob->size));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_varint_ce)) {
      auto varint = PHP_SCYLLADB_GET_NUMERIC(value);
      size_t size;
      PHP_SCYLLADB_CLEANUP(php_scylladb_free_bytes)
      cass_byte_t* data = export_twos_complement(varint->data.varint.value, &size);
      CHECK_RESULT(cass_statement_bind_bytes_by_name(statement, name, data, size));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_decimal_ce)) {
      auto decimal = PHP_SCYLLADB_GET_NUMERIC(value);
      size_t size;
      PHP_SCYLLADB_CLEANUP(php_scylladb_free_bytes)
      cass_byte_t* data = export_twos_complement(decimal->data.decimal.value, &size);
      CHECK_RESULT(cass_statement_bind_decimal_by_name(statement, name, data, size,
                                                       decimal->data.decimal.scale));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_uuid_interface_ce)) {
      auto uuid = PHP_SCYLLADB_GET_UUID(value);
      CHECK_RESULT(cass_statement_bind_uuid_by_name(statement, name, uuid->uuid));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_inet_ce)) {
      auto inet = PHP_SCYLLADB_GET_INET(value);
      CHECK_RESULT(cass_statement_bind_inet_by_name(statement, name, inet->inet));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_duration_ce)) {
      auto duration = PHP_SCYLLADB_GET_DURATION(value);
      CHECK_RESULT(cass_statement_bind_duration_by_name(statement, name, duration->months,
                                                        duration->days, duration->nanos));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_set_ce)) {
      auto set = PHP_SCYLLADB_GET_SET(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_collection_free) CassCollection* collection = nullptr;
      if (!php_scylladb_collection_from_set(set, &collection)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_collection_by_name(statement, name, collection));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_map_ce)) {
      auto map = PHP_SCYLLADB_GET_MAP(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_collection_free) CassCollection* collection = nullptr;
      if (!php_scylladb_collection_from_map(map, &collection)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_collection_by_name(statement, name, collection));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_collection_ce)) {
      auto coll = PHP_SCYLLADB_GET_COLLECTION(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_collection_free) CassCollection* collection = nullptr;
      if (!php_scylladb_collection_from_collection(coll, &collection)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_collection_by_name(statement, name, collection));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_tuple_ce)) {
      auto tuple = PHP_SCYLLADB_GET_TUPLE(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_tuple_free) CassTuple* tup = nullptr;
      if (!php_scylladb_tuple_from_tuple(tuple, &tup)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_tuple_by_name(statement, name, tup));
    }

    if (instanceof_function(Z_OBJCE_P(value), php_scylladb_user_type_value_ce)) {
      auto user_type_value = PHP_SCYLLADB_GET_USER_TYPE_VALUE(value);
      PHP_SCYLLADB_CLEANUP(php_scylladb_cass_user_type_free) CassUserType* ut = nullptr;
      if (!php_scylladb_user_type_from_user_type_value(user_type_value, &ut)) return FAILURE;
      CHECK_RESULT(cass_statement_bind_user_type_by_name(statement, name, ut));
    }
  }

  zend_throw_exception_ex(
      php_scylladb_invalid_argument_exception_ce, 0,
      "Unsupported argument type for '%s'", name);
  return FAILURE;
}

static int bind_arguments(CassStatement* statement, HashTable* arguments) {
  int rc = SUCCESS;

  zval* current;
  zend_ulong num_key;
  zend_string* key;
  ZEND_HASH_FOREACH_KEY_VAL(arguments, num_key, key, current) {
    if (key) {
      rc = bind_argument_by_name(statement, key->val, current);
    } else {
      rc = bind_argument_by_index(statement, num_key, current);
    }
    if (rc == FAILURE) break;
  }
  ZEND_HASH_FOREACH_END();

  return rc;
}

static CassStatement* create_statement(php_scylladb_statement* statement, HashTable* arguments) {
  CassStatement* stmt;
  uint32_t count;

  switch (statement->type) {
    case PHP_SCYLLADB_SIMPLE_STATEMENT:
      count = 0;

      if (arguments) count = zend_hash_num_elements(arguments);

      stmt = cass_statement_new(statement->data.simple.cql, count);
      break;
    case PHP_SCYLLADB_PREPARED_STATEMENT:
      stmt = cass_prepared_bind(statement->data.prepared.prepared);
      break;
    default:
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0, "Unsupported statement type.");
      return nullptr;
  }

  if (stmt == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassStatement");
    return nullptr;
  }

  if (arguments && bind_arguments(stmt, arguments) == FAILURE) {
    cass_statement_free(stmt);
    return nullptr;
  }

  return stmt;
}

static CassBatch* create_batch(php_scylladb_statement* batch, CassConsistency consistency,
                               CassRetryPolicy* retry_policy, cass_int64_t timestamp,
                               php_scylladb_tristate idempotent) {
  CassBatch* cass_batch = cass_batch_new(batch->data.batch.type);
  if (cass_batch == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a CassBatch");
    return nullptr;
  }
  CassError rc = CASS_OK;

  zval* current;
  ZEND_HASH_FOREACH_VAL(&batch->data.batch.statements, current) {
    php_scylladb_statement* statement;
    php_scylladb_statement simple_statement;
    HashTable* arguments;
    CassStatement* stmt;

    php_scylladb_batch_statement_entry* batch_statement_entry =
        (php_scylladb_batch_statement_entry*)Z_PTR_P(current);

    if (Z_TYPE(batch_statement_entry->statement) == IS_STRING) {
      simple_statement.type = PHP_SCYLLADB_SIMPLE_STATEMENT;
      simple_statement.data.simple.cql = Z_STRVAL(batch_statement_entry->statement);
      simple_statement.idempotent = PHP_SCYLLADB_TRISTATE_UNSET;
      statement = &simple_statement;
    } else {
      statement = PHP_SCYLLADB_GET_STATEMENT(&batch_statement_entry->statement);
    }

    arguments = !Z_ISUNDEF(batch_statement_entry->arguments)
                    ? Z_ARRVAL_P(&batch_statement_entry->arguments)
                    : nullptr;

    stmt = create_statement(statement, arguments);
    if (!stmt) {
      cass_batch_free(cass_batch);
      return nullptr;
    }
    cass_batch_add_statement(cass_batch, stmt);
    cass_statement_free(stmt);
  }
  ZEND_HASH_FOREACH_END();

  rc = cass_batch_set_consistency(cass_batch, consistency);
  ASSERT_SUCCESS_BLOCK(rc, cass_batch_free(cass_batch); return nullptr;);

  rc = cass_batch_set_retry_policy(cass_batch, retry_policy);
  ASSERT_SUCCESS_BLOCK(rc, cass_batch_free(cass_batch); return nullptr;);

  // INT64_MIN is our sentinel for "no explicit timestamp set" (see
  // ExecutionOptions). The legacy cpp-driver tolerated forwarding it on the
  // wire; cpp-rs-driver rejects with "Out of bound timestamp" since the
  // protocol range excludes INT64_MIN. Only set when caller provided a value.
  if (timestamp != INT64_MIN) {
    rc = cass_batch_set_timestamp(cass_batch, timestamp);
    ASSERT_SUCCESS_BLOCK(rc, cass_batch_free(cass_batch); return nullptr;);
  }

  /* Idempotence applies to the batch as a whole. The driver reads it off the
   * batch, so a flag on a statement added to the batch has no effect. */
  if (idempotent != PHP_SCYLLADB_TRISTATE_UNSET) {
    rc = cass_batch_set_is_idempotent(
        cass_batch, idempotent == PHP_SCYLLADB_TRISTATE_TRUE ? cass_true : cass_false);
    ASSERT_SUCCESS_BLOCK(rc, cass_batch_free(cass_batch); return nullptr;);
  }

  return cass_batch;
}

static CassStatement* create_single(php_scylladb_statement* statement, HashTable* arguments,
                                    CassConsistency consistency, long serial_consistency,
                                    int page_size, const char* paging_state_token,
                                    size_t paging_state_token_size, CassRetryPolicy* retry_policy,
                                    cass_int64_t timestamp, php_scylladb_tristate idempotent) {
  CassError rc = CASS_OK;
  CassStatement* stmt = create_statement(statement, arguments);
  if (!stmt) return nullptr;

  rc = cass_statement_set_consistency(stmt, consistency);

  if (rc == CASS_OK && serial_consistency >= 0)
    rc = cass_statement_set_serial_consistency(stmt,
                                               (CassConsistency)serial_consistency);

  if (rc == CASS_OK && page_size >= 0) rc = cass_statement_set_paging_size(stmt, page_size);

  if (rc == CASS_OK && paging_state_token) {
    rc = cass_statement_set_paging_state_token(stmt, paging_state_token, paging_state_token_size);
  }

  if (rc == CASS_OK && retry_policy) rc = cass_statement_set_retry_policy(stmt, retry_policy);

  // INT64_MIN means "no explicit timestamp" — skip to avoid cpp-rs-driver
  // rejecting it as out-of-range. See create_batch() for the same reason.
  if (rc == CASS_OK && timestamp != INT64_MIN)
    rc = cass_statement_set_timestamp(stmt, timestamp);

  if (rc == CASS_OK && idempotent != PHP_SCYLLADB_TRISTATE_UNSET)
    rc = cass_statement_set_is_idempotent(
        stmt, idempotent == PHP_SCYLLADB_TRISTATE_TRUE ? cass_true : cass_false);

  if (rc != CASS_OK) {
    cass_statement_free(stmt);
    zend_throw_exception_ex(exception_class(rc), rc, "%s", cass_error_desc(rc));
    return nullptr;
  }

  return stmt;
}

ZEND_METHOD(Cassandra_DefaultSession, execute) {
  zval* statement = nullptr;
  zval* options = nullptr;
  php_scylladb_session* self = nullptr;
  php_scylladb_statement* stmt = nullptr;
  php_scylladb_statement simple_statement;
  HashTable* arguments = nullptr;
  CassConsistency consistency = PHP_SCYLLADB_DEFAULT_CONSISTENCY;
  int page_size = -1;
  char* paging_state_token = nullptr;
  size_t paging_state_token_size = 0;
  zval* timeout = nullptr;
  long serial_consistency = -1;
  CassRetryPolicy* retry_policy = nullptr;
  cass_int64_t timestamp = INT64_MIN;
  php_scylladb_tristate idempotent = PHP_SCYLLADB_TRISTATE_UNSET;
  php_scylladb_execution_options* opts = nullptr;
  php_scylladb_execution_options local_opts;
  CassFuture* future = nullptr;
  CassStatement* single = nullptr;
  CassBatch* batch = nullptr;
  PHP_SCYLLADB_CLEANUP(php_scylladb_zs_release) zend_string* execution_profile = nullptr;

  zval* execution_profile_zv = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 3)
    Z_PARAM_ZVAL(statement)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(options)
    Z_PARAM_ZVAL_OR_NULL(execution_profile_zv)
  ZEND_PARSE_PARAMETERS_END();

  /* Third argument wins over anything an ExecutionOptions object carries. */
  if (execution_profile_zv != nullptr && Z_TYPE_P(execution_profile_zv) != IS_NULL) {
    execution_profile = php_scylladb_name_from_string_or_enum(execution_profile_zv);
    if (execution_profile == nullptr || ZSTR_LEN(execution_profile) == 0) {
      if (execution_profile) zend_string_release(execution_profile);
      throw_invalid_argument(execution_profile_zv, "executionProfile",
                             "a non-empty string or an enum case naming a profile registered on the cluster");
      return;
    }
  }

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  if (Z_TYPE_P(statement) == IS_STRING) {
    simple_statement.type = PHP_SCYLLADB_SIMPLE_STATEMENT;
    simple_statement.data.simple.cql = Z_STRVAL_P(statement);
    simple_statement.idempotent = PHP_SCYLLADB_TRISTATE_UNSET;
    stmt = &simple_statement;
  } else if (Z_TYPE_P(statement) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(statement), php_scylladb_statement_ce)) {
    stmt = PHP_SCYLLADB_GET_STATEMENT(statement);
  } else {
    INVALID_ARGUMENT(statement, "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Statement");
  }

  idempotent = stmt->idempotent;

  consistency = (CassConsistency)self->default_consistency;
  page_size = self->default_page_size;
  timeout = &self->default_timeout;

  if (options) {
    if (Z_TYPE_P(options) != IS_ARRAY &&
        (Z_TYPE_P(options) != IS_OBJECT ||
         !instanceof_function(Z_OBJCE_P(options), php_scylladb_execution_options_ce))) {
      INVALID_ARGUMENT(
          options, "an instance of " PHP_SCYLLADB_NAMESPACE "\\ExecutionOptions or an array or null");
    }

    if (Z_TYPE_P(options) == IS_OBJECT) {
      opts = PHP_SCYLLADB_GET_EXECUTION_OPTIONS(options);
    } else {
      if (php_scylladb_execution_options_build_local_from_array(&local_opts, options) == FAILURE) {
        return;
      }
      opts = &local_opts;
    }

    if (!Z_ISUNDEF(opts->arguments)) arguments = Z_ARRVAL(opts->arguments);

    if (opts->consistency >= 0) consistency = (CassConsistency)opts->consistency;

    if (opts->page_size >= 0) page_size = opts->page_size;

    if (opts->paging_state_token) {
      paging_state_token = opts->paging_state_token;
      paging_state_token_size = opts->paging_state_token_size;
    }

    if (!Z_ISUNDEF(opts->timeout)) timeout = &opts->timeout;

    if (opts->serial_consistency >= 0) serial_consistency = opts->serial_consistency;

    if (!Z_ISUNDEF(opts->retry_policy))
      retry_policy = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, Z_OBJ_P(&opts->retry_policy))->policy;

    timestamp = opts->timestamp;

    /* A per-call option overrides whatever the statement carries. */
    if (opts->idempotent != PHP_SCYLLADB_TRISTATE_UNSET) idempotent = opts->idempotent;
  }

  switch (stmt->type) {
    case PHP_SCYLLADB_SIMPLE_STATEMENT:
    case PHP_SCYLLADB_PREPARED_STATEMENT:
      single = create_single(stmt, arguments, consistency, serial_consistency, page_size,
                             paging_state_token, paging_state_token_size, retry_policy, timestamp,
                             idempotent);

      if (!single) return;

      if (execution_profile) {
        cass_statement_set_execution_profile_n(single, ZSTR_VAL(execution_profile),
                                              ZSTR_LEN(execution_profile));
      }

      future = cass_session_execute(self->session, single);
      break;
    case PHP_SCYLLADB_BATCH_STATEMENT:
      batch = create_batch(stmt, consistency, retry_policy, timestamp, idempotent);

      if (!batch) return;

      if (execution_profile) {
        cass_batch_set_execution_profile_n(batch, ZSTR_VAL(execution_profile),
                                           ZSTR_LEN(execution_profile));
      }

      future = cass_session_execute_batch(self->session, batch);
      break;
    default:
      INVALID_ARGUMENT(statement,
                       "an instance of " PHP_SCYLLADB_NAMESPACE
                       "\\SimpleStatement, " PHP_SCYLLADB_NAMESPACE
                       "\\PreparedStatement or " PHP_SCYLLADB_NAMESPACE "\\BatchStatement");
      return;
  }

  do {
    const CassResult* result = nullptr;
    php_scylladb_rows* rows = nullptr;

    if (php_scylladb_future_wait_timed(future, timeout) == FAILURE ||
        php_scylladb_future_is_error(future) == FAILURE) {
      cass_future_free(future);
      break;
    }

    result = cass_future_get_result(future);
    cass_future_free(future);

    if (!result) {
      zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                              "Future doesn't contain a result.");
      break;
    }

    object_init_ex(return_value, php_scylladb_rows_ce);
    rows = PHP_SCYLLADB_GET_ROWS(return_value);

    if (php_scylladb_get_result(result, &rows->rows) == FAILURE) {
      cass_result_free(result);
      break;
    }

    if (single && cass_result_has_more_pages(result)) {
      /* Transfer statement and result ownership to Rows for paging. */
      rows->statement = zend_register_resource(single, php_le_cass_statement());
      rows->result    = result;
      ZVAL_COPY(&rows->session, getThis());
      single = nullptr;   /* ownership transferred via resource */
      return;
    }

    cass_result_free(result);
  } while (0);

  if (batch) cass_batch_free(batch);

  if (single) cass_statement_free(single);
}

ZEND_METHOD(Cassandra_DefaultSession, executeAsync) {
  zval* statement = nullptr;
  zval* options = nullptr;
  php_scylladb_session* self = nullptr;
  php_scylladb_statement* stmt = nullptr;
  php_scylladb_statement simple_statement;
  HashTable* arguments = nullptr;
  CassConsistency consistency = PHP_SCYLLADB_DEFAULT_CONSISTENCY;
  int page_size = -1;
  char* paging_state_token = nullptr;
  size_t paging_state_token_size = 0;
  long serial_consistency = -1;
  CassRetryPolicy* retry_policy = nullptr;
  cass_int64_t timestamp = INT64_MIN;
  php_scylladb_tristate idempotent = PHP_SCYLLADB_TRISTATE_UNSET;
  php_scylladb_execution_options* opts = nullptr;
  php_scylladb_execution_options local_opts;
  php_scylladb_future_rows* future_rows = nullptr;
  CassStatement* single = nullptr;
  CassBatch* batch = nullptr;
  PHP_SCYLLADB_CLEANUP(php_scylladb_zs_release) zend_string* execution_profile = nullptr;

  zval* execution_profile_zv = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 3)
    Z_PARAM_ZVAL(statement)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(options)
    Z_PARAM_ZVAL_OR_NULL(execution_profile_zv)
  ZEND_PARSE_PARAMETERS_END();

  /* Third argument wins over anything an ExecutionOptions object carries. */
  if (execution_profile_zv != nullptr && Z_TYPE_P(execution_profile_zv) != IS_NULL) {
    execution_profile = php_scylladb_name_from_string_or_enum(execution_profile_zv);
    if (execution_profile == nullptr || ZSTR_LEN(execution_profile) == 0) {
      if (execution_profile) zend_string_release(execution_profile);
      throw_invalid_argument(execution_profile_zv, "executionProfile",
                             "a non-empty string or an enum case naming a profile registered on the cluster");
      return;
    }
  }

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  if (Z_TYPE_P(statement) == IS_STRING) {
    simple_statement.type = PHP_SCYLLADB_SIMPLE_STATEMENT;
    simple_statement.data.simple.cql = Z_STRVAL_P(statement);
    simple_statement.idempotent = PHP_SCYLLADB_TRISTATE_UNSET;
    stmt = &simple_statement;
  } else if (Z_TYPE_P(statement) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(statement), php_scylladb_statement_ce)) {
    stmt = PHP_SCYLLADB_GET_STATEMENT(statement);
  } else {
    INVALID_ARGUMENT(statement, "a string or an instance of " PHP_SCYLLADB_NAMESPACE "\\Statement");
  }

  idempotent = stmt->idempotent;

  consistency = (CassConsistency)self->default_consistency;
  page_size = self->default_page_size;

  if (options) {
    if (Z_TYPE_P(options) != IS_ARRAY &&
        (Z_TYPE_P(options) != IS_OBJECT ||
         !instanceof_function(Z_OBJCE_P(options), php_scylladb_execution_options_ce))) {
      INVALID_ARGUMENT(
          options, "an instance of " PHP_SCYLLADB_NAMESPACE "\\ExecutionOptions or an array or null");
    }

    if (Z_TYPE_P(options) == IS_OBJECT) {
      opts = PHP_SCYLLADB_GET_EXECUTION_OPTIONS(options);
    } else {
      if (php_scylladb_execution_options_build_local_from_array(&local_opts, options) == FAILURE) {
        return;
      }
      opts = &local_opts;
    }

    if (!Z_ISUNDEF(opts->arguments)) arguments = Z_ARRVAL(opts->arguments);

    if (opts->consistency >= 0) consistency = (CassConsistency)opts->consistency;

    if (opts->page_size >= 0) page_size = opts->page_size;

    if (opts->paging_state_token) {
      paging_state_token = opts->paging_state_token;
      paging_state_token_size = opts->paging_state_token_size;
    }

    if (opts->serial_consistency >= 0) serial_consistency = opts->serial_consistency;

    if (!Z_ISUNDEF(opts->retry_policy))
      retry_policy = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, Z_OBJ_P(&opts->retry_policy))->policy;

    timestamp = opts->timestamp;

    /* A per-call option overrides whatever the statement carries. */
    if (opts->idempotent != PHP_SCYLLADB_TRISTATE_UNSET) idempotent = opts->idempotent;
  }

  object_init_ex(return_value, php_scylladb_future_rows_ce);
  future_rows = PHP_SCYLLADB_GET_FUTURE_ROWS(return_value);

  switch (stmt->type) {
    case PHP_SCYLLADB_SIMPLE_STATEMENT:
    case PHP_SCYLLADB_PREPARED_STATEMENT:
      single = create_single(stmt, arguments, consistency, serial_consistency, page_size,
                             paging_state_token, paging_state_token_size, retry_policy, timestamp,
                             idempotent);

      if (!single) return;

      if (execution_profile) {
        cass_statement_set_execution_profile_n(single, ZSTR_VAL(execution_profile),
                                              ZSTR_LEN(execution_profile));
      }

      future_rows->statement = zend_register_resource(single, php_le_cass_statement());
      future_rows->future = cass_session_execute(self->session, single);
      ZVAL_COPY(&future_rows->session, getThis());
      break;
    case PHP_SCYLLADB_BATCH_STATEMENT:
      batch = create_batch(stmt, consistency, retry_policy, timestamp, idempotent);

      if (!batch) return;

      if (execution_profile) {
        cass_batch_set_execution_profile_n(batch, ZSTR_VAL(execution_profile),
                                           ZSTR_LEN(execution_profile));
      }

      future_rows->future = cass_session_execute_batch(self->session, batch);
      cass_batch_free(batch);
      break;
    default:
      INVALID_ARGUMENT(statement,
                       "an instance of " PHP_SCYLLADB_NAMESPACE
                       "\\SimpleStatement, " PHP_SCYLLADB_NAMESPACE
                       "\\PreparedStatement or " PHP_SCYLLADB_NAMESPACE "\\BatchStatement");
      return;
  }
}

ZEND_METHOD(Cassandra_DefaultSession, prepare) {
  zval* cql = nullptr;
  zval* options = nullptr;
  zend_ulong cache_key = 0;
  php_scylladb_session* self = nullptr;
  php_scylladb_execution_options* opts = nullptr;
  php_scylladb_execution_options local_opts;
  CassFuture* future = nullptr;
  zval* timeout = nullptr;
  php_scylladb_statement* prepared_statement = nullptr;
  php_scylladb_pprepared_statement* pprepared_statement = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_ZVAL(cql)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(options)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  if (options) {
    if (Z_TYPE_P(options) != IS_ARRAY &&
        (Z_TYPE_P(options) != IS_OBJECT ||
         !instanceof_function(Z_OBJCE_P(options), php_scylladb_execution_options_ce))) {
      INVALID_ARGUMENT(
          options, "an instance of " PHP_SCYLLADB_NAMESPACE "\\ExecutionOptions or an array or null");
    }

    if (Z_TYPE_P(options) == IS_OBJECT) {
      opts = PHP_SCYLLADB_GET_EXECUTION_OPTIONS(options);
    } else {
      if (php_scylladb_execution_options_build_local_from_array(&local_opts, options) == FAILURE) {
        return;
      }
      opts = &local_opts;
    }
    timeout = &opts->timeout;
  }

  if (self->persist) {
    /* Mix session cache_key + CQL bytes + keyspace into a uint64_t.
       Zero allocation per prepare() call, even on cache hit. */
    cache_key = php_scylladb_cache_key_mix_ulong(php_scylladb_cache_key_init(), self->cache_key);
    cache_key = php_scylladb_cache_key_mix_bytes(cache_key, Z_STRVAL_P(cql), Z_STRLEN_P(cql));
    cache_key = php_scylladb_cache_key_mix_cstr(cache_key, ":prepared_statement:");
    cache_key = php_scylladb_cache_key_mix_cstr(cache_key, SAFE_STR(self->keyspace));

    zval* le = zend_hash_index_find(&EG(persistent_list), cache_key);
    if (le != nullptr && Z_RES_P(le)->type == php_le_php_scylladb_prepared_statement()) {
      pprepared_statement = (php_scylladb_pprepared_statement*)Z_RES_P(le)->ptr;

      /* The cached future may belong to a previous prepare that has since
       * failed (e.g. a server-side schema drop). Validate the error code
       * before trusting cass_future_get_prepared, which returns nullptr on
       * error futures and would otherwise corrupt the prepared statement. */
      if (cass_future_error_code(pprepared_statement->future) == CASS_OK) {
        const CassPrepared* cached = cass_future_get_prepared(pprepared_statement->future);
        if (cached != nullptr) {
          object_init_ex(return_value, php_scylladb_prepared_statement_ce);
          prepared_statement = PHP_SCYLLADB_GET_STATEMENT(return_value);
          prepared_statement->data.prepared.prepared = cached;
          return;
        }
      }

      /* Cached future is bad - drop the entry and re-prepare below. */
      (void)zend_hash_index_del(&EG(persistent_list), cache_key);
    }
  }

  /* Cache miss. An unbounded prepared-statement cache is the failure mode
     cassandra.max_persistent_prepared_statements exists for: queries built by
     string concatenation produce a distinct cache_key each time. Past the cap
     the statement is still prepared, just not kept. */
  bool cache_prepared =
      self->persist && php_scylladb_persistent_can_cache(PHP_SCYLLADB_PERSISTENT_PREPARED_STATEMENTS);

  future =
      cass_session_prepare_n(self->session, Z_STRVAL_P(cql), Z_STRLEN_P(cql));

  if (future == nullptr) {
    zend_throw_exception_ex(php_scylladb_runtime_exception_ce, 0,
                            "Failed to allocate a prepare future");
    return;
  }

  if (php_scylladb_future_wait_timed(future, timeout) == FAILURE) {
    cass_future_free(future);
    return;
  }

  if (php_scylladb_future_is_error(future) == FAILURE) {
    cass_future_free(future);
    return;
  }

  object_init_ex(return_value, php_scylladb_prepared_statement_ce);
  prepared_statement = PHP_SCYLLADB_GET_STATEMENT(return_value);
  prepared_statement->data.prepared.prepared = cass_future_get_prepared(future);

  if (cache_prepared) {
    zval resource;
    pprepared_statement =
        (php_scylladb_pprepared_statement*)pecalloc(1, sizeof(php_scylladb_pprepared_statement), 1);
    /* No back-ref to the session: persistent_list cleanup is LIFO,
       so this entry is destroyed before the psession it depends on,
       and CassFuture holds an internal CassSession ref for safety. */
    pprepared_statement->future = future;

    ZVAL_NEW_PERSISTENT_RES(&resource, 0, pprepared_statement,
                            php_le_php_scylladb_prepared_statement());
    (void)zend_hash_index_update(&EG(persistent_list), cache_key, &resource);
    PHP_SCYLLADB_G(persistent_prepared_statements)++;
    /* Ownership of `future` transferred to the persistent_list entry. */
  } else {
    cass_future_free(future);
  }
}

ZEND_METHOD(Cassandra_DefaultSession, prepareAsync) {
  zval* cql = nullptr;
  zval* options = nullptr;
  php_scylladb_session* self = nullptr;
  CassFuture* future = nullptr;
  php_scylladb_future_prepared_statement* future_prepared = nullptr;

  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_ZVAL(cql)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(options)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  future =
      cass_session_prepare_n(self->session, Z_STRVAL_P(cql), Z_STRLEN_P(cql));

  object_init_ex(return_value, php_scylladb_future_prepared_statement_ce);
  future_prepared = PHP_SCYLLADB_GET_FUTURE_PREPARED_STATEMENT(return_value);

  future_prepared->future = future;
}

ZEND_METHOD(Cassandra_DefaultSession, close) {
  zval* timeout = nullptr;
  CassFuture* future = nullptr;
  php_scylladb_session* self;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  if (self->persist) return;

  future = cass_session_close(self->session);

  if (php_scylladb_future_wait_timed(future, timeout) == SUCCESS) php_scylladb_future_is_error(future);

  cass_future_free(future);
}

ZEND_METHOD(Cassandra_DefaultSession, closeAsync) {
  php_scylladb_session* self;
  php_scylladb_future_close* future = nullptr;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  if (self->persist) {
    object_init_ex(return_value, php_scylladb_future_value_ce);
    return;
  }

  object_init_ex(return_value, php_scylladb_future_close_ce);
  future = PHP_SCYLLADB_GET_FUTURE_CLOSE(return_value);

  future->future = cass_session_close(self->session);
  ZVAL_COPY(&future->session, getThis());
}

ZEND_METHOD(Cassandra_DefaultSession, metrics) {
  CassMetrics metrics;
  zval requests;
  zval stats;
  zval errors;
  auto self = PHP_SCYLLADB_GET_SESSION(getThis());

  if (zend_parse_parameters_none() == FAILURE) return;

  cass_session_get_metrics(self->session, &metrics);

  array_init(&requests);
  add_assoc_long(&requests, "min", metrics.requests.min);
  add_assoc_long(&requests, "max", metrics.requests.max);
  add_assoc_long(&requests, "mean", metrics.requests.mean);
  add_assoc_long(&requests, "stddev", metrics.requests.stddev);
  add_assoc_long(&requests, "median", metrics.requests.median);
  add_assoc_long(&requests, "p75", metrics.requests.percentile_75th);
  add_assoc_long(&requests, "p95", metrics.requests.percentile_95th);
  add_assoc_long(&requests, "p98", metrics.requests.percentile_98th);
  add_assoc_long(&requests, "p99", metrics.requests.percentile_99th);
  add_assoc_long(&requests, "p999", metrics.requests.percentile_999th);
  add_assoc_double(&requests, "mean_rate", metrics.requests.mean_rate);
  add_assoc_double(&requests, "m1_rate", metrics.requests.one_minute_rate);
  add_assoc_double(&requests, "m5_rate", metrics.requests.five_minute_rate);
  add_assoc_double(&requests, "m15_rate", metrics.requests.fifteen_minute_rate);

  array_init(&stats);
  add_assoc_long(&stats, "total_connections", metrics.stats.total_connections);
  add_assoc_long(&stats, "available_connections", metrics.stats.available_connections);
  add_assoc_long(&stats, "exceeded_pending_requests_water_mark",
                 metrics.stats.exceeded_pending_requests_water_mark);
  add_assoc_long(&stats, "exceeded_write_bytes_water_mark",
                 metrics.stats.exceeded_write_bytes_water_mark);

  array_init(&errors);
  add_assoc_long(&errors, "connection_timeouts", metrics.errors.connection_timeouts);
  add_assoc_long(&errors, "pending_request_timeouts", metrics.errors.pending_request_timeouts);
  add_assoc_long(&errors, "request_timeouts", metrics.errors.request_timeouts);

  array_init(return_value);
  add_assoc_zval(return_value, "stats", &stats);
  add_assoc_zval(return_value, "requests", &requests);
  add_assoc_zval(return_value, "errors", &errors);
}

ZEND_METHOD(Cassandra_DefaultSession, schema) {
  php_scylladb_session* self;
  php_scylladb_schema* schema;

  if (zend_parse_parameters_none() == FAILURE) return;

  self = PHP_SCYLLADB_GET_SESSION(getThis());

  object_init_ex(return_value, php_scylladb_default_schema_ce);
  schema = PHP_SCYLLADB_GET_SCHEMA(return_value);

  schema->schema_meta = cass_session_get_schema_meta(self->session);
}

HashTable* php_scylladb_default_session_properties(zend_object* object) {
  HashTable* props = zend_std_get_properties(object);

  return props;
}

int php_scylladb_default_session_compare(zval* obj1, zval* obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

  return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_default_session_free(zend_object* object) {
  auto self = php_scylladb_session_object_fetch(object);

  /* Persistent: the psession entry in EG(persistent_list) owns the
     CassSession; we just borrowed the pointer. */
  if (!self->persist && self->session) {
    cass_session_free(self->session);
    self->session = nullptr;
  }
  zval_ptr_dtor(&self->default_timeout);

  if (self->keyspace) {
    efree(self->keyspace);
    self->keyspace = nullptr;
  }

  zend_object_std_dtor(&self->zendObject);
}

zend_object* php_scylladb_default_session_new(zend_class_entry* ce) {
  php_scylladb_session* self =
      PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_session, ce, &php_scylladb_default_session_handlers);

  self->session = nullptr;
  self->persist = cass_false;
  self->default_consistency = PHP_SCYLLADB_DEFAULT_CONSISTENCY;
  self->default_page_size = 5000;
  self->keyspace = nullptr;
  self->cache_key = 0;
  ZVAL_UNDEF(&self->default_timeout);
  return &self->zendObject;
}
