<?php

declare(strict_types=1);

use Cassandra\Async\Reactor;
use Cassandra\Async\ReactorRevolt;
use Cassandra\FuturePreparedStatement;
use Cassandra\FutureRows;
use Cassandra\FutureValue;
use Cassandra\SimpleStatement;
use Revolt\EventLoop;

const REACTOR_CQL = 'SELECT release_version FROM system.local';

/**
 * Shared reactor: one fd multiplexes many in-flight futures.
 */

it('returns a single, stable resource for the whole reactor', function () {
    $resource = Reactor::resource();

    expect(is_resource($resource))->toBeTrue()
        ->and(get_resource_type($resource))->toContain('stream')
        // resource() is cached — same handle every call (one watcher for the loop).
        ->and(Reactor::resource())->toBe($resource);
})->group('feature', 'async', 'reactor');

it('dispatches many concurrent futures through one fd', function () {
    $session = scyllaDbConnection();
    $count   = 256;

    for ($i = 0; $i < $count; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    }
    expect(Reactor::pending())->toBe($count);

    $resource = Reactor::resource();
    $done     = 0;

    while (Reactor::pending() > 0) {
        $read = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 5))->toBeGreaterThan(0);

        foreach (Reactor::poll() as $future) {
            expect($future)->toBeInstanceOf(FutureRows::class)
                ->and($future->get()->count())->toBeGreaterThan(0);
            $done++;
        }
    }

    expect($done)->toBe($count)
        ->and(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'reactor');

it('caps a batch and keeps the rest readable for the next tick', function () {
    $session = scyllaDbConnection();
    $count   = 32;

    for ($i = 0; $i < $count; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    }

    $resource = Reactor::resource();
    $batches  = [];
    $done     = 0;

    while (Reactor::pending() > 0) {
        $read = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 5))->toBeGreaterThan(0);

        $batch = Reactor::poll(4);
        expect(count($batch))->toBeLessThanOrEqual(4);

        $batches[] = count($batch);
        $done += count($batch);
    }

    // A capped drain must not lose or duplicate completions, and 32 futures
    // cannot arrive in one batch of 4.
    expect($done)->toBe($count)
        ->and(count($batches))->toBeGreaterThan(1);
})->group('feature', 'async', 'reactor');

it('calls the completion callback instead of returning the future', function () {
    $session = scyllaDbConnection();
    $count   = 16;

    $called = 0;
    for ($i = 0; $i < $count; $i++) {
        // Half push through a callback, half stay pull-style.
        Reactor::add(
            $session->executeAsync(new SimpleStatement(REACTOR_CQL)),
            $i % 2 === 0
                ? function (FutureRows $future) use (&$called) {
                    expect($future->get()->count())->toBeGreaterThan(0);
                    $called++;
                }
                : null,
        );
    }

    $resource = Reactor::resource();
    $returned = 0;

    while (Reactor::pending() > 0) {
        $read = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 5))->toBeGreaterThan(0);

        foreach (Reactor::poll() as $future) {
            expect($future->get()->count())->toBeGreaterThan(0);
            $returned++;
        }
    }

    expect($called)->toBe($count / 2)
        ->and($returned)->toBe($count / 2);
})->group('feature', 'async', 'reactor');

it('stops the batch when a callback throws and keeps the rest queued', function () {
    $session = scyllaDbConnection();

    for ($i = 0; $i < 8; $i++) {
        Reactor::add(
            $session->executeAsync(new SimpleStatement(REACTOR_CQL)),
            fn () => throw new LogicException('boom'),
        );
    }

    $thrown = 0;
    while (Reactor::pending() > 0) {
        try {
            Reactor::poll();
        } catch (LogicException) {
            $thrown++;
        }

        expect($thrown)->toBeLessThanOrEqual(8);
    }

    // Every future was dispatched, one throw at a time — none lost, none stuck.
    expect($thrown)->toBe(8)
        ->and(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'reactor');

it('rejects a non-positive batch size', function () {
    expect(fn () => Reactor::poll(0))->toThrow(ValueError::class);
})->group('feature', 'async', 'reactor');

it('handles completions from multiple driver IO threads', function () {
    // io_threads>1 makes the driver fire completion callbacks from several
    // threads concurrently — exercises the MPSC queue's multi-producer path.
    $session = Cassandra::cluster()
        ->withContactPoints(...explode(',', (string) env('SCYLLADB_HOSTS', '127.0.0.1')))
        ->withPort((int) env('SCYLLADB_PORT', 9042))
        ->withCredentials(env('SCYLLADB_USERNAME', 'cassandra'), env('SCYLLADB_PASSWORD', 'cassandra'))
        ->withIOThreads(4)
        ->withPersistentSessions(false)
        ->build()
        ->connect();

    $count = 512;
    for ($i = 0; $i < $count; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    }

    $resource = Reactor::resource();
    $done     = 0;
    while (Reactor::pending() > 0) {
        $read = [$resource];
        $write = $except = [];
        stream_select($read, $write, $except, 5);
        foreach (Reactor::poll() as $future) {
            $future->get();
            $done++;
        }
    }

    expect($done)->toBe($count);
})->group('feature', 'async', 'reactor');

it('surfaces a failed query through poll() and get()', function () {
    $session = scyllaDbConnection();
    Reactor::add($session->executeAsync(new SimpleStatement('SELECT * FROM nope_ks_zz.nope')));

    // Loop until the future is dispatched — the reactor's eventfd is
    // level-triggered and can report a spurious wake (harmless empty poll), so a
    // single select+poll is not guaranteed to return the result on the first tick.
    $resource = Reactor::resource();
    $ready    = [];
    while (Reactor::pending() > 0) {
        $read = [$resource];
        $write = $except = [];
        if (stream_select($read, $write, $except, 5) < 1) {
            break;
        }
        foreach (Reactor::poll() as $future) {
            $ready[] = $future;
        }
    }

    expect($ready)->toHaveCount(1)
        ->and(fn () => $ready[0]->get())->toThrow(Exception::class)
        ->and(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'reactor');

it('rejects mixing the reactor with per-future getResource()', function () {
    $session = scyllaDbConnection();

    // getResource() first → reactor add must reject.
    $a = $session->executeAsync(new SimpleStatement(REACTOR_CQL));
    $a->getResource();
    expect(fn () => Reactor::add($a))->toThrow(Exception::class);

    // reactor add first → getResource() must reject.
    $b = $session->executeAsync(new SimpleStatement(REACTOR_CQL));
    Reactor::add($b);
    expect(fn () => $b->getResource())->toThrow(Exception::class);

    // drain so we don't leave b pending for the next test.
    $resource = Reactor::resource();
    while (Reactor::pending() > 0) {
        $read = [$resource];
        $write = $except = [];
        stream_select($read, $write, $except, 5);
        foreach (Reactor::poll() as $f) {
            $f->get();
        }
    }
    $a->get();
})->group('feature', 'async', 'reactor');

it('awaits many futures concurrently via the Revolt reactor adapter', function () {
    $session  = scyllaDbConnection();
    $resolved = 0;

    EventLoop::queue(function () use ($session, &$resolved) {
        $futures = [];
        for ($i = 0; $i < 50; $i++) {
            $futures[] = $session->executeAsync(new SimpleStatement(REACTOR_CQL));
        }

        $results  = ReactorRevolt::awaitAll($futures);
        $resolved = count($results);
        foreach ($results as $rows) {
            expect($rows->count())->toBeGreaterThan(0);
        }
    });
    EventLoop::run();

    expect($resolved)->toBe(50);
})->group('feature', 'async', 'reactor', 'revolt')
    ->skip(! class_exists(EventLoop::class), 'revolt/event-loop not installed');

it('reclaims collected futures when a later callback throws', function () {
    $session = scyllaDbConnection();

    // Registered without callbacks: poll() returns these.
    for ($i = 0; $i < 6; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    }
    // Registered last, with a throwing callback: it aborts the batch after the
    // six above have already been collected into poll()'s return array.
    Reactor::add(
        $session->executeAsync(new SimpleStatement(REACTOR_CQL)),
        fn () => throw new LogicException('boom'),
    );

    $resource = Reactor::resource();
    $returned = 0;
    $thrown   = 0;

    while (Reactor::pending() > 0) {
        $read  = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

        try {
            foreach (Reactor::poll() as $future) {
                expect($future->isReady())->toBeTrue();
                $returned++;
            }
        } catch (LogicException) {
            $thrown++;
        }
    }

    // The throw discarded poll()'s array, but nothing was dropped: every
    // callback-less future came back on a later tick.
    expect($thrown)->toBe(1)
        ->and($returned)->toBe(6)
        ->and(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'reactor');

it('registers every concrete future type', function () {
    $session = scyllaDbConnection();

    Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    Reactor::add($session->prepareAsync(REACTOR_CQL));

    expect(Reactor::pending())->toBe(2);

    $resource = Reactor::resource();
    $seen     = [];

    while (Reactor::pending() > 0) {
        $read  = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

        foreach (Reactor::poll() as $future) {
            $seen[] = $future::class;
            $future->get();
        }
    }

    sort($seen);
    expect($seen)->toBe([FuturePreparedStatement::class, FutureRows::class]);
})->group('feature', 'async', 'reactor');

it('reports an already-resolved FutureValue through poll()', function () {
    $session = scyllaDbConnection();

    // A single-page result: nextPageAsync() resolves on construction, so the
    // reactor has no driver callback to wait for.
    $future = $session->execute(new SimpleStatement(REACTOR_CQL))->nextPageAsync();
    expect($future)->toBeInstanceOf(FutureValue::class)
        ->and($future->isReady())->toBeTrue();

    Reactor::add($future);
    expect(Reactor::pending())->toBe(1);

    $polled = Reactor::poll();

    expect($polled)->toHaveCount(1)
        ->and($polled[0])->toBe($future)
        ->and(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'reactor');

it('rejects a future already bound to a per-future descriptor', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(REACTOR_CQL));

    $future->getResource();

    expect(fn () => Reactor::add($future))
        ->toThrow(Cassandra\Exception\RuntimeException::class);

    $future->get();
})->group('feature', 'async', 'reactor');

it('returns an empty batch when nothing is registered', function () {
    expect(Reactor::pending())->toBe(0)
        ->and(Reactor::poll())->toBe([])
        ->and(Reactor::poll(1))->toBe([]);
})->group('feature', 'async', 'reactor');

it('keeps a future alive after userland drops its last reference', function () {
    $session = scyllaDbConnection();

    Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    // Nothing in PHP holds the future now — only the reactor's own pin does.
    expect(Reactor::pending())->toBe(1);

    gc_collect_cycles();

    $resource = Reactor::resource();
    $read  = [$resource];
    $write = $except = [];
    expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

    $polled = Reactor::poll();
    expect($polled)->toHaveCount(1)
        ->and($polled[0])->toBeInstanceOf(FutureRows::class)
        ->and($polled[0]->get()->count())->toBeGreaterThan(0);
})->group('feature', 'async', 'reactor');

it('hands the resolved future to the callback as its only argument', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(REACTOR_CQL));

    $args = null;
    Reactor::add($future, function (...$received) use (&$args) {
        $args = $received;
    });

    $resource = Reactor::resource();
    $read  = [$resource];
    $write = $except = [];
    stream_select($read, $write, $except, 10);
    Reactor::poll();

    expect($args)->toHaveCount(1)
        ->and($args[0])->toBe($future)
        ->and($args[0]->isReady())->toBeTrue();
})->group('feature', 'async', 'reactor');

it('takes exactly $max completions per call', function () {
    $session = scyllaDbConnection();
    $total   = 12;

    for ($i = 0; $i < $total; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    }

    $resource = Reactor::resource();
    $batches  = [];

    while (Reactor::pending() > 0) {
        $read  = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

        $batch = Reactor::poll(5);
        if ($batch !== []) {
            expect(count($batch))->toBeLessThanOrEqual(5);
            $batches[] = count($batch);
        }
        foreach ($batch as $future) {
            $future->get();
        }
    }

    expect(array_sum($batches))->toBe($total);
})->group('feature', 'async', 'reactor');

it('runs the reactor and per-future descriptors side by side', function () {
    $session = scyllaDbConnection();

    // Same loop, two models, different futures — the guard is per future, not
    // per process.
    $viaReactor = 0;
    for ($i = 0; $i < 8; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(REACTOR_CQL)));
    }

    $standalone = [];
    for ($i = 0; $i < 8; $i++) {
        $future                            = $session->executeAsync(new SimpleStatement(REACTOR_CQL));
        $standalone[(int) $future->getResource()] = $future;
    }

    $streams = array_map(fn ($f) => $f->getResource(), $standalone);
    $viaFd   = 0;

    while (Reactor::pending() > 0 || $standalone !== []) {
        $read  = array_values($streams);
        if (Reactor::pending() > 0) {
            $read[] = Reactor::resource();
        }
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

        foreach (Reactor::poll() as $future) {
            $future->get();
            $viaReactor++;
        }
        foreach ($read as $stream) {
            $id = (int) $stream;
            if (isset($standalone[$id])) {
                $standalone[$id]->get();
                unset($standalone[$id], $streams[$id]);
                $viaFd++;
            }
        }
    }

    expect($viaReactor)->toBe(8)->and($viaFd)->toBe(8);
})->group('feature', 'async', 'reactor');
