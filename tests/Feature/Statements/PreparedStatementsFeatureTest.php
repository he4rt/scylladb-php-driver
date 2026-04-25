<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

use Cassandra;
use Cassandra\Uuid;

$keyspace = 'prepared_feat_' . substr(bin2hex(random_bytes(4)), 0, 8);
$table    = 'playlists';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
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

it('supports prepared statements with named arguments', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $insert  = $session->prepare(
        "INSERT INTO $table (id, song_id, artist, title, album) " .
        'VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)'
    );

    $songs = [
        [
            'song_id' => new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'),
            'title'   => 'La Petite Tonkinoise',
            'album'   => 'Bye Bye Blackbird',
            'artist'  => 'Joséphine Baker',
        ],
        [
            'song_id' => new Uuid('f6071e72-48ec-4fcb-bf3e-379c8a696488'),
            'title'   => 'Die Mösch',
            'album'   => 'In Gold',
            'artist'  => 'Willi Ostermann',
        ],
        [
            'song_id' => new Uuid('fbdf82ed-0063-4796-9c7c-a3d4f47b4b25'),
            'title'   => 'Memo From Turner',
            'album'   => 'Performance',
            'artist'  => 'Mick Jager',
        ],
    ];

    foreach ($songs as $song) {
        $session->execute($insert, ['arguments' => $song]);
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    $seen = [];
    foreach ($rows as $row) {
        $seen[] = $row['artist'] . ': ' . $row['title'] . ' / ' . $row['album'];
    }

    expect($seen)->toContain('Joséphine Baker: La Petite Tonkinoise / Bye Bye Blackbird')
        ->and($seen)->toContain('Willi Ostermann: Die Mösch / In Gold')
        ->and($seen)->toContain('Mick Jager: Memo From Turner / Performance');
});
