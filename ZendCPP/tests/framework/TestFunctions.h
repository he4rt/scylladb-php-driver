/**
 * Test framework function declarations
 * This needs to be in a header to avoid linking issues on macOS
 */

#pragma once

#include <php.h>

BEGIN_EXTERN_C()
// Declare the PHP functions
PHP_FUNCTION(zendcpp_run_test);
PHP_FUNCTION(zendcpp_list_tests);
PHP_FUNCTION(zendcpp_run_category);

// Include the generated arginfo
#include "zendcpp_test_arginfo.h"

// Define the function entries array inline
// This avoids symbol visibility issues on macOS with -undefined dynamic_lookup
static constexpr zend_function_entry test_framework_functions[] = {
    PHP_FE(zendcpp_run_test, arginfo_zendcpp_run_test)
        PHP_FE(zendcpp_list_tests, arginfo_zendcpp_list_tests)
            PHP_FE(zendcpp_run_category, arginfo_zendcpp_run_category) PHP_FE_END};

END_EXTERN_C()