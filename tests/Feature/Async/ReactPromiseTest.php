<?php

declare(strict_types=1);

use Cassandra\Async\ReactPhp;
use Cassandra\SimpleStatement;
use React\EventLoop\Loop;

use function React\Promise\all;

it('resolves a React promise when the future completes', function () {
    $session = scyllaDbConnection();
    $loop    = Loop::get();
    $version = null;

    ReactPhp::toPromise($session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')), $loop)
        ->then(function ($rows) use (&$version, $loop) {
            $version = $rows->first()['release_version'] ?? null;
            $loop->stop();
        });
    $loop->run();

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'react')
    ->skip(! class_exists(Loop::class), 'react/event-loop not installed');

it('resolves many promises concurrently', function () {
    $session  = scyllaDbConnection();
    $loop     = Loop::get();
    $resolved = 0;

    $promises = [];
    for ($i = 0; $i < 10; $i++) {
        $promises[] = ReactPhp::toPromise(
            $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')),
            $loop,
        );
    }

    all($promises)->then(function (array $results) use (&$resolved, $loop) {
        $resolved = count($results);
        $loop->stop();
    });
    $loop->run();

    expect($resolved)->toBe(10);
})->group('feature', 'async', 'react')
    ->skip(! class_exists(Loop::class), 'react/event-loop not installed');

it('rejects the promise on a failed query', function () {
    $session = scyllaDbConnection();
    $loop    = Loop::get();
    $error   = null;

    ReactPhp::toPromise($session->executeAsync(new SimpleStatement('SELECT * FROM nope_ks_zz.nope')), $loop)
        ->then(
            fn () => null,
            function (\Throwable $e) use (&$error, $loop) {
                $error = $e;
                $loop->stop();
            },
        );
    $loop->run();

    expect($error)->toBeInstanceOf(\Throwable::class);
})->group('feature', 'async', 'react')
    ->skip(! class_exists(Loop::class), 'react/event-loop not installed');
