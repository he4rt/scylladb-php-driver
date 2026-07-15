<?php

declare(strict_types=1);

use Cassandra\Async\Swoole;
use Cassandra\SimpleStatement;

/**
 * Swoole / OpenSwoole coroutine integration. Runs when ext-swoole or
 * ext-openswoole is loaded (CI builds the extension with the native flag against
 * the matching source and loads the runtime); skipped otherwise.
 */

/** Run $fn inside a coroutine on whichever runtime is loaded. */
function scyllaSwooleRun(Closure $fn): void
{
    if (function_exists('Swoole\Coroutine\run')) {
        \Swoole\Coroutine\run($fn);
        return;
    }
    if (function_exists('OpenSwoole\Coroutine\run')) {
        \OpenSwoole\Coroutine\run($fn);
        return;
    }
    if (is_callable(['OpenSwoole\Coroutine', 'run'])) {
        \OpenSwoole\Coroutine::run($fn);
        return;
    }
    if (is_callable(['Swoole\Coroutine', 'run'])) {
        \Swoole\Coroutine::run($fn);
        return;
    }
    throw new RuntimeException('No swoole/openswoole coroutine runner available');
}

function scyllaSwooleMissing(): bool
{
    return ! extension_loaded('swoole') && ! extension_loaded('openswoole');
}

it('makes Future::get() coroutine-aware under a native build', function () {
    // With -DPHP_SCYLLADB_ENABLE_SWOOLE/_OPENSWOOLE, get() suspends the current
    // coroutine (via swoole::coroutine::System::wait_event) instead of blocking
    // the worker — so plain blocking-style code cooperates with the scheduler.
    $version = null;

    scyllaSwooleRun(function () use (&$version) {
        $session = scyllaDbConnection();
        $rows    = $session
            ->executeAsync(new SimpleStatement('SELECT release_version FROM system.local'))
            ->get();
        $version = $rows->first()['release_version'] ?? null;
    });

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'swoole')
    ->skip(scyllaSwooleMissing(), 'swoole / openswoole extension not installed');

it('awaits a Cassandra future via the Swoole adapter', function () {
    $version = null;

    scyllaSwooleRun(function () use (&$version) {
        $session = scyllaDbConnection();
        $rows    = Swoole::await(
            $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')),
        );
        $version = $rows->first()['release_version'] ?? null;
    });

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'swoole')
    ->skip(scyllaSwooleMissing(), 'swoole / openswoole extension not installed');

it('overlaps many in-flight futures inside one coroutine', function () {
    // Fire N async reads, then await them: each get() suspends this coroutine
    // rather than blocking, and all N requests are in flight concurrently.
    $count = 0;

    scyllaSwooleRun(function () use (&$count) {
        $session = scyllaDbConnection();

        $futures = [];
        for ($i = 0; $i < 16; $i++) {
            $futures[] = $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local'));
        }

        foreach ($futures as $future) {
            if ($future->get()->count() > 0) {
                $count++;
            }
        }
    });

    expect($count)->toBe(16);
})->group('feature', 'async', 'swoole')
    ->skip(scyllaSwooleMissing(), 'swoole / openswoole extension not installed');
