<?php

declare(strict_types=1);

use Cassandra\Async\Reactor;
use Cassandra\Async\ReactorRevolt;
use Cassandra\FutureRows;
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

    $resource = Reactor::resource();
    $read = [$resource];
    $write = $except = [];
    expect(stream_select($read, $write, $except, 5))->toBeGreaterThan(0);

    $ready = Reactor::poll();
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
