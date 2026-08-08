<?php

declare(strict_types=1);

use Amp\Future;
use Cassandra\Async\Amp;
use Cassandra\SimpleStatement;

use function Amp\async;

it('awaits a Cassandra future as an Amp future', function () {
    $version = async(function () {
        $session = scyllaDbConnection();
        $rows    = Amp::toFuture(
            $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')),
        )->await();

        return $rows->first()['release_version'] ?? null;
    })->await();

    expect($version)->not->toBeNull();
})->group('feature', 'async', 'amp')
    ->skip(! class_exists(Future::class), 'amphp/amp not installed');

it('awaits many futures concurrently via Amp\\Future\\await', function () {
    $resolved = async(function () {
        $session = scyllaDbConnection();

        $futures = [];
        for ($i = 0; $i < 10; $i++) {
            $futures[] = Amp::toFuture(
                $session->executeAsync(new SimpleStatement('SELECT release_version FROM system.local')),
            );
        }

        return count(Future\await($futures));
    })->await();

    expect($resolved)->toBe(10);
})->group('feature', 'async', 'amp')
    ->skip(! class_exists(Future::class), 'amphp/amp not installed');

it('errors the Amp future on a failed query', function () {
    $error = async(function () {
        $session = scyllaDbConnection();
        try {
            Amp::toFuture(
                $session->executeAsync(new SimpleStatement('SELECT * FROM nope_ks_zz.nope')),
            )->await();
        } catch (\Throwable $e) {
            return $e;
        }

        return null;
    })->await();

    expect($error)->toBeInstanceOf(\Throwable::class);
})->group('feature', 'async', 'amp')
    ->skip(! class_exists(Future::class), 'amphp/amp not installed');
