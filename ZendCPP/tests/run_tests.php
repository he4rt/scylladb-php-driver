#!/usr/bin/env php
<?php
/**
 * ZendCPP Test Runner
 *
 * Discovers and runs all tests registered in the extension
 */

if (!extension_loaded('zendcpp_test')) {
    echo "Error: zendcpp_test extension not loaded\n";
    echo "Load it with: php -d extension=modules/zendcpp_test.so\n";
    exit(1);
}

class TestRunner {
    private $passed = 0;
    private $failed = 0;
    private $errors = [];

    public function run() {
        echo "==========================================\n";
        echo "ZendCPP Test Suite\n";
        echo "==========================================\n\n";

        $tests = zendcpp_list_tests();
        foreach ($tests as $category => $test_names) {
            $this->runCategory($category, $test_names);
        }

        $this->printSummary();

        return $this->failed === 0 ? 0 : 1;
    }

    private function runCategory($category, $test_names) {
        echo "--- $category Tests ---\n";

        foreach ($test_names as $test_name) {
            $this->runTest($category, $test_name);
        }

        echo "\n";
    }

    private function runTest($category, $test_name) {
        echo "  Testing: $test_name ... ";

        try {
            $result = zendcpp_run_test($category, $test_name);

            if ($result === true) {
                echo "✓ PASS\n";
                $this->passed++;
            } else {
                echo "✗ FAIL\n";
                $this->failed++;
                $this->errors[] = "$category::$test_name - Unknown failure";
            }
        } catch (Exception $e) {
            echo "✗ FAIL\n";
            $this->failed++;
            $this->errors[] = "$category::$test_name - " . $e->getMessage();
        }
    }

    private function printSummary() {
        echo "==========================================\n";
        echo "Test Summary\n";
        echo "==========================================\n";
        echo "Passed: {$this->passed}\n";
        echo "Failed: {$this->failed}\n";
        echo "Total:  " . ($this->passed + $this->failed) . "\n";

        if (!empty($this->errors)) {
            echo "\n--- Failures ---\n";
            foreach ($this->errors as $error) {
                echo "  ✗ $error\n";
            }
        }

        echo "\n";

        if ($this->failed === 0) {
            echo "✓ ALL TESTS PASSED!\n";
        } else {
            echo "✗ SOME TESTS FAILED!\n";
        }
    }
}

$runner = new TestRunner();
exit($runner->run());
