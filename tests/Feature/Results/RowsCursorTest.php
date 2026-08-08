<?php

declare(strict_types=1);

use Cassandra\Rows;
use Cassandra\SimpleStatement;

/**
 * FutureRows::get() caches the decoded row array and hands every Rows object a
 * refcounted copy of it. The iterator cursor must therefore live on the Rows
 * object, not on the shared array — otherwise two Rows built from one future
 * (or a Rows whose future is still alive) share one position.
 */

const ROWS_CURSOR_CQL = 'SELECT keyspace_name FROM system_schema.keyspaces';

it('iterates fully while the producing future is still alive', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ROWS_CURSOR_CQL));

    $rows = $future->get();
    expect($rows)->toBeInstanceOf(Rows::class);

    $seen = 0;
    foreach ($rows as $_) {
        $seen++;
    }

    expect($seen)->toBe($rows->count())
        ->and($seen)->toBeGreaterThan(0);

    // The future is deliberately kept referenced until here.
    expect($future->isReady())->toBeTrue();
})->group('feature', 'async');

it('gives each Rows from one future an independent cursor', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ROWS_CURSOR_CQL));

    $first  = $future->get();
    $second = $future->get();

    expect($first->count())->toBeGreaterThan(1);

    foreach ($first as $_) {
    }

    // $first is exhausted; $second must still iterate from the beginning.
    $seen = 0;
    foreach ($second as $_) {
        $seen++;
    }

    expect($seen)->toBe($second->count())
        ->and($second->first())->toBe($first->first());
})->group('feature', 'async');

it('keeps cursors independent across many concurrently held futures', function () {
    $session = scyllaDbConnection();

    $futures = [];
    for ($i = 0; $i < 16; $i++) {
        $futures[] = $session->executeAsync(new SimpleStatement(ROWS_CURSOR_CQL));
    }

    $counts = [];
    foreach ($futures as $future) {
        $rows = $future->get();
        $seen = 0;
        foreach ($rows as $_) {
            $seen++;
        }
        $counts[] = $seen;
    }

    expect($counts)->toHaveCount(16)
        ->and(array_unique($counts))->toHaveCount(1)
        ->and($counts[0])->toBeGreaterThan(0);
})->group('feature', 'async');
