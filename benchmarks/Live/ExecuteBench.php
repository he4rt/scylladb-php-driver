<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Live;

use Cassandra\Bigint;
use Cassandra\PreparedStatement;
use Cassandra\SimpleStatement;
use Cassandra\Uuid;
use Cassandra\Benchmarks\Support\Env;
use Cassandra\Benchmarks\Support\LiveBenchCase;
use PhpBench\Attributes as Bench;

/**
 * Read-path throughput: simple vs prepared, and the cost of binding.
 *
 * A SimpleStatement is re-parsed server-side on every execute; a
 * PreparedStatement is parsed once and only bound values travel afterwards.
 * The gap between benchSimpleSelect and benchPreparedSelect is the headline
 * "why prepare" number. benchPreparedSelectBound adds positional binding so
 * the marshalling cost is visible on top of the round trip.
 */
#[Bench\BeforeMethods('setUp')]
#[Bench\AfterMethods('closeSession')]
#[Bench\Groups(['live', 'execute'])]
#[Bench\Revs(50)]
#[Bench\Iterations(5)]
#[Bench\Warmup(1)]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class ExecuteBench extends LiveBenchCase
{
    private const TABLE = 'bench_events';

    private SimpleStatement $simpleSelect;
    private PreparedStatement $preparedSelect;
    private PreparedStatement $preparedById;
    private Uuid $seedId;

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

        // Seed a single deterministic row so every SELECT returns exactly one.
        $this->seedId = new Uuid('62c36092-82a1-3a00-93d1-46196ee77204');
        $this->session->execute(
            new SimpleStatement(sprintf(
                'INSERT INTO %s (id, kind, payload, created) VALUES (%s, ?, ?, ?)',
                self::TABLE,
                (string) $this->seedId,
            )),
            ['arguments' => ['seed', 'hello', new Bigint(1)]],
        );

        $this->simpleSelect   = new SimpleStatement('SELECT id, kind, payload FROM ' . self::TABLE . ' LIMIT 1');
        $this->preparedSelect = $this->session->prepare('SELECT id, kind, payload FROM ' . self::TABLE . ' LIMIT 1');
        $this->preparedById   = $this->session->prepare('SELECT id, kind, payload FROM ' . self::TABLE . ' WHERE id = ?');
    }

    public function benchSimpleSelect(): void
    {
        $this->drain($this->session->execute($this->simpleSelect));
    }

    public function benchPreparedSelect(): void
    {
        $this->drain($this->session->execute($this->preparedSelect));
    }

    public function benchPreparedSelectBound(): void
    {
        $this->drain($this->session->execute($this->preparedById, ['arguments' => [$this->seedId]]));
    }

    private function drain(iterable $rows): void
    {
        foreach ($rows as $_) {
            // materialise every row so decoding is included in the measurement
        }
    }
}
