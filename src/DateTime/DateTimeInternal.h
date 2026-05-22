#pragma once

#include <ZendCPP/ZendCPP.hpp>
#include <functional>
#include <string>
#include <string_view>

zend_result scylladb_php_to_datetime_internal(
    zval* dst, const char* format, const std::function<zend_string*()>& get_timestamp) noexcept;