#include <php.h>
#include <ext/date/php_date.h>

#include "DateTimeInternal.h"

#undef vsnprintf

zend_result scylladb_php_to_datetime_internal(
    zval *dst, const char *format,
    scylladb_get_timestamp_fn get_timestamp, void *ctx)
{
    zval datetime;

    if (php_date_instantiate(php_date_get_date_ce(), &datetime) == nullptr) {
        return FAILURE;
    }

    php_date_obj *datetime_obj = Z_PHPDATE_P(&datetime);
    zend_string *timestamp = get_timestamp(ctx);

    if (!php_date_initialize(datetime_obj, ZSTR_VAL(timestamp), ZSTR_LEN(timestamp),
                             format, nullptr, PHP_DATE_INIT_FORMAT)) {
        zend_object_std_dtor(&datetime_obj->std);
        zval_ptr_dtor(&datetime);
        zend_string_release(timestamp);
        return FAILURE;
    }

    zend_string_release(timestamp);
    ZVAL_ZVAL(dst, &datetime, 1, 1);
    return SUCCESS;
}
