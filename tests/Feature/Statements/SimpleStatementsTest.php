<?php

declare(strict_types=1);

use Cassandra\SimpleStatement;
use Cassandra\Uuid;

$keyspace = 'simple_statements';
$table    = 'playlists';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE $table (
        id       uuid,
        title    text,
        album    text,
        artist   text,
        song_id  uuid,
        PRIMARY KEY (id, title, album, artist)
    )
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('executes a SELECT and returns an empty result on a fresh table', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $result  = $session->execute(new SimpleStatement("SELECT * FROM $table"));

    expect($result->count())->toBe(0);
})->group('feature');

it('supports positional ? arguments', function () use ($keyspace, $table) {
    $session   = scyllaDbConnection($keyspace);
    $songs     = [
        [new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'), 'Joséphine Baker', 'La Petite Tonkinoise', 'Bye Bye Blackbird'],
        [new Uuid('f6071e72-48ec-4fcb-bf3e-379c8a696488'), 'Willi Ostermann', 'Die Mösch', 'In Gold'],
        [new Uuid('fbdf82ed-0063-4796-9c7c-a3d4f47b4b25'), 'Mick Jager', 'Memo From Turner', 'Performance'],
    ];
    $statement = new SimpleStatement(
        "INSERT INTO $table (id, song_id, artist, title, album) VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)"
    );

    foreach ($songs as $song) {
        $session->execute($statement, ['arguments' => $song]);
    }

    expect($session->execute("SELECT * FROM $keyspace.$table")->count())->toBe(3);
})->group('feature');

it('supports named arguments', function () use ($keyspace, $table) {
    $session   = scyllaDbConnection($keyspace);
    $statement = new SimpleStatement(
        "INSERT INTO $table (id, song_id, artist, title, album) VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)"
    );

    $session->execute($statement, ['arguments' => [
        'song_id' => new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'),
        'artist'  => 'Joséphine Baker',
        'title'   => 'La Petite Tonkinoise',
        'album'   => 'Bye Bye Blackbird',
    ]]);

    $row = $session->execute(
        "SELECT * FROM $keyspace.$table WHERE id = 62c36092-82a1-3a00-93d1-46196ee77204"
    )->first();
    expect($row['artist'])->toBe('Joséphine Baker');
})->group('feature');

it('supports :name placeholder arguments', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $session->execute(
        "INSERT INTO $table (id, song_id, artist, title, album) VALUES (62c36092-82a1-3a00-93d1-46196ee77204, ?, ?, ?, ?)",
        ['arguments' => [
            'song_id' => new Uuid('756716f7-2e54-4715-9f00-91dcbea6cf50'),
            'artist'  => 'Joséphine Baker',
            'title'   => 'La Petite Tonkinoise',
            'album'   => 'Bye Bye Blackbird',
        ]]
    );

    $result = $session->execute(
        new SimpleStatement("SELECT * FROM $keyspace.$table WHERE id = :id AND artist = :artist AND title = :title AND album = :album"),
        ['arguments' => [
            'id'     => new Uuid('62c36092-82a1-3a00-93d1-46196ee77204'),
            'artist' => 'Joséphine Baker',
            'title'  => 'La Petite Tonkinoise',
            'album'  => 'Bye Bye Blackbird',
        ]]
    );

    $row = $result->first();
    expect("$row[artist]: $row[title] / $row[album]")
        ->toBe('Joséphine Baker: La Petite Tonkinoise / Bye Bye Blackbird');
})->group('feature');
