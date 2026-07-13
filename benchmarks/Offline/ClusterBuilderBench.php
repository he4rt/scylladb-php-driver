<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Offline;

use Cassandra;
use PhpBench\Attributes as Bench;

/**
 * Cluster builder + build() — configuration cost with no network.
 *
 * build() materialises the whole CassCluster config from the fluent builder.
 * In 1.4.0 the persistent-session cache key moved from a heap-allocated
 * ~200-char string to an inline uint64 FNV-1a fingerprint; that work happens
 * here, on every build(). This benchmark isolates it from connect() so a
 * regression in the config/fingerprint path can't hide behind network noise.
 * Compare the persistent ON vs OFF subjects and watch mem_peak.
 */
#[Bench\Groups(['offline', 'builder'])]
#[Bench\Revs(2000)]
#[Bench\Iterations(5)]
#[Bench\Warmup(2)]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class ClusterBuilderBench
{
    #[Bench\Revs(20000)]
    public function benchBuilderChain(): void
    {
        // Fluent configuration only — no build().
        Cassandra::cluster()
            ->withContactPoints('127.0.0.1')
            ->withPort(9042)
            ->withTokenAwareRouting(true)
            ->withConnectTimeout(5.0)
            ->withRequestTimeout(12.0)
            ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_ONE);
    }

    public function benchBuildPersistentOff(): void
    {
        $this->configure(false)->build();
    }

    public function benchBuildPersistentOn(): void
    {
        $this->configure(true)->build();
    }

    private function configure(bool $persistent): \Cassandra\Cluster\Builder
    {
        return Cassandra::cluster()
            ->withContactPoints('127.0.0.1')
            ->withPort(9042)
            ->withPersistentSessions($persistent)
            ->withTokenAwareRouting(true)
            ->withConnectTimeout(5.0)
            ->withRequestTimeout(12.0)
            ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_ONE);
    }
}
