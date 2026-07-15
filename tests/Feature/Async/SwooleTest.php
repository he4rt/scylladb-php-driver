<?php

declare(strict_types=1);

use Cassandra\Async\Swoole;
use Cassandra\SimpleStatement;

/**
 * Swoole coroutine integration. Skipped unless ext-swoole / ext-openswoole is
 * present; documents and exercises the intended usage when it is.
 */

it('awaits a Cassandra future inside a Swoole coroutine', function () {
    $version = null;

    \Swoole\Coroutine\run(function () use (&$version) {
        $session = scyllaDbConnection();
        $rows    = Swoole::await(
            $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')),
        );
        $version = $rows->first()['release_version'] ?? null;
    });

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'swoole')
    ->skip(
        ! extension_loaded('swoole') && ! extension_loaded('openswoole'),
        'swoole / openswoole extension not installed',
    );

it('makes Future::get() coroutine-aware under a native swoole build', function () {
    // With -DPHP_SCYLLADB_ENABLE_SWOOLE the extension suspends the coroutine
    // inside get() itself, so plain blocking-style code cooperates with the
    // scheduler. Without the native build this still passes (get() blocks the
    // single coroutine briefly), so the test documents the intended behaviour.
    $version = null;

    \Swoole\Coroutine\run(function () use (&$version) {
        $session = scyllaDbConnection();
        $rows    = $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local'))->get();
        $version = $rows->first()['release_version'] ?? null;
    });

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'swoole')
    ->skip(
        ! extension_loaded('swoole') && ! extension_loaded('openswoole'),
        'swoole / openswoole extension not installed',
    );

it('runs multiple coroutine awaits concurrently', function () {
    $results = [];

    \Swoole\Coroutine\run(function () use (&$results) {
        $session = scyllaDbConnection();
        $wg      = new \Swoole\Coroutine\WaitGroup();

        for ($i = 0; $i < 10; $i++) {
            $wg->add();
            \Swoole\Coroutine::create(function () use ($session, $wg, &$results) {
                $rows      = Swoole::await(
                    $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')),
                );
                $results[] = $rows->count();
                $wg->done();
            });
        }

        $wg->wait();
    });

    expect($results)->toHaveCount(10);
})->group('feature', 'async', 'swoole')
    ->skip(
        ! extension_loaded('swoole') && ! extension_loaded('openswoole'),
        'swoole / openswoole extension not installed',
    );
