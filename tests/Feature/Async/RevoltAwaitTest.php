<?php

declare(strict_types=1);

use Cassandra\Async\Revolt;
use Cassandra\SimpleStatement;
use Revolt\EventLoop;

/**
 * The framework-agnostic fiber-based await (the ergonomic default).
 */

it('awaits a Cassandra future by suspending a fiber', function () {
    $session = scyllaDbConnection();
    $version = null;

    EventLoop::queue(function () use ($session, &$version) {
        $rows    = Revolt::await($session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')));
        $version = $rows->first()['release_version'] ?? null;
    });
    EventLoop::run();

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'revolt')
    ->skip(! class_exists(EventLoop::class), 'revolt/event-loop not installed');

it('awaits many futures concurrently with awaitAll', function () {
    $session  = scyllaDbConnection();
    $resolved = 0;

    EventLoop::queue(function () use ($session, &$resolved) {
        $futures = [];
        for ($i = 0; $i < 10; $i++) {
            $futures[] = $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local'));
        }

        $results  = Revolt::awaitAll($futures);
        $resolved = count($results);

        foreach ($results as $rows) {
            expect($rows->count())->toBeGreaterThan(0);
        }
    });
    EventLoop::run();

    expect($resolved)->toBe(10);
})->group('feature', 'async', 'revolt')
    ->skip(! class_exists(EventLoop::class), 'revolt/event-loop not installed');

it('propagates a driver error out of await()', function () {
    $session = scyllaDbConnection();
    $error   = null;

    EventLoop::queue(function () use ($session, &$error) {
        try {
            Revolt::await($session->executeAsync(new SimpleStatement('SELECT * FROM nope_ks_zz.nope')));
        } catch (\Throwable $e) {
            $error = $e;
        }
    });
    EventLoop::run();

    expect($error)->toBeInstanceOf(\Throwable::class);
})->group('feature', 'async', 'revolt')
    ->skip(! class_exists(EventLoop::class), 'revolt/event-loop not installed');
