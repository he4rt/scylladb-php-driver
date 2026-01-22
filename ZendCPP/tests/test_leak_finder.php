<?php
// Test each ZVal test individually to find the leak

echo "Testing ZVal tests individually...\n\n";

$tests = [
    'ZVal::set_string_zend_string',
    'ZVal::to_string_from_string',
    'ZVal::to_string_from_long',
];

foreach ($tests as $test) {
    list($category, $name) = explode('::', $test);
    echo "Running $test... ";
    flush();

    try {
        zendcpp_run_test($category, $name);
        echo "✓ PASS\n";
    } catch (Exception $e) {
        echo "✗ FAIL: " . $e->getMessage() . "\n";
    }
}

echo "\nDone. Check for memory leaks above.\n";
