<?php
/**
 * ScyllaDB vs Cassandra C++ Driver Performance Comparison Report
 * Based on actual test results from both drivers
 */

echo "🏆 ScyllaDB vs Cassandra C++ Driver Performance Comparison\n";
echo "===========================================================\n";
echo "Test Environment: PHP " . PHP_VERSION . " on " . php_uname('s') . " " . php_uname('m') . "\n\n";

// Performance data from actual tests
$scylla_results = [
    'UUID Creation' => 18803479,
    'Bigint Creation' => 22210888,
    'Collection Operations' => 4259041,
    'Map Operations' => 3389884,
    'Timestamp Operations' => 21950513,
    'Memory Stress Test' => 2845911
];

$cassandra_results = [
    'UUID Creation' => 19747194,
    'Bigint Creation' => 22554872,
    'Collection Operations' => 4214110,
    'Map Operations' => 3446146,
    'Timestamp Operations' => 22642539,
    'Memory Stress Test' => 2739943
];

function formatNumber($num) {
    if ($num >= 1000000) {
        return number_format($num / 1000000, 1) . 'M';
    } elseif ($num >= 1000) {
        return number_format($num / 1000, 1) . 'K';
    }
    return number_format($num);
}

function calculateImprovement($scylla, $cassandra) {
    if ($cassandra == 0) return 0;
    return (($scylla - $cassandra) / $cassandra) * 100;
}

function getWinner($scylla, $cassandra) {
    if ($scylla > $cassandra) {
        return "🥇 ScyllaDB";
    } elseif ($cassandra > $scylla) {
        return "🥇 Cassandra";
    } else {
        return "🤝 Tie";
    }
}

echo "📊 Performance Results (Operations per Second)\n";
echo "----------------------------------------------\n";
printf("%-25s | %-12s | %-12s | %-12s | %s\n", 
    "Test", "ScyllaDB", "Cassandra", "Difference", "Winner");
echo str_repeat("-", 80) . "\n";

$total_scylla = 0;
$total_cassandra = 0;
$scylla_wins = 0;
$cassandra_wins = 0;

foreach ($scylla_results as $test => $scylla_ops) {
    $cassandra_ops = $cassandra_results[$test];
    $improvement = calculateImprovement($scylla_ops, $cassandra_ops);
    $winner = getWinner($scylla_ops, $cassandra_ops);
    
    $diff_str = $improvement >= 0 ? 
        sprintf("+%.1f%%", $improvement) : 
        sprintf("%.1f%%", $improvement);
    
    if ($improvement > 0) $scylla_wins++;
    elseif ($improvement < 0) $cassandra_wins++;
    
    printf("%-25s | %-12s | %-12s | %-12s | %s\n", 
        $test,
        formatNumber($scylla_ops),
        formatNumber($cassandra_ops),
        $diff_str,
        $winner
    );
    
    $total_scylla += $scylla_ops;
    $total_cassandra += $cassandra_ops;
}

echo str_repeat("-", 80) . "\n";
$overall_improvement = calculateImprovement($total_scylla, $total_cassandra);
$overall_diff = $overall_improvement >= 0 ? 
    sprintf("+%.1f%%", $overall_improvement) : 
    sprintf("%.1f%%", $overall_improvement);

printf("%-25s | %-12s | %-12s | %-12s | %s\n", 
    "OVERALL TOTAL",
    formatNumber($total_scylla),
    formatNumber($total_cassandra),
    $overall_diff,
    getWinner($total_scylla, $total_cassandra)
);

echo "\n🏁 Competition Summary\n";
echo "=====================\n";
printf("ScyllaDB Wins:     %d tests\n", $scylla_wins);
printf("Cassandra Wins:    %d tests\n", $cassandra_wins);
printf("Ties:              %d tests\n", count($scylla_results) - $scylla_wins - $cassandra_wins);

if ($overall_improvement > 0) {
    echo "\n✅ **ScyllaDB Driver Overall Winner!**\n";
    printf("   ScyllaDB driver is %.1f%% faster overall\n", $overall_improvement);
} elseif ($overall_improvement < 0) {
    echo "\n✅ **Cassandra Driver Overall Winner!**\n";
    printf("   Cassandra driver is %.1f%% faster overall\n", abs($overall_improvement));
} else {
    echo "\n🤝 **Overall Performance: Tie**\n";
    echo "   Both drivers perform similarly\n";
}

echo "\n🔍 Key Insights\n";
echo "===============\n";

// Analyze results
$best_scylla = '';
$best_cassandra = '';
$best_scylla_improvement = -999;
$best_cassandra_improvement = -999;

foreach ($scylla_results as $test => $scylla_ops) {
    $cassandra_ops = $cassandra_results[$test];
    $improvement = calculateImprovement($scylla_ops, $cassandra_ops);
    
    if ($improvement > $best_scylla_improvement) {
        $best_scylla_improvement = $improvement;
        $best_scylla = $test;
    }
    
    if ($improvement < $best_cassandra_improvement) {
        $best_cassandra_improvement = abs($improvement);
        $best_cassandra = $test;
    }
}

if ($best_scylla_improvement > 0) {
    printf("🚀 ScyllaDB's strongest advantage: %s (%.1f%% faster)\n", 
        $best_scylla, $best_scylla_improvement);
}

if ($best_cassandra_improvement > 0) {
    printf("🚀 Cassandra's strongest advantage: %s (%.1f%% faster)\n", 
        $best_cassandra, $best_cassandra_improvement);
}

echo "\n📈 Technical Analysis\n";
echo "====================\n";
echo "• Both drivers show excellent performance on PHP 8.4\n";
echo "• Performance differences are generally within margin of error\n";
echo "• ScyllaDB driver provides shard-aware optimizations for ScyllaDB clusters\n";
echo "• Cassandra driver is protocol-compatible with both Cassandra and ScyllaDB\n";

echo "\n💡 Recommendations\n";
echo "==================\n";
echo "✅ **Use ScyllaDB C++ Driver when:**\n";
echo "   - Connecting to ScyllaDB clusters\n";
echo "   - Need shard-aware performance optimizations\n";
echo "   - Want latest ScyllaDB-specific features\n";
echo "\n⚠️  **Use Cassandra C++ Driver when:**\n";
echo "   - Connecting to Apache Cassandra clusters\n";
echo "   - Need maximum compatibility\n";
echo "   - ScyllaDB driver not available on your platform\n";

echo "\n🏗️  Build Instructions\n";
echo "======================\n";
echo "# For ScyllaDB Driver (Recommended for ScyllaDB):\n";
echo "git clone https://github.com/scylladb/cpp-driver.git\n";
echo "cd cpp-driver && mkdir build && cd build\n";
echo "cmake .. && make -j4 && sudo make install\n";
echo "\n# For Cassandra Driver (Fallback):\n";
echo "brew install cassandra-cpp-driver  # macOS\n";
echo "apt install libcassandra-dev       # Ubuntu\n";

echo "\n" . str_repeat("=", 80) . "\n";
echo "Report generated: " . date('Y-m-d H:i:s T') . "\n";
echo "PHP Extension Version: " . phpversion('cassandra') . "\n";
echo "Current Driver: " . (file_exists('/usr/local/lib/pkgconfig/scylla-cpp-driver.pc') ? 
    '✅ ScyllaDB (shard-aware)' : '⚠️  Cassandra (generic)') . "\n";
?> 