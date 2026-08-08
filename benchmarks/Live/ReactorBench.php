<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Live;

use Cassandra\Async\Reactor;
use Cassandra\FutureRows;
use Cassandra\PreparedStatement;
use Cassandra\SimpleStatement;
use Cassandra\Uuid;
use Cassandra\Benchmarks\Support\LiveBenchCase;
use PhpBench\Attributes as Bench;

/**
 * Shared reactor (O(1) fds) vs per-future notification fds, at increasing fan-out.
 *
 * benchPerFutureFds fires N async reads and awaits them with one stream_select
 * loop over N separate Future::getResource() fds — the fd count grows with N.
 * benchSharedReactor fires the same N, registers them with the shared reactor,
 * and awaits them through its single fd. At matched N (64/256) both have
 * near-identical, network-bound latency.
 *
 * The per-future approach is bounded: stream_select() cannot watch more than
 * FD_SETSIZE fds (~512 on macOS/typical libc), so benchPerFutureFds is only run
 * up to 256. benchSharedReactorHighFanout drives the reactor to 1024/4096 — it
 * watches ONE fd regardless of N, so it scales cleanly past that ceiling. That
 * unbounded fan-out is the reactor's whole point.
 */
#[Bench\BeforeMethods('setUp')]
#[Bench\AfterMethods('closeSession')]
#[Bench\Groups(['live', 'async', 'reactor'])]
#[Bench\Revs(3)]
#[Bench\Iterations(3)]
#[Bench\Warmup(1)]
#[Bench\OutputTimeUnit('milliseconds', precision: 3)]
final class ReactorBench extends LiveBenchCase
{
    private const TABLE = 'bench_reactor';

    private PreparedStatement $select;
    private Uuid $seedId;

    public function setUp(): void
    {
        $keyspace = \Cassandra\Benchmarks\Support\Env::keyspace();

        $this->ensureSchema([
            sprintf(
                'CREATE TABLE IF NOT EXISTS %s.%s (id uuid PRIMARY KEY, payload text)',
                $keyspace,
                self::TABLE,
            ),
        ]);

        $this->seedId = new Uuid('22222222-2222-2222-2222-222222222222');
        $this->session->execute(
            new SimpleStatement('INSERT INTO ' . self::TABLE . ' (id, payload) VALUES (' . $this->seedId . ', ?)'),
            ['arguments' => ['hello']],
        );

        $this->select = $this->session->prepare('SELECT id, payload FROM ' . self::TABLE . ' WHERE id = ?');
    }

    #[Bench\ParamProviders('provideConcurrency')]
    public function benchPerFutureFds(array $params): void
    {
        /** @var array<int, FutureRows> $pending */
        $pending   = [];
        $resources = [];
        for ($i = 0, $n = $params['requests']; $i < $n; $i++) {
            $future            = $this->session->executeAsync($this->select, ['arguments' => [$this->seedId]]);
            $resource          = $future->getResource();
            $id                = (int) $resource;
            $pending[$id]      = $future;
            $resources[$id]    = $resource;
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

    #[Bench\ParamProviders('provideConcurrency')]
    public function benchSharedReactor(array $params): void
    {
        $this->drainViaReactor($params['requests']);
    }

    /**
     * Reactor-only fan-out well past FD_SETSIZE — impossible with per-future fds.
     */
    #[Bench\ParamProviders('provideHighFanout')]
    public function benchSharedReactorHighFanout(array $params): void
    {
        $this->drainViaReactor($params['requests']);
    }

    private function drainViaReactor(int $requests): void
    {
        for ($i = 0; $i < $requests; $i++) {
            Reactor::add($this->session->executeAsync($this->select, ['arguments' => [$this->seedId]]));
        }

        $resource = Reactor::resource();
        while (Reactor::pending() > 0) {
            $read  = [$resource];
            $write = $except = [];
            if (stream_select($read, $write, $except, 5) < 1) {
                break;
            }
            foreach (Reactor::poll() as $future) {
                foreach ($future->get() as $_) {
                }
            }
        }
    }

    /** Matched fan-out for the per-future vs reactor comparison (within FD_SETSIZE). */
    /** @return iterable<string, array{requests: int}> */
    public function provideConcurrency(): iterable
    {
        yield '64'  => ['requests' => 64];
        yield '256' => ['requests' => 256];
    }

    /** @return iterable<string, array{requests: int}> */
    public function provideHighFanout(): iterable
    {
        yield '1024' => ['requests' => 1024];
        yield '4096' => ['requests' => 4096];
    }
}
