<?php
/**
 * ScyllaDB PHP Driver - Performance Comparison Test
 * Compares performance between ScyllaDB and Cassandra C++ drivers
 */

echo "ScyllaDB PHP Driver - Performance Comparison\n";
echo "============================================\n";
echo "PHP Version: " . PHP_VERSION . "\n";
echo "Extension Version: " . phpversion('cassandra') . "\n\n";

function benchmark($name, $iterations, $callback) {
    echo "Testing: $name ($iterations iterations)\n";
    
    // Warm up
    for ($i = 0; $i < 100; $i++) {
        $callback();
    }
    
    // Measure
    $start = microtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        $callback();
    }
    $end = microtime(true);
    
    $total_time = $end - $start;
    $ops_per_sec = $iterations / $total_time;
    
    printf("  Time: %.4f seconds\n", $total_time);
    printf("  Operations/sec: %.0f\n", $ops_per_sec);
    printf("  μs per operation: %.2f\n\n", ($total_time / $iterations) * 1000000);
    
    return $ops_per_sec;
}

// Test 1: Basic UUID creation
$uuid_ops = benchmark("UUID Creation", 50000, function() {
    new Cassandra\Uuid();
});

// Test 2: Bigint creation
$bigint_ops = benchmark("Bigint Creation", 50000, function() {
    new Cassandra\Bigint(123456789);
});

// Test 3: Collection creation
$collection_ops = benchmark("Collection Creation", 10000, function() {
    $set = new Cassandra\Set(Cassandra\Type::text());
    $set->add("test1");
    $set->add("test2");
    $set->add("test3");
});

// Test 4: Map operations
$map_ops = benchmark("Map Operations", 10000, function() {
    $map = new Cassandra\Map(Cassandra\Type::text(), Cassandra\Type::int());
    $map->set("key1", 100);
    $map->set("key2", 200);
    $map->set("key3", 300);
});

// Test 5: Timestamp operations
$timestamp_ops = benchmark("Timestamp Operations", 25000, function() {
    new Cassandra\Timestamp(time());
});

// Test 6: Memory allocation stress
$memory_ops = benchmark("Memory Stress (Mixed Objects)", 5000, function() {
    $uuid = new Cassandra\Uuid();
    $bigint = new Cassandra\Bigint(rand(1, 1000000));
    $timestamp = new Cassandra\Timestamp(time());
    $blob = new Cassandra\Blob("test data " . rand(1, 1000));
    
    // Force some operations
    $string_uuid = (string) $uuid;
    $string_bigint = (string) $bigint;
    $string_timestamp = (string) $timestamp;
    $string_blob = (string) $blob;
});

echo "=== Performance Summary ===\n";
printf("UUID Creation:          %.0f ops/sec\n", $uuid_ops);
printf("Bigint Creation:        %.0f ops/sec\n", $bigint_ops);
printf("Collection Operations:  %.0f ops/sec\n", $collection_ops);
printf("Map Operations:         %.0f ops/sec\n", $map_ops);
printf("Timestamp Operations:   %.0f ops/sec\n", $timestamp_ops);
printf("Memory Stress Test:     %.0f ops/sec\n", $memory_ops);

echo "\n=== System Info ===\n";
echo "Memory Usage: " . round(memory_get_peak_usage(true) / 1024 / 1024, 2) . " MB\n";
echo "CPU Architecture: " . php_uname('m') . "\n";
echo "Operating System: " . php_uname('s') . " " . php_uname('r') . "\n";

// Check if we can identify which driver is being used
$reflection = new ReflectionExtension('cassandra');
echo "Extension Info: " . $reflection->getVersion() . "\n";

echo "\nTest completed successfully!\n";
?> 