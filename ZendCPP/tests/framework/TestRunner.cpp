/**
 * Test runner - generates PHP functions for all registered tests
 */

#include <Zend/zend_exceptions.h>
#include <php.h>

#include "TestFramework.hpp"
#include "TestFunctions.h"

BEGIN_EXTERN_C()

// PHP function to run a specific test
PHP_FUNCTION(zendcpp_run_test) {
  char* category;
  size_t category_len;
  char* test_name;
  size_t test_name_len;

  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_STRING(category, category_len)
  Z_PARAM_STRING(test_name, test_name_len)
  ZEND_PARSE_PARAMETERS_END();

  auto& registry = ZendCPPTest::TestRegistry::Instance();
  const auto& tests = registry.GetTests();

  for (const auto& test : tests) {
    if (test.category == category && test.name == test_name) {
      try {
        test.cpp_test();
        RETURN_BOOL(true);
      } catch (const std::exception& e) {
        zend_throw_exception(nullptr, e.what(), 0);
        return;
      }
    }
  }

  zend_throw_exception(nullptr, "Test not found", 0);
}

// PHP function to list all tests
PHP_FUNCTION(zendcpp_list_tests) {
  auto& registry = ZendCPPTest::TestRegistry::Instance();
  const auto& tests = registry.GetTests();

  array_init(return_value);

  // Group tests by category
  for (const auto& test : tests) {
    zval* category_array;

    // Check if category already exists in return_value
    category_array = zend_hash_str_find(Z_ARRVAL_P(return_value),
                                        test.category.c_str(),
                                        test.category.length());

    if (!category_array) {
      // Create new array for this category
      zval new_array;
      array_init(&new_array);
      category_array = zend_hash_str_add(Z_ARRVAL_P(return_value),
                                         test.category.c_str(),
                                         test.category.length(),
                                         &new_array);
    }

    // Add test name to category's array
    add_next_index_string(category_array, test.name.c_str());
  }
}

// PHP function to run all tests in a category
PHP_FUNCTION(zendcpp_run_category) {
  char* category;
  size_t category_len;

  ZEND_PARSE_PARAMETERS_START(1, 1)
  Z_PARAM_STRING(category, category_len)
  ZEND_PARSE_PARAMETERS_END();

  auto& registry = ZendCPPTest::TestRegistry::Instance();
  const auto tests = registry.GetTestsByCategory(category);

  array_init(return_value);

  int passed = 0;
  int failed = 0;

  for (const auto& test : tests) {
    try {
      test.cpp_test();
      add_assoc_bool(return_value, test.name.c_str(), true);
      passed++;
    } catch (const std::exception& e) {
      add_assoc_string(return_value, test.name.c_str(), e.what());
      failed++;
    }
  }

  add_assoc_long(return_value, "passed", passed);
  add_assoc_long(return_value, "failed", failed);
}
END_EXTERN_C()