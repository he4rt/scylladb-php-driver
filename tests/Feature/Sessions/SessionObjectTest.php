<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Sessions;

use Cassandra\Uuid;

/*
 * Migrated from tests-old/features/sessions/session_object.feature
 *
 * Single scenario: one cluster opens three sessions, each bound to a different
 * keyspace, and confirms each can read its own playlists table independently.
 */

$suffix = substr(bin2hex(random_bytes(4)), 0, 8);
$french = "session_obj_french_$suffix";
$german = "session_obj_german_$suffix";
$uk     = "session_obj_uk_$suffix";

beforeAll(function () use ($french, $german, $uk) {
    foreach ([$french, $german, $uk] as $ks) {
        migrateKeyspace(<<<CQL
        CREATE KEYSPACE $ks WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
        CREATE TABLE $ks.playlists (
            id uuid,
            title text,
            album text,
            artist text,
            song_id uuid,
            PRIMARY KEY (id, title, album, artist)
        )
        CQL);
    }

    $session = scyllaDbConnection();
    $session->execute(
        "INSERT INTO $french.playlists (id, song_id, artist, title, album) VALUES (?, ?, ?, ?, ?)",
        ['arguments' => [new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'), new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'), 'Joséphine Baker', 'La Petite Tonkinoise', 'Bye Bye Blackbird']]
    );
    $session->execute(
        "INSERT INTO $german.playlists (id, song_id, artist, title, album) VALUES (?, ?, ?, ?, ?)",
        ['arguments' => [new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'), new Uuid('f6071e72-48ec-4fcb-bf3e-379c8a696488'), 'Willi Ostermann', 'Die Mösch', 'In Gold']]
    );
    $session->execute(
        "INSERT INTO $uk.playlists (id, song_id, artist, title, album) VALUES (?, ?, ?, ?, ?)",
        ['arguments' => [new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'), new Uuid('fbdf82ed-0063-4796-9c7c-a3d4f47b4b25'), 'Mick Jager', 'Memo From Turner', 'Performance']]
    );
});

afterAll(function () use ($french, $german, $uk) {
    foreach ([$french, $german, $uk] as $ks) {
        dropKeyspace($ks);
    }
});

it('lets one cluster operate on multiple keyspaces via separate sessions', function () use ($french, $german, $uk) {
    $frenchSession = scyllaDbConnection($french);
    $germanSession = scyllaDbConnection($german);
    $ukSession     = scyllaDbConnection($uk);

    expect($frenchSession->execute('SELECT * FROM playlists')->count())->toBe(1)
        ->and($germanSession->execute('SELECT * FROM playlists')->count())->toBe(1)
        ->and($ukSession->execute('SELECT * FROM playlists')->count())->toBe(1);
});
