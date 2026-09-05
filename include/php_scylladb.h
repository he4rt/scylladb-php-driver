#pragma once

#include <cassandra.h>
/* Must follow cassandra.h: shims the schema-metadata declarations that
 * cpp-rs-driver master removed. No effect on the other two backends. */
#include <php_scylladb_rs_compat.h>
#include <gmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <version.h>

#include <inttypes.h>
#include <math.h>
#include <string.h>

#include <php.h>

#include <Zend/zend_attributes.h>
#include <Zend/zend_enum.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_types.h>

#if PHP_VERSION_ID < 80100
#error PHP 8.1.0 or later is required in order to build the driver
#endif

#if PHP_VERSION_ID < 80400
/* Shim for the PHP 8.4 helper used by stub-generated arginfo headers. */
static zend_always_inline zend_class_entry *zend_register_internal_class_with_flags(
    zend_class_entry *class_entry, zend_class_entry *parent_ce, uint32_t ce_flags) {
    zend_class_entry *ce = zend_register_internal_class_ex(class_entry, parent_ce);
    ce->ce_flags |= ce_flags;
    return ce;
}
#endif


#define PHP_SCYLLADB_NAMESPACE "Cassandra"

#define PHP_SCYLLADB_NAMESPACE_ZEND_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null)                               \
    ZEND_ARG_OBJ_INFO(pass_by_ref, name, Cassandra\\classname, allow_null)

#define PHP_SCYLLADB_CORE_METHOD(name) PHP_METHOD(Cassandra, name)

#define PHP_SCYLLADB_CORE_ME(name, arg_info, flags) PHP_ME(Cassandra, name, arg_info, flags)

/* Inline functions give us single-evaluation of the argument without the
 * GNU statement-expression extension that -Wpedantic complains about. */
[[gnu::const]]
static inline const char *php_scylladb_safe_str(const char *s) {
    return s ? s : "";
}

[[gnu::pure]]
static inline const char *php_scylladb_safe_zend_string(const zend_string *s) {
    return s != nullptr ? ZSTR_VAL(s) : "";
}

#define SAFE_STR(a)         php_scylladb_safe_str(a)
#define SAFE_ZEND_STRING(a) php_scylladb_safe_zend_string(a)

/* Resolves a name given either as a string or as a PHP enum case.
 *
 * A string-backed enum contributes its value, so `enum P: string { case A = 'a'; }`
 * yields "a". Anything else — a pure enum, or one backed by int, where the value
 * would be a meaningless name — contributes its case name, so `P::A` yields "A".
 *
 * Returns nullptr when the zval is neither a string nor an enum case. The caller
 * owns the returned string. */
[[nodiscard]] static inline zend_string *php_scylladb_name_from_string_or_enum(const zval *v)
{
    if (Z_TYPE_P(v) == IS_STRING) {
        return zend_string_copy(Z_STR_P(v));
    }

    if (Z_TYPE_P(v) == IS_OBJECT && (Z_OBJCE_P(v)->ce_flags & ZEND_ACC_ENUM)) {
        zend_object *obj = Z_OBJ_P(v);

        if (Z_OBJCE_P(v)->enum_backing_type == IS_STRING) {
            return zend_string_copy(Z_STR_P(zend_enum_fetch_case_value(obj)));
        }

        return zend_string_copy(Z_STR_P(zend_enum_fetch_case_name(obj)));
    }

    return nullptr;
}

/* Branch-prediction hints — gcc/clang builtins, both are C23-compatible. */
#define PHP_SCYLLADB_LIKELY(x)   __builtin_expect(!!(x), 1)
#define PHP_SCYLLADB_UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Scope-bound cleanup. The destructor must take `T **` (cleanup attribute passes
 * a pointer to the variable). Example:
 *
 *     static inline void php_scylladb_zs_release(zend_string **p) {
 *         if (*p) zend_string_release(*p);
 *     }
 *     PHP_SCYLLADB_CLEANUP(php_scylladb_zs_release) zend_string *s = zend_string_init(...);
 *
 * Fires on every exit path (return / break / goto / fall-through). */
#define PHP_SCYLLADB_CLEANUP(fn) __attribute__((cleanup(fn)))

#define PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(op1, op2)                                                                \
    do {                                                                                                               \
        ZEND_COMPARE_OBJECTS_FALLBACK(op1, op2)                                                                        \
    } while (0)

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define PHP_SCYLLADB_G(v) TSRMG(php_scylladb_globals_id, zend_php_scylladb_globals *, v)
#else
#define PHP_SCYLLADB_G(v) (php_scylladb_globals.v)
#endif

#define CPP_DRIVER_VERSION(major, minor, patch) (((major) << 16) + ((minor) << 8) + (patch))

#define CURRENT_CPP_DRIVER_VERSION CPP_DRIVER_VERSION(CASS_VERSION_MAJOR, CASS_VERSION_MINOR, CASS_VERSION_PATCH)

extern zend_module_entry php_scylladb_module_entry;
#define phpext_cassandra_ptr &php_scylladb_module_entry

PHP_MINIT_FUNCTION(php_scylladb);
PHP_MSHUTDOWN_FUNCTION(php_scylladb);
PHP_RINIT_FUNCTION(php_scylladb);
PHP_RSHUTDOWN_FUNCTION(php_scylladb);
PHP_MINFO_FUNCTION(php_scylladb);
PHP_INI_MH(OnUpdateLogLevel);
PHP_INI_MH(OnUpdateLog);
PHP_INI_MH(OnUpdateDefaultConsistency);
PHP_INI_MH(OnUpdatePort);
PHP_INI_MH(OnUpdateProtocolVersion);
PHP_INI_MH(OnUpdatePositiveLong);
PHP_INI_MH(OnUpdatePersistentMax);
PHP_INI_MH(OnUpdateNewRequestRatio);
PHP_INI_MH(OnUpdateTracingConsistency);

/* Which persistent_list population a cache decision is about. The enumerator
 * names double as the `cassandra.max_persistent_*` suffix in the warning. */
typedef enum : uint8_t {
    PHP_SCYLLADB_PERSISTENT_CLUSTERS,
    PHP_SCYLLADB_PERSISTENT_SESSIONS,
    PHP_SCYLLADB_PERSISTENT_PREPARED_STATEMENTS,
} php_scylladb_persistent_kind;

/* True when a new entry of `kind` may be added to EG(persistent_list).
 * Pure read — the counters are still incremented by the caller on insert.
 * On the first refusal in a request it emits one E_WARNING, so a saturated
 * cache is visible in the log instead of silently degrading throughput. */
bool php_scylladb_persistent_can_cache(php_scylladb_persistent_kind kind);

/* The error → exception_ce mapping is a closed lookup with no side effects
 * and no memory reads beyond its argument — `const` is exact. */
[[gnu::const]] zend_class_entry *exception_class(CassError rc);

[[gnu::nonnull(1, 2, 3)]]
void throw_invalid_argument(const zval *object, const char *object_name, const char *expected_type);

#define INVALID_ARGUMENT(object, expected)                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        throw_invalid_argument(object, #object, expected);                                                   \
        return;                                                                                                        \
    } while (0)

#define INVALID_ARGUMENT_VALUE(object, expected, failed_value)                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        throw_invalid_argument(object, #object, expected);                                                   \
        return failed_value;                                                                                           \
    } while (0)

#define PHP_SCYLLADB_THROW_NO_LEGACY_SCHEMA_META(what)                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        zend_throw_exception_ex(php_scylladb_domain_exception_ce, 0,                                                   \
                                what " needs a build with -DPHP_SCYLLADB_ENABLE_LEGACY_SCHEMA_META=ON. "                \
                                     "The C/C++ driver leaves the matching metadata API unimplemented.");               \
        return;                                                                                                        \
    } while (0)

#define ASSERT_SUCCESS_BLOCK(rc, block)                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        if (rc != CASS_OK)                                                                                             \
        {                                                                                                              \
            zend_throw_exception_ex(exception_class(rc), rc, "%s", cass_error_desc(rc));                     \
            block                                                                                                      \
        }                                                                                                              \
    } while (0)

#define ASSERT_SUCCESS(rc) ASSERT_SUCCESS_BLOCK(rc, return;)

#define ASSERT_SUCCESS_VALUE(rc, value) ASSERT_SUCCESS_BLOCK(rc, return value;)

/* Kept as #define: clang-tidy on CI runs without -std=c23 and chokes on
 * `constexpr` despite the build compiler supporting it. Revisit once the
 * lint job inherits the build's compilation database flags. */
/* LOCAL_QUORUM, not the C driver's LOCAL_ONE. LOCAL_ONE can serve a read from a
 * replica that has not yet seen the last write, which surprises people who never
 * chose a consistency level. LOCAL_QUORUM is the level the guide already tells
 * applications to pick. It is deliberately not plain QUORUM: QUORUM spans every
 * datacenter, so it adds cross-DC latency to every query and fails when a remote
 * datacenter is down. */
#define PHP_SCYLLADB_DEFAULT_CONSISTENCY CASS_CONSISTENCY_LOCAL_QUORUM

/* String-literal concatenation forces these to stay as #define. */
#define PHP_SCYLLADB_DEFAULT_LOG PHP_SCYLLADB_NAME ".log"
#define PHP_SCYLLADB_DEFAULT_LOG_LEVEL "ERROR"

/* INI defaults. PHP_INI_ENTRY needs a string literal but GINIT needs the number,
 * and the two must never drift, so each value is defined once as a number and
 * stringified for the INI table. A rejected php.ini value falls back to exactly
 * these, which is what keeps ini_get() and the driver in agreement. */
#define PHP_SCYLLADB_STR_(x) #x
#define PHP_SCYLLADB_STR(x) PHP_SCYLLADB_STR_(x)

#define PHP_SCYLLADB_DEFAULT_PORT_N 9042
#define PHP_SCYLLADB_DEFAULT_CONNECT_TIMEOUT_N 5000
#define PHP_SCYLLADB_DEFAULT_REQUEST_TIMEOUT_N 12000
#define PHP_SCYLLADB_DEFAULT_PAGE_SIZE_N 5000
#define PHP_SCYLLADB_DEFAULT_PROTOCOL_VERSION_N 4
#define PHP_SCYLLADB_DEFAULT_IO_THREADS_N 1
#define PHP_SCYLLADB_DEFAULT_CORE_CONNECTIONS_PER_HOST_N 1
#define PHP_SCYLLADB_DEFAULT_MAX_CONNECTIONS_PER_HOST_N 2
#define PHP_SCYLLADB_DEFAULT_RECONNECT_INTERVAL_N 2000
#define PHP_SCYLLADB_DEFAULT_HEARTBEAT_INTERVAL_N 30
#define PHP_SCYLLADB_DEFAULT_TCP_KEEPALIVE_DELAY_N 0
/* Runaway guards, not tuning knobs: set about ten times what a well-behaved
 * application uses, so nobody legitimate reaches one. Sized from the measured
 * per-entry cost, per PHP worker:
 *   cluster (built, not connected) ~12 KB
 *   session (connected)            ~2.2 MB, plus 2 sockets per node
 *   prepared statement             ~7.4 KB
 * The caps therefore bound a worker at roughly 35 MB of sessions and 7 MB of
 * prepared statements. Past a cap the resource still works, it is just rebuilt
 * per request, so raising one is safe. */
/* Every value below is the C driver's own default, so exposing the setting
 * does not change behaviour on its own. */
#define PHP_SCYLLADB_DEFAULT_CONNECTION_IDLE_TIMEOUT_N 60
#define PHP_SCYLLADB_DEFAULT_MAX_SCHEMA_WAIT_TIME_N 10000
#define PHP_SCYLLADB_DEFAULT_RESOLVE_TIMEOUT_N 2000
#define PHP_SCYLLADB_DEFAULT_MONITOR_REPORTING_INTERVAL_N 300
#define PHP_SCYLLADB_DEFAULT_QUEUE_SIZE_IO_N 8192
#define PHP_SCYLLADB_DEFAULT_TRACING_MAX_WAIT_N 15
#define PHP_SCYLLADB_DEFAULT_TRACING_RETRY_WAIT_N 3
#define PHP_SCYLLADB_DEFAULT_TRACING_CONSISTENCY CASS_CONSISTENCY_ONE
#define PHP_SCYLLADB_DEFAULT_TRACING_CONSISTENCY_NAME "ONE"

#define PHP_SCYLLADB_DEFAULT_CONNECTION_IDLE_TIMEOUT \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_CONNECTION_IDLE_TIMEOUT_N)
#define PHP_SCYLLADB_DEFAULT_MAX_SCHEMA_WAIT_TIME \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_MAX_SCHEMA_WAIT_TIME_N)
#define PHP_SCYLLADB_DEFAULT_RESOLVE_TIMEOUT PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_RESOLVE_TIMEOUT_N)
#define PHP_SCYLLADB_DEFAULT_MONITOR_REPORTING_INTERVAL \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_MONITOR_REPORTING_INTERVAL_N)
#define PHP_SCYLLADB_DEFAULT_QUEUE_SIZE_IO PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_QUEUE_SIZE_IO_N)
#define PHP_SCYLLADB_DEFAULT_TRACING_MAX_WAIT PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_TRACING_MAX_WAIT_N)
#define PHP_SCYLLADB_DEFAULT_TRACING_RETRY_WAIT PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_TRACING_RETRY_WAIT_N)

/* C driver defaults for the settings we newly expose. */
#define PHP_SCYLLADB_DEFAULT_RECONNECT_MAX_INTERVAL_N 60000
#define PHP_SCYLLADB_DEFAULT_SPECULATIVE_DELAY_N 0
#define PHP_SCYLLADB_DEFAULT_SPECULATIVE_MAX_N 2
#define PHP_SCYLLADB_DEFAULT_COALESCE_DELAY_N 200
#define PHP_SCYLLADB_DEFAULT_NEW_REQUEST_RATIO_N 50

#define PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_CLUSTERS_N 16
#define PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_SESSIONS_N 16
#define PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_PREPARED_N 1000

#define PHP_SCYLLADB_DEFAULT_CONSISTENCY_NAME "LOCAL_QUORUM"
#define PHP_SCYLLADB_DEFAULT_CONTACT_POINTS "127.0.0.1"
#define PHP_SCYLLADB_DEFAULT_PORT PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_PORT_N)
#define PHP_SCYLLADB_DEFAULT_CONNECT_TIMEOUT PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_CONNECT_TIMEOUT_N)
#define PHP_SCYLLADB_DEFAULT_REQUEST_TIMEOUT PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_REQUEST_TIMEOUT_N)
#define PHP_SCYLLADB_DEFAULT_PAGE_SIZE PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_PAGE_SIZE_N)
#define PHP_SCYLLADB_DEFAULT_PROTOCOL_VERSION PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_PROTOCOL_VERSION_N)
#define PHP_SCYLLADB_DEFAULT_IO_THREADS PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_IO_THREADS_N)
#define PHP_SCYLLADB_DEFAULT_CORE_CONNECTIONS_PER_HOST \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_CORE_CONNECTIONS_PER_HOST_N)
#define PHP_SCYLLADB_DEFAULT_MAX_CONNECTIONS_PER_HOST \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_MAX_CONNECTIONS_PER_HOST_N)
#define PHP_SCYLLADB_DEFAULT_RECONNECT_INTERVAL PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_RECONNECT_INTERVAL_N)
#define PHP_SCYLLADB_DEFAULT_HEARTBEAT_INTERVAL PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_HEARTBEAT_INTERVAL_N)
#define PHP_SCYLLADB_DEFAULT_TCP_KEEPALIVE_DELAY \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_TCP_KEEPALIVE_DELAY_N)
#define PHP_SCYLLADB_DEFAULT_RECONNECT_MAX_INTERVAL \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_RECONNECT_MAX_INTERVAL_N)
#define PHP_SCYLLADB_DEFAULT_SPECULATIVE_DELAY PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_SPECULATIVE_DELAY_N)
#define PHP_SCYLLADB_DEFAULT_SPECULATIVE_MAX PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_SPECULATIVE_MAX_N)
#define PHP_SCYLLADB_DEFAULT_COALESCE_DELAY PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_COALESCE_DELAY_N)
#define PHP_SCYLLADB_DEFAULT_NEW_REQUEST_RATIO PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_NEW_REQUEST_RATIO_N)

#define PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_CLUSTERS \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_CLUSTERS_N)
#define PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_SESSIONS \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_SESSIONS_N)
#define PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_PREPARED \
    PHP_SCYLLADB_STR(PHP_SCYLLADB_DEFAULT_MAX_PERSISTENT_PREPARED_N)
