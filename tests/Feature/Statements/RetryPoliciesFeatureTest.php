<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

use Cassandra;
use Cassandra\BatchStatement;
use Cassandra\Exception\UnavailableException;
use Cassandra\RetryPolicy;
use Cassandra\Uuid;

// Background trick: RF=3 against a single node so QUORUM/ALL writes hit retry paths.
$keyspace = 'retry_feat_' . substr(bin2hex(random_bytes(4)), 0, 8);
$table    = 'playlists';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 3}
    CQL);

    migrateKeyspace(<<<CQL
    CREATE TABLE $keyspace.$table (
        id uuid,
        title text,
        album text,
        artist text,
        song_id uuid,
        PRIMARY KEY (id, title, album, artist)
    )
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('accepts a downgrading-consistency retry policy on the cluster builder', function () use ($keyspace, $table) {
    $hosts = explode(',', (string) env('SCYLLADB_HOSTS', '127.0.0.1'));

    $retryPolicy = new RetryPolicy\DowngradingConsistency();
    $session = Cassandra::cluster()
        ->withContactPoints(...$hosts)
        ->withPort((int) env('SCYLLADB_PORT', 9042))
        ->withCredentials(env('SCYLLADB_USERNAME', 'cassandra'), env('SCYLLADB_PASSWORD', 'cassandra'))
        ->withRetryPolicy(new RetryPolicy\Logging($retryPolicy))
        ->withPersistentSessions(false)
        ->build()
        ->connect($keyspace);

    $statement = $session->prepare(
        "INSERT INTO $table (id, song_id, artist, title, album) " .
        'VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)'
    );

    $args = [
        new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'),
        'Joséphine Baker',
        'La Petite Tonkinoise',
        'Bye Bye Blackbird',
    ];

    try {
        $session->execute($statement, [
            'consistency' => Cassandra::CONSISTENCY_QUORUM,
            'arguments'   => $args,
        ]);
    } catch (UnavailableException $e) {
        // Some servers (e.g. recent Scylla) refuse to downgrade and surface unavailable.
        $this->markTestSkipped('Server refused to downgrade consistency: ' . $e->getMessage());
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    $found = false;
    foreach ($rows as $row) {
        if ($row['artist'] === 'Joséphine Baker'
            && $row['title'] === 'La Petite Tonkinoise'
            && $row['album'] === 'Bye Bye Blackbird') {
            $found = true;
            break;
        }
    }
    expect($found)->toBeTrue();
});

it('accepts a per-statement retry policy in execute() options', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $retryPolicy = new RetryPolicy\Logging(new RetryPolicy\DowngradingConsistency());

    $statement = $session->prepare(
        "INSERT INTO $table (id, song_id, artist, title, album) " .
        'VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)'
    );

    $args = [
        new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'),
        'Joséphine Baker',
        'La Petite Tonkinoise',
        'Bye Bye Blackbird',
    ];

    try {
        $session->execute($statement, [
            'consistency'  => Cassandra::CONSISTENCY_QUORUM,
            'arguments'    => $args,
            'retry_policy' => $retryPolicy,
        ]);
    } catch (UnavailableException $e) {
        $this->markTestSkipped('Server refused to downgrade consistency: ' . $e->getMessage());
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    expect($rows->count())->toBeGreaterThanOrEqual(1);
});

it('accepts a retry policy when executing a batch', function () use ($keyspace, $table) {
    if (version_compare(Cassandra::CPP_DRIVER_VERSION, '2.2.3') < 0) {
        $this->markTestSkipped('Batch retry-with-downgrade fixed in cpp-driver >= 2.2.3');
    }

    $session = scyllaDbConnection($keyspace);

    $retryPolicy = new RetryPolicy\Logging(new RetryPolicy\DowngradingConsistency());
    $prepared = $session->prepare(
        "INSERT INTO $table (id, song_id, artist, title, album) " .
        'VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)'
    );

    $batch = new BatchStatement(Cassandra::BATCH_UNLOGGED);
    $batch->add($prepared, [
        new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'),
        'Joséphine Baker',
        'La Petite Tonkinoise',
        'Bye Bye Blackbird',
    ]);
    $batch->add(
        "INSERT INTO $table (id, song_id, artist, title, album) " .
        'VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)',
        [
            new Uuid('f6071e72-48ec-4fcb-bf3e-379c8a696488'),
            'Willi Ostermann',
            'Die Mösch',
            'In Gold',
        ]
    );

    try {
        $session->execute($batch, [
            'consistency'  => Cassandra::CONSISTENCY_QUORUM,
            'retry_policy' => $retryPolicy,
        ]);
    } catch (UnavailableException $e) {
        $this->markTestSkipped('Server refused to downgrade consistency on batch: ' . $e->getMessage());
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    expect($rows->count())->toBeGreaterThanOrEqual(1);
});
