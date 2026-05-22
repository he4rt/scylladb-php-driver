#pragma once

#include <cassandra.h>
#include <gmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <version.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>
#include <math.h>
#include <string.h>

#include <php.h>

#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_types.h>

#if PHP_VERSION_ID < 80100
#error PHP 8.1.0 or later is required in order to build the driver
#endif


#define PHP_DRIVER_NAMESPACE "Cassandra"

#define PHP_DRIVER_NAMESPACE_ZEND_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null)                               \
    ZEND_ARG_OBJ_INFO(pass_by_ref, name, Cassandra\\classname, allow_null)

#define PHP_DRIVER_CORE_METHOD(name) PHP_METHOD(Cassandra, name)

#define PHP_DRIVER_CORE_ME(name, arg_info, flags) PHP_ME(Cassandra, name, arg_info, flags)

#define SAFE_STR(a) ((a) ? a : "")

#define SAFE_ZEND_STRING(a) ((a != NULL) ? ZSTR_VAL(a) : "")

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define PHP_DRIVER_G(v) TSRMG(php_driver_globals_id, zend_php_driver_globals *, v)
#else
#define PHP_DRIVER_G(v) (php_driver_globals.v)
#endif

#define CPP_DRIVER_VERSION(major, minor, patch) (((major) << 16) + ((minor) << 8) + (patch))

#define CURRENT_CPP_DRIVER_VERSION CPP_DRIVER_VERSION(CASS_VERSION_MAJOR, CASS_VERSION_MINOR, CASS_VERSION_PATCH)

typedef unsigned long ulong;

    extern zend_module_entry php_driver_module_entry;
#define phpext_cassandra_ptr &php_driver_module_entry

    PHP_MINIT_FUNCTION(php_driver);
    PHP_MSHUTDOWN_FUNCTION(php_driver);
    PHP_RINIT_FUNCTION(php_driver);
    PHP_RSHUTDOWN_FUNCTION(php_driver);
    PHP_MINFO_FUNCTION(php_driver);
    PHP_INI_MH(OnUpdateLogLevel);
    PHP_INI_MH(OnUpdateLog);

    zend_class_entry *exception_class(CassError rc);

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

#define PHP_DRIVER_DEFAULT_CONSISTENCY CASS_CONSISTENCY_LOCAL_ONE

#define PHP_DRIVER_DEFAULT_LOG PHP_DRIVER_NAME ".log"
#define PHP_DRIVER_DEFAULT_LOG_LEVEL "ERROR"


#ifdef __cplusplus
}
#endif
