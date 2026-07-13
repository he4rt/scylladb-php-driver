<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Offline;

use Cassandra;
use Cassandra\BatchStatement;
use Cassandra\SimpleStatement;
use Cassandra\Uuid;
use Cassandra\Benchmarks\Support\Sizes;
use PhpBench\Attributes as Bench;

/**
 * Statement object construction — the allocation cost before anything is sent.
 *
 * SimpleStatement is a thin CQL wrapper; BatchStatement accumulates entries
 * and copies their argument arrays. Building a large batch is a common
 * ingestion pattern, so `benchBatchAdd` scales the entry count to show the
 * per-row overhead of assembling a batch independent of the network.
 */
#[Bench\Groups(['offline', 'statement'])]
#[Bench\Iterations(5)]
#[Bench\Warmup(2)]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class StatementBench
{
    use Sizes;

    private string $cql = 'INSERT INTO events (id, kind, payload, created) VALUES (?, ?, ?, ?)';

    #[Bench\Revs(20000)]
    public function benchSimpleStatement(): void
    {
        new SimpleStatement($this->cql);
    }

    #[Bench\Revs(20000)]
    public function benchEmptyBatch(): void
    {
        new BatchStatement(Cassandra::BATCH_UNLOGGED);
    }

    #[Bench\Revs(500)]
    #[Bench\ParamProviders('provideElementCounts')]
    public function benchBatchAdd(array $params): void
    {
        $batch = new BatchStatement(Cassandra::BATCH_UNLOGGED);
        $stmt  = new SimpleStatement($this->cql);

        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $batch->add($stmt, [new Uuid(), 'evt', 'payload', $i]);
        }
    }
}
