<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Tuples;

use Cassandra\Uuid;
use Cassandra;

$keyspace = 'tuple';
$table = 'users';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {
        'class': 'SimpleStrategy',
        'replication_factor': 1
      };
      USE $keyspace;
      CREATE TABLE $table (
        id uuid PRIMARY KEY,
        name text,
        address tuple<text, text, int>
      );
    CQL
    );
});

afterAll(function () use ($keyspace) {
    dropKeyspace($keyspace);
});

test('Using Cassandra tuples', function () use($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $tupleType = Cassandra\Type::tuple(Cassandra\Type::text(), Cassandra\Type::text(), Cassandra\Type::int());

    $users = [
        [
            new Uuid('56357d2b-4586-433c-ad24-afa9918bc415'),
            'Arthur Canhassi',
            $tupleType->create('Phoenix', '9042 Cassandra Lane', 85023)
        ],
        [
            new Uuid('ce359590-8528-4682-a9f3-add53fc9aa09'),
            'Kevin Malone',
            $tupleType->create('New York', '1000 Database Road', 10025)
        ],
        [
            new Uuid('7d64dca1-dd4d-4f3c-bec4-6a88fa082a13'),
            'Michael Scott',
            $tupleType->create('Santa Clara', '20000 Log Ave', 95054)
        ]
    ];

    foreach ($users as $user) {
        $options = ['arguments' => $user];
        $session->execute("INSERT INTO users (id, name, address) VALUES (?, ?, ?)", $options);
    }

    $result = $session->execute("SELECT * FROM $table");

    $row = $result->first();

    expect($row['id'])
        ->toBeInstanceOf(Uuid::class)
        ->and((string) $row['id'])
        ->toBe('56357d2b-4586-433c-ad24-afa9918bc415');

    $address = $row['address'];
    expect($address)
        ->toHaveCount(3)
        ->and($address->values()[0])
        ->toBe('Phoenix')
        ->and($address->values()[1])
        ->toBe('9042 Cassandra Lane')
        ->and($address->values()[2])
        ->toBe(85023)
        ->and($row['name'])
        ->toBe('Arthur Canhassi');

    $row = $result->offsetGet(1);
    expect($row['id'])
        ->toBeInstanceOf(Uuid::class)
        ->and((string) $row['id'])
        ->toBe('ce359590-8528-4682-a9f3-add53fc9aa09');

    $address = $row['address'];
    expect($address)
        ->toHaveCount(3)
        ->and($address->values()[0])
        ->toBe('New York')
        ->and($address->values()[1])
        ->toBe('1000 Database Road')
        ->and($address->values()[2])
        ->toBe(10025)
        ->and($row['name'])
        ->toBe('Kevin Malone');

    $row = $result->offsetGet(2);
    expect($row['id'])
        ->toBeInstanceOf(Uuid::class)
        ->and((string) $row['id'])
        ->toBe('7d64dca1-dd4d-4f3c-bec4-6a88fa082a13');

    $address = $row['address'];
    expect($address)
        ->toHaveCount(3)
        ->and($address->values()[0])
        ->toBe('Santa Clara')
        ->and($address->values()[1])
        ->toBe('20000 Log Ave')
        ->and($address->values()[2])
        ->toBe(95054)
        ->and($row['name'])
        ->toBe('Michael Scott');
});
