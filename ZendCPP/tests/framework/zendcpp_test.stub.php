<?php

/** @generate-function-entries */

/**
 * Run a specific test by category and name
 *
 * @param string $category Test category
 * @param string $test_name Test name
 * @return bool True if test passed
 * @throws Exception If test fails or not found
 */
function zendcpp_run_test(string $category, string $test_name): bool {}

/**
 * List all registered tests
 *
 * @return array Array of tests organized by category
 */
function zendcpp_list_tests(): array {}

/**
 * Run all tests in a category
 *
 * @param string $category Test category to run
 * @return array Summary with 'passed', 'failed', and 'results' keys
 */
function zendcpp_run_category(string $category): array {}
