<?php

declare(strict_types=1);

use Cassandra\FutureRows;
use Cassandra\SimpleStatement;

/**
 * Low-level async primitive: Future::getResource() / Future::isReady().
 *
 * These tests need no event-loop framework — they prove the extension turns
 * "the driver resolved this future on its IO thread" into "this php stream is
 * now readable", which is the foundation every loop adapter builds on.
 */

const ASYNC_READ_CQL = 'SELECT release_version FROM system.local';

it('exposes a readable, cached stream resource from getResource()', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));

    expect($future)->toBeInstanceOf(FutureRows::class);

    $resource = $future->getResource();

    expect(is_resource($resource))->toBeTrue()
        ->and(get_resource_type($resource))->toContain('stream')
        // getResource() is idempotent: same underlying resource each call.
        ->and($future->getResource())->toBe($resource);

    $future->get();
    expect($future->isReady())->toBeTrue();
})->group('feature', 'async');

it('signals completion via stream_select, after which get() does not block', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));

    $read = [$future->getResource()];
    $write = $except = [];

    $ready = stream_select($read, $write, $except, 5);

    expect($ready)->toBe(1)
        ->and($future->isReady())->toBeTrue();

    expect($future->get()->count())->toBeGreaterThan(0);
})->group('feature', 'async');

it('multiplexes many concurrent futures through a single stream_select loop', function () {
    $session = scyllaDbConnection();
    $count   = 16;

    /** @var array<int, array{0: FutureRows, 1: resource}> $pending */
    $pending = [];
    for ($i = 0; $i < $count; $i++) {
        $future            = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));
        $resource          = $future->getResource();
        $pending[(int) $resource] = [$future, $resource];
    }

    $resolved = 0;
    while ($pending !== []) {
        $read  = array_map(static fn (array $p) => $p[1], $pending);
        $write = $except = [];

        $ready = stream_select($read, $write, $except, 5);
        expect($ready)->toBeGreaterThan(0);

        foreach ($read as $resource) {
            [$future] = $pending[(int) $resource];
            expect($future->get()->count())->toBeGreaterThan(0);
            unset($pending[(int) $resource]);
            $resolved++;
        }
    }

    expect($resolved)->toBe($count);
})->group('feature', 'async');

it('survives dropping a future (and its stream) before it resolves', function () {
    $session = scyllaDbConnection();

    // Grab the notification fd, then discard the future without ever calling
    // get(). The driver still resolves the query on its IO thread and fires the
    // completion callback afterwards; the notifier keeps its own read end open
    // so that late write can never raise SIGPIPE against a closed pipe.
    for ($i = 0; $i < 200; $i++) {
        $future   = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));
        $resource = $future->getResource();
        unset($future, $resource);
    }

    // Let any in-flight callbacks fire into the now reader-less pipes.
    usleep(100_000);

    expect(true)->toBeTrue(); // reaching here means no SIGPIPE / crash
})->group('feature', 'async');

it('notifies on a failed query and rethrows the driver error from get()', function () {
    $session = scyllaDbConnection();
    // Syntactically valid CQL against a missing keyspace/table — the failure
    // surfaces when the future resolves, not at executeAsync().
    $future = $session->executeAsync(
        new SimpleStatement('SELECT * FROM nonexistent_keyspace_zzz.nonexistent_table'),
    );

    $read = [$future->getResource()];
    $write = $except = [];

    // The fd must still become readable even though the future resolved to an error.
    expect(stream_select($read, $write, $except, 5))->toBe(1);

    expect(fn () => $future->get())->toThrow(Exception::class);
})->group('feature', 'async');

it('keeps the descriptor readable until the result is consumed', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));

    $resource = $future->getResource();

    $read = [$resource];
    $write = $except = [];
    expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

    // Level-triggered: nothing drains the descriptor on the driver's behalf, so
    // a loop that wakes twice before dispatching still sees the completion the
    // second time. A one-shot edge would lose it.
    for ($i = 0; $i < 3; $i++) {
        $read = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 0))
            ->toBeGreaterThan(0, "descriptor went quiet on poll $i");
    }

    $future->get();
})->group('feature', 'async');

it('returns the same result from repeated get() calls', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));

    $first  = $future->get();
    $second = $future->get();

    expect($first->count())->toBe($second->count())
        ->and($first->first())->toBe($second->first());

    // Iterating one must not move the other's cursor.
    foreach ($first as $_) {
    }
    $seen = 0;
    foreach ($second as $_) {
        $seen++;
    }
    expect($seen)->toBe($second->count());
})->group('feature', 'async');

it('honours a timeout on get() for a future that never resolves in time', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));

    // A zero/near-zero timeout either wins the race or times out; both are
    // correct. What must never happen is a hang or a wrong-typed error.
    try {
        $rows = $future->get(0.000001);
        expect($rows->count())->toBeGreaterThanOrEqual(0);
    } catch (Cassandra\Exception\TimeoutException $e) {
        expect($e->getMessage())->not->toBeEmpty();
        // The future is still usable after a timeout.
        expect($future->get(10)->count())->toBeGreaterThan(0);
    }
})->group('feature', 'async');

it('lets the descriptor outlive a future that is never read', function () {
    $session = scyllaDbConnection();

    $resource = null;
    (function () use ($session, &$resource) {
        $future   = $session->executeAsync(new SimpleStatement(ASYNC_READ_CQL));
        $resource = $future->getResource();
    })();

    gc_collect_cycles();

    // The stream owns its own dup of the read end, so it stays a valid resource
    // after the future is gone. The driver's late write lands on the notifier's
    // own copy and cannot raise SIGPIPE.
    expect(is_resource($resource))->toBeTrue();

    $read = [$resource];
    $write = $except = [];
    stream_select($read, $write, $except, 5);
})->group('feature', 'async');
