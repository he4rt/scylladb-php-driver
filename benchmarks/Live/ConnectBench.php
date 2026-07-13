<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Live;

use Cassandra\Benchmarks\Support\LiveBenchCase;
use PhpBench\Attributes as Bench;

/**
 * Session establishment cost, persistent sessions ON vs OFF.
 *
 * Each revolution does a full build() + connect() + close(). With persistent
 * sessions OFF every revolution spins up a fresh CassSession (control
 * connection, host discovery). With them ON, the first revolution in the
 * iteration's subprocess populates EG(persistent_list) and the rest reuse the
 * cached CassCluster/CassSession keyed by the 1.4.0 uint64 fingerprint — so
 * the ON subject should be dramatically cheaper from the second rev onward,
 * and its mem_peak should stay flat.
 *
 * Revolution counts are deliberately low: connect() is orders of magnitude
 * more expensive than an offline value construction.
 */
#[Bench\Groups(['live', 'connect'])]
#[Bench\Iterations(3)]
#[Bench\OutputTimeUnit('milliseconds', precision: 3)]
final class ConnectBench extends LiveBenchCase
{
    #[Bench\Revs(10)]
    #[Bench\Warmup(1)]
    public function benchConnectTransient(): void
    {
        $this->connect(persistent: false)->close();
    }

    #[Bench\Revs(20)]
    #[Bench\Warmup(1)]
    public function benchConnectPersistent(): void
    {
        // close() returns the session to the persistent pool; the next build()
        // with the same config fingerprint reuses it.
        $this->connect(persistent: true)->close();
    }
}
