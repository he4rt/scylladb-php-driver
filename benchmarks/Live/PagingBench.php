<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Live;

use Cassandra\Rows;
use Cassandra\SimpleStatement;
use Cassandra\Benchmarks\Support\Env;
use Cassandra\Benchmarks\Support\LiveBenchCase;
use PhpBench\Attributes as Bench;

/**
 * Result paging: full scan of a fixed row set at different page sizes.
 *
 * The table is seeded once (idempotently) to ROWS rows. Each subject walks the
 * entire result set, pulling the next page synchronously until the last. A
 * small page size means more round trips; a large one means fewer, bigger
 * decodes. Sweeping page_size shows where the round-trip/decoding trade-off
 * sits for this schema and cluster.
 */
#[Bench\BeforeMethods('setUp')]
#[Bench\AfterMethods('closeSession')]
#[Bench\Groups(['live', 'paging'])]
#[Bench\Revs(10)]
#[Bench\Iterations(3)]
#[Bench\Warmup(1)]
#[Bench\OutputTimeUnit('milliseconds', precision: 3)]
final class PagingBench extends LiveBenchCase
{
    private const TABLE = 'bench_paging';
    private const ROWS = 1000;

    private SimpleStatement $scan;

    public function setUp(): void
    {
        $keyspace = Env::keyspace();

        $this->ensureSchema([
            sprintf(
                'CREATE TABLE IF NOT EXISTS %s.%s (bucket int, id int, payload text, PRIMARY KEY (bucket, id))',
                $keyspace,
                self::TABLE,
            ),
        ]);

        $this->seedOnce();
        $this->scan = new SimpleStatement('SELECT bucket, id, payload FROM ' . self::TABLE . ' WHERE bucket = 0');
    }

    #[Bench\ParamProviders('providePageSizes')]
    public function benchFullScan(array $params): void
    {
        $rows = $this->session->execute($this->scan, ['page_size' => $params['page_size']]);
        $this->walk($rows);
    }

    private function walk(Rows $rows): void
    {
        while (true) {
            foreach ($rows as $_) {
                // decode every row on the page
            }
            if ($rows->isLastPage()) {
                break;
            }
            $rows = $rows->nextPage();
        }
    }

    private function seedOnce(): void
    {
        $existing = $this->session
            ->execute(new SimpleStatement('SELECT COUNT(*) AS c FROM ' . self::TABLE . ' WHERE bucket = 0'))
            ->first();

        if ((int) ($existing['c'] ?? 0) >= self::ROWS) {
            return;
        }

        $insert = $this->session->prepare('INSERT INTO ' . self::TABLE . ' (bucket, id, payload) VALUES (0, ?, ?)');
        for ($i = 0; $i < self::ROWS; $i++) {
            $this->session->execute($insert, ['arguments' => [$i, 'payload-' . $i]]);
        }
    }

    /** @return iterable<string, array{page_size: int}> */
    public function providePageSizes(): iterable
    {
        yield '50'   => ['page_size' => 50];
        yield '250'  => ['page_size' => 250];
        yield '1000' => ['page_size' => 1000];
    }
}
