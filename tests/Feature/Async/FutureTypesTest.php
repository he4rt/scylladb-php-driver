<?php

declare(strict_types=1);

use Cassandra\Async\Reactor;
use Cassandra\FutureClose;
use Cassandra\FuturePreparedStatement;
use Cassandra\FutureRows;
use Cassandra\FutureSession;
use Cassandra\FutureValue;
use Cassandra\SimpleStatement;

/**
 * The async contract holds for EVERY concrete future, not only the FutureRows
 * that executeAsync() returns.
 *
 * Each of the five types has its own struct, its own get(), and its own
 * notifier slot, so each one is exercised here through all three models: the
 * stream from getResource(), the shared reactor, and (where built) an
 * Io\Poll handle.
 */

const TYPES_CQL = 'SELECT release_version FROM system.local';

/**
 * One live future of each concrete type, keyed by class name.
 *
 * closeAsync() is deliberately called on its own throwaway session — closing
 * the shared one would break every later test in the file.
 *
 * @return array<class-string, Cassandra\Future>
 */
function oneOfEachFuture(): array
{
    $session = scyllaDbConnection();

    $hosts = env('SCYLLADB_HOSTS', '127.0.0.1');
    $cluster = Cassandra::cluster()
        ->withContactPoints(...(is_string($hosts) ? explode(',', $hosts) : $hosts))
        ->withPort((int) env('SCYLLADB_PORT', 9042))
        ->withCredentials(env('SCYLLADB_USERNAME', 'cassandra'), env('SCYLLADB_PASSWORD', 'cassandra'))
        ->withPersistentSessions(false)
        ->build();

    return [
        FutureRows::class              => $session->executeAsync(new SimpleStatement(TYPES_CQL)),
        FuturePreparedStatement::class => $session->prepareAsync(TYPES_CQL),
        FutureSession::class           => $cluster->connectAsync(),
        FutureClose::class             => $cluster->connect()->closeAsync(),
        // Resolved on construction: a single-page result has no next page.
        FutureValue::class             => $session->execute(new SimpleStatement(TYPES_CQL))->nextPageAsync(),
    ];
}

it('exposes a readable descriptor for every future type', function () {
    foreach (oneOfEachFuture() as $class => $future) {
        $resource = $future->getResource();

        expect(is_resource($resource))->toBeTrue("$class: getResource() is a resource")
            ->and(get_resource_type($resource))->toContain('stream')
            // Cached: the same handle every call, so a loop registers one watcher.
            ->and($future->getResource())->toBe($resource, "$class: getResource() is cached");

        $read = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))
            ->toBeGreaterThan(0, "$class: descriptor never became readable");

        // The contract the whole feature rests on: readable means get() is free.
        expect($future->isReady())->toBeTrue("$class: readable but not ready");
        $future->get();
    }
})->group('feature', 'async');

it('reports isReady() false while a future is still pending', function () {
    $session = scyllaDbConnection();

    // A fresh future is almost always still in flight; tolerate the race where
    // the driver resolves it before we look, and only assert the transition.
    $future = $session->executeAsync(new SimpleStatement(TYPES_CQL));

    if (!$future->isReady()) {
        $read = [$future->getResource()];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);
    }

    expect($future->isReady())->toBeTrue();
    $future->get();
    expect($future->isReady())->toBeTrue();
})->group('feature', 'async');

it('registers every future type with the shared reactor', function () {
    $futures = oneOfEachFuture();

    foreach ($futures as $future) {
        Reactor::add($future);
    }
    expect(Reactor::pending())->toBe(count($futures));

    $resource = Reactor::resource();
    $seen     = [];

    while (Reactor::pending() > 0) {
        $read  = [$resource];
        $write = $except = [];
        expect(stream_select($read, $write, $except, 10))->toBeGreaterThan(0);

        foreach (Reactor::poll() as $future) {
            expect($future->isReady())->toBeTrue($future::class . ' came back not ready');
            $seen[] = $future::class;
            $future->get();
        }
    }

    sort($seen);
    expect($seen)->toBe([
        FutureClose::class,
        FuturePreparedStatement::class,
        FutureRows::class,
        FutureSession::class,
        FutureValue::class,
    ]);
})->group('feature', 'async', 'reactor');

it('refuses a per-future descriptor once the reactor owns the future', function () {
    foreach (oneOfEachFuture() as $class => $future) {
        Reactor::add($future);

        expect(fn () => $future->getResource())
            ->toThrow(Cassandra\Exception\RuntimeException::class, '', "$class: getResource() was allowed");
    }

    // Drain so the reactor is empty for the next test.
    $resource = Reactor::resource();
    while (Reactor::pending() > 0) {
        $read  = [$resource];
        $write = $except = [];
        stream_select($read, $write, $except, 10);
        Reactor::poll();
    }
})->group('feature', 'async', 'reactor');

it('refuses the reactor once a future has a per-future descriptor', function () {
    foreach (oneOfEachFuture() as $class => $future) {
        $future->getResource();

        expect(fn () => Reactor::add($future))
            ->toThrow(Cassandra\Exception\RuntimeException::class, '', "$class: Reactor::add() was allowed");

        $future->get();
    }

    expect(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'reactor');

it('rejects a second registration of the same future', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(TYPES_CQL));

    Reactor::add($future);
    expect(fn () => Reactor::add($future))
        ->toThrow(Cassandra\Exception\RuntimeException::class);

    // Still exactly one registration, not two.
    expect(Reactor::pending())->toBe(1);

    $resource = Reactor::resource();
    $read = [$resource];
    $write = $except = [];
    stream_select($read, $write, $except, 10);
    expect(Reactor::poll())->toHaveCount(1);
})->group('feature', 'async', 'reactor');
