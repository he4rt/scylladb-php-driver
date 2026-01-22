/**
 * Main test extension entry point
 */

#include <Zend/zend_API.h>
#include <php.h>

#include "cases/ArrayTest.cpp"
#include "cases/ExceptionTest.cpp"
#include "cases/HashTableTest.cpp"
#include "cases/StringBuilderTest.cpp"
#include "cases/ZArrayTest.cpp"
#include "cases/ZValTest.cpp"
#include "framework/TestFramework.hpp"
#include "framework/TestFunctions.h"

BEGIN_EXTERN_C()
// Include all test files - they auto-register themselves
// Module entry
zend_module_entry zendcpp_test_module_entry = {STANDARD_MODULE_HEADER,
                                               "zendcpp_test",
                                               test_framework_functions,
                                               nullptr,  // MINIT
                                               nullptr,  // MSHUTDOWN
                                               nullptr,  // RINIT
                                               nullptr,  // RSHUTDOWN
                                               nullptr,  // MINFO
                                               "1.0.0",
                                               STANDARD_MODULE_PROPERTIES};

ZEND_GET_MODULE(zendcpp_test)
END_EXTERN_C()