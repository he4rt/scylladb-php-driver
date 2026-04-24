<?php

declare(strict_types=1);

use Cassandra\Duration;

$keyspace = 'duration_test';
$table    = 'durations';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE $table (alias text PRIMARY KEY, duration_at duration)
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('stores and retrieves duration values correctly', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $durations = [
        'two_days'                                => new Duration(2, 0, 0),
        'twelve_hours'                            => new Duration(0, 12, 0),
        'three_seconds'                           => new Duration(0, 0, 3 * 1_000_000_000),
        'two_days_twelve_hours_and_three_seconds' => new Duration(2, 12, 3 * 1_000_000_000),
    ];

    foreach ($durations as $alias => $duration) {
        $session->execute(
            "INSERT INTO $table (alias, duration_at) VALUES (?, ?)",
            ['arguments' => [$alias, $duration]]
        );
    }

    $expected = [
        'twelve_hours'                            => '0mo12d0ns',
        'three_seconds'                           => '0mo0d3000000000ns',
        'two_days_twelve_hours_and_three_seconds' => '2mo12d3000000000ns',
        'two_days'                                => '2mo0d0ns',
    ];

    $rows = $session->execute("SELECT * FROM $table");
    foreach ($rows as $row) {
        expect((string) $row['duration_at'])->toBe($expected[$row['alias']]);
    }
})->group('feature');
