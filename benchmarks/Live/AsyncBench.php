<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Live;

use Cassandra\FutureRows;
use Cassandra\PreparedStatement;
use Cassandra\SimpleStatement;
use Cassandra\Uuid;
use Cassandra\Benchmarks\Support\Env;
use Cassandra\Benchmarks\Support\LiveBenchCase;
use PhpBench\Attributes as Bench;

/**
 * Async pipelining vs sequential execution at a fixed request count.
 *
 * Both subjects issue the same number of point reads. benchSequential blocks
 * on each execute() before starting the next, so its latency is N * round_trip.
 * benchPipelined fires all executeAsync() first, then drains the futures, so
 * in-flight requests overlap and latency approaches round_trip + N * server
 * cost. The ratio between them is the real-world win from the async API and
 * the driver's request coalescing.
 */
#[Bench\BeforeMethods('setUp')]
#[Bench\AfterMethods('closeSession')]
#[Bench\Groups(['live', 'async'])]
#[Bench\Revs(5)]
#[Bench\Iterations(3)]
#[Bench\Warmup(1)]
#[Bench\ParamProviders('provideConcurrency')]
#[Bench\OutputTimeUnit('milliseconds', precision: 3)]
final class AsyncBench extends LiveBenchCase
{
    private const TABLE = 'bench_async';

    private PreparedStatement $select;
    private Uuid $seedId;

    public function setUp(): void
    {
        $keyspace = Env::keyspace();

        $this->ensureSchema([
            sprintf(
                'CREATE TABLE IF NOT EXISTS %s.%s (id uuid PRIMARY KEY, payload text)',
                $keyspace,
                self::TABLE,
            ),
        ]);

        $this->seedId = new Uuid('11111111-1111-1111-1111-111111111111');
        $this->session->execute(
            new SimpleStatement('INSERT INTO ' . self::TABLE . ' (id, payload) VALUES (' . $this->seedId . ', ?)'),
            ['arguments' => ['hello']],
        );

        $this->select = $this->session->prepare('SELECT id, payload FROM ' . self::TABLE . ' WHERE id = ?');
    }

    public function benchSequential(array $params): void
    {
        for ($i = 0, $n = $params['requests']; $i < $n; $i++) {
            $rows = $this->session->execute($this->select, ['arguments' => [$this->seedId]]);
            foreach ($rows as $_) {
            }
        }
    }

    public function benchPipelined(array $params): void
    {
        /** @var list<FutureRows> $futures */
        $futures = [];
        for ($i = 0, $n = $params['requests']; $i < $n; $i++) {
            $futures[] = $this->session->executeAsync($this->select, ['arguments' => [$this->seedId]]);
        }

        foreach ($futures as $future) {
            foreach ($future->get() as $_) {
            }
        }
    }

    /**
     * Event-loop style: fire all requests, then await completion by watching
     * each future's notification fd (Future::getResource()) with a single
     * stream_select() loop — the exact path ReactPHP/AMPHP/Swoole take. The
     * delta versus benchPipelined is the overhead of the fd-notification layer
     * (pipe + callback + select) over a blind blocking drain, and unlike
     * benchPipelined it never blocks a thread that a real reactor could use for
     * other IO.
     */
    public function benchEventLoopConcurrent(array $params): void
    {
        /** @var array<int, FutureRows> $pending */
        $pending   = [];
        $resources = [];
        for ($i = 0, $n = $params['requests']; $i < $n; $i++) {
            $future             = $this->session->executeAsync($this->select, ['arguments' => [$this->seedId]]);
            $resource           = $future->getResource();
            $id                 = (int) $resource;
            $pending[$id]       = $future;
            $resources[$id]     = $resource;
        }

        while ($pending !== []) {
            $read  = $resources;
            $write = $except = [];
            if (stream_select($read, $write, $except, 5) < 1) {
                break;
            }

            foreach ($read as $resource) {
                $id = (int) $resource;
                foreach ($pending[$id]->get() as $_) {
                }
                unset($pending[$id], $resources[$id]);
            }
        }
    }

    /** @return iterable<string, array{requests: int}> */
    public function provideConcurrency(): iterable
    {
        yield '16'  => ['requests' => 16];
        yield '64'  => ['requests' => 64];
        yield '256' => ['requests' => 256];
    }
}
