<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Sessions;

use Cassandra;
use Cassandra\Exception\RuntimeException;
use Cassandra\Uuid;

/*
 * Migrated from tests-old/features/sessions/session_management.feature
 */

$keyspace = 'session_mgmt_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    CREATE TABLE $keyspace.playlists (
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

it('throws when the same session is closed twice', function () {
    $cluster = Cassandra::cluster()
        ->withContactPoints(...explode(',', (string) env('SCYLLADB_HOSTS', '127.0.0.1')))
        ->withPort((int) env('SCYLLADB_PORT', 9042))
        ->withCredentials(env('SCYLLADB_USERNAME', 'cassandra'), env('SCYLLADB_PASSWORD', 'cassandra'))
        ->withPersistentSessions(false)
        ->build();

    $session = $cluster->connect();
    $session->close();

    expect(fn () => $session->close())
        ->toThrow(RuntimeException::class, 'Already closing or closed');
});

it('completes outstanding async requests after close() is called', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $insert = "INSERT INTO $keyspace.playlists (id, song_id, artist, title, album) VALUES (?, ?, ?, ?, ?)";
    $rows = [
        [new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'), new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'), 'Joséphine Baker', 'La Petite Tonkinoise', 'Bye Bye Blackbird'],
        [new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'), new Uuid('f6071e72-48ec-4fcb-bf3e-379c8a696488'), 'Willi Ostermann', 'Die Mösch', 'In Gold'],
        [new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'), new Uuid('fbdf82ed-0063-4796-9c7c-a3d4f47b4b25'), 'Mick Jager', 'Memo From Turner', 'Performance'],
    ];
    foreach ($rows as $r) {
        $session->execute($insert, ['arguments' => $r]);
    }

    $future = $session->executeAsync("SELECT * FROM $keyspace.playlists");
    $session->close();

    expect($future->get()->count())->toBe(3);
});
