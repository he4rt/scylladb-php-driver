<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Live;

use Cassandra;
use Cassandra\BatchStatement;
use Cassandra\Bigint;
use Cassandra\PreparedStatement;
use Cassandra\SimpleStatement;
use Cassandra\Uuid;
use Cassandra\Benchmarks\Support\Env;
use Cassandra\Benchmarks\Support\LiveBenchCase;
use PhpBench\Attributes as Bench;

/**
 * Write-path throughput: single prepared insert vs batched inserts.
 *
 * benchSingleInsert is the per-row round-trip cost. benchBatchInsert scales
 * the batch size so you can read the amortised per-row cost of an UNLOGGED
 * batch (mode / count) and find the point of diminishing returns for your
 * cluster. All rows use fresh UUIDs, so runs don't collide and the table just
 * grows — see the README for cleanup.
 */
#[Bench\BeforeMethods('setUp')]
#[Bench\AfterMethods('closeSession')]
#[Bench\Groups(['live', 'write'])]
#[Bench\Iterations(5)]
#[Bench\Warmup(1)]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class WriteBench extends LiveBenchCase
{
    private const TABLE = 'bench_writes';

    private PreparedStatement $insert;

    public function setUp(): void
    {
        $keyspace = Env::keyspace();

        $this->ensureSchema([
            sprintf(
                'CREATE TABLE IF NOT EXISTS %s.%s '
                    . '(id uuid PRIMARY KEY, kind text, payload text, created bigint)',
                $keyspace,
                self::TABLE,
            ),
        ]);

        $this->insert = $this->session->prepare(
            'INSERT INTO ' . self::TABLE . ' (id, kind, payload, created) VALUES (?, ?, ?, ?)',
        );
    }

    #[Bench\Revs(50)]
    public function benchSingleInsert(): void
    {
        $this->session->execute($this->insert, [
            'arguments' => [new Uuid(), 'evt', 'payload', new Bigint(1)],
        ]);
    }

    #[Bench\Revs(10)]
    #[Bench\ParamProviders('provideBatchSizes')]
    public function benchBatchInsert(array $params): void
    {
        $batch = new BatchStatement(Cassandra::BATCH_UNLOGGED);

        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $batch->add($this->insert, [new Uuid(), 'evt', 'payload', new Bigint($i)]);
        }

        $this->session->execute($batch);
    }

    /** @return iterable<string, array{count: int}> */
    public function provideBatchSizes(): iterable
    {
        yield '10'  => ['count' => 10];
        yield '50'  => ['count' => 50];
        yield '100' => ['count' => 100];
    }
}
