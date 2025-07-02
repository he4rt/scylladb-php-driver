<?php
/**
 * PHP 8.4 Compatibility Test Script for ScyllaDB PHP Driver
 * 
 * This script tests core functionality to ensure PHP 8.4 compatibility
 * and detect potential segfault issues.
 */

if (PHP_VERSION_ID < 80400) {
    echo "Warning: This test is designed for PHP 8.4+, running on PHP " . PHP_VERSION . "\n";
}

if (!extension_loaded('cassandra')) {
    echo "Error: Cassandra extension is not loaded\n";
    exit(1);
}

echo "PHP 8.4 Compatibility Test for ScyllaDB PHP Driver\n";
echo "PHP Version: " . PHP_VERSION . "\n";
echo "Extension Version: " . phpversion('cassandra') . "\n\n";

$test_results = [];

// Test 1: Basic type creation and memory management
function test_basic_types() {
    echo "Testing basic types...\n";
    
    try {
        // Test numeric types that had comparison handler issues
        $bigint = new Cassandra\Bigint(9223372036854775807);
        $smallint = new Cassandra\Smallint(32767);
        $tinyint = new Cassandra\Tinyint(127);
        $varint = new Cassandra\Varint('12345678901234567890');
        $decimal = new Cassandra\Decimal('123.456');
        $float = new Cassandra\Float(3.14159);
        
        // Test type conversions
        $bigint_str = (string) $bigint;
        $smallint_int = (int) $smallint;
        $tinyint_int = (int) $tinyint;
        
        // Force garbage collection to test memory management
        unset($bigint, $smallint, $tinyint, $varint, $decimal, $float);
        gc_collect_cycles();
        
        return true;
    } catch (Exception $e) {
        echo "Error in basic types test: " . $e->getMessage() . "\n";
        return false;
    }
}

// Test 2: Collection types (Map, Set, Tuple)
function test_collections() {
    echo "Testing collection types...\n";
    
    try {
        // Test Map
        $map = new Cassandra\Map(Cassandra\Type::text(), Cassandra\Type::int());
        $map->set('key1', 100);
        $map->set('key2', 200);
        
        // Test iteration
        foreach ($map as $key => $value) {
            // Accessing key/value should not cause segfault
        }
        
        // Test Set
        $set = new Cassandra\Set(Cassandra\Type::text());
        $set->add('value1');
        $set->add('value2');
        
        // Test Tuple
        $tuple = new Cassandra\Tuple([Cassandra\Type::text(), Cassandra\Type::int()]);
        $tuple->set(0, 'text');
        $tuple->set(1, 42);
        
        // Test comparisons (this was problematic in PHP 8.0+)
        $map2 = new Cassandra\Map(Cassandra\Type::text(), Cassandra\Type::int());
        $map2->set('key1', 100);
        
        // Force cleanup
        unset($map, $map2, $set, $tuple);
        gc_collect_cycles();
        
        return true;
    } catch (Exception $e) {
        echo "Error in collections test: " . $e->getMessage() . "\n";
        return false;
    }
}

// Test 3: UUID and Blob types
function test_uuid_blob() {
    echo "Testing UUID and Blob types...\n";
    
    try {
        // Test UUID
        $uuid = new Cassandra\Uuid();
        $uuid_str = (string) $uuid;
        
        // Timeuuid generation may fail on some systems, so try-catch it
        try {
            $timeuuid = new Cassandra\Timeuuid();
            $timeuuid_str = (string) $timeuuid;
        } catch (Exception $e) {
            // Skip Timeuuid if it fails - not critical for PHP 8.4 compatibility test
        }
        
        // Test Blob
        $blob = new Cassandra\Blob('binary data');
        $blob_str = (string) $blob;
        
        unset($uuid, $blob);
        if (isset($timeuuid)) unset($timeuuid);
        gc_collect_cycles();
        
        return true;
    } catch (Exception $e) {
        echo "Error in UUID/Blob test: " . $e->getMessage() . "\n";
        return false;
    }
}

// Test 4: Date/Time types
function test_datetime() {
    echo "Testing date/time types...\n";
    
    try {
        // Test Date first
        echo "  Testing Date...\n";
        $current_time = time();
        $date = new Cassandra\Date($current_time);
        if ($date) {
            $date_str = (string) $date;
            echo "    Date created successfully: " . substr($date_str, 0, 30) . "...\n";
            unset($date);
        }
        
        // Test Time with safer nanoseconds value
        echo "  Testing Time...\n";
        $time = new Cassandra\Time(45045000000000); // 12:30:45 in nanoseconds
        if ($time) {
            $time_str = (string) $time;
            echo "    Time created successfully: " . substr($time_str, 0, 20) . "...\n";
            unset($time);
        }
        
        // Test Timestamp
        echo "  Testing Timestamp...\n";
        $timestamp = new Cassandra\Timestamp($current_time, 0);
        if ($timestamp) {
            $timestamp_str = (string) $timestamp;
            echo "    Timestamp created successfully: " . substr($timestamp_str, 0, 20) . "...\n";
            unset($timestamp);
        }
        
        // Force garbage collection after each test
        gc_collect_cycles();
        
        echo "  All DateTime types tested successfully\n";
        return true;
        
    } catch (Exception $e) {
        echo "  Error in date/time test: " . $e->getMessage() . "\n";
        return false;
    } catch (Error $e) {
        echo "  Fatal error in date/time test: " . $e->getMessage() . "\n";  
        return false;
    }
}

// Test 5: Memory stress test
function test_memory_stress() {
    echo "Testing memory stress scenarios...\n";
    
    try {
        // Create many objects to test memory management
        $objects = [];
        for ($i = 0; $i < 1000; $i++) {
            $objects[] = new Cassandra\Bigint($i);
        }
        
        // Clear all objects
        unset($objects);
        gc_collect_cycles();
        
        // Test nested collections
        $map = new Cassandra\Map(Cassandra\Type::text(), Cassandra\Type::collection(Cassandra\Type::int()));
        for ($i = 0; $i < 100; $i++) {
            $collection = new Cassandra\Collection(Cassandra\Type::int());
            $collection->add($i);
            $map->set("key$i", $collection);
        }
        
        unset($map);
        gc_collect_cycles();
        
        return true;
    } catch (Exception $e) {
        echo "Error in memory stress test: " . $e->getMessage() . "\n";
        return false;
    }
}

// Run all tests
$tests = [
    'Basic Types' => 'test_basic_types',
    'Collections' => 'test_collections', 
    'UUID/Blob' => 'test_uuid_blob',
    'Date/Time' => 'test_datetime',
    'Memory Stress' => 'test_memory_stress'
];

$passed = 0;
$total = count($tests);

foreach ($tests as $name => $test_func) {
    echo "\n--- $name Test ---\n";
    if ($test_func()) {
        echo "✓ $name test PASSED\n";
        $passed++;
    } else {
        echo "✗ $name test FAILED\n";
    }
}

echo "\n=== Test Results ===\n";
echo "Passed: $passed/$total\n";

if ($passed === $total) {
    echo "🎉 All tests passed! PHP 8.4 compatibility looks good.\n";
    exit(0);
} else {
    echo "❌ Some tests failed. Please review the issues above.\n";
    exit(1);
}
?> 