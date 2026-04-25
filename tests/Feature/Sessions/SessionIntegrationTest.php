<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Sessions;

use Cassandra;
use Cassandra\Exception\RuntimeException;

it('does not crash when connecting asynchronously to a non-existent host', function () {
    // The original test stopped a CCM-managed cluster, attempted async connect,
    // and asserted a RuntimeException. We approximate by pointing the cluster
    // at an unroutable address — there is no CCM equivalent in the Pest setup.
    $cluster = Cassandra::cluster()
        ->withContactPoints('127.0.0.99')
        ->withPort(9042)
        ->withConnectTimeout(1)
        ->withPersistentSessions(false)
        ->build();

    $future = $cluster->connectAsync();

    expect(fn () => $future->get())->toThrow(RuntimeException::class);
})->skip('Requires CCM (cluster start/stop) — Pest helpers do not yet provide a node-lifecycle fixture. Re-enable once a CCM-equivalent fixture exists.');
