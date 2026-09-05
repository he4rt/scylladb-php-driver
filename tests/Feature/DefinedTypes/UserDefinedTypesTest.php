<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\DefinedTypes;

use Cassandra\Uuid;

$keyspace = 'user_defined_types';
$table = 'users';

beforeAll(function () use($keyspace, $table) {
    migrateKeyspace(<<<CQL
        CREATE KEYSPACE $keyspace WITH replication = {
            'class': 'SimpleStrategy',
            'replication_factor': 1
        };
        USE $keyspace;
        CREATE TYPE address (street text, city text, zip int);
        CREATE TYPE addresses (home frozen<address>, work frozen<address>);
        CREATE TABLE $table (
            id uuid PRIMARY KEY,
            name text,
            addresses frozen<addresses>
        );
    CQL
    );
});

afterAll(function () use($keyspace) {
    dropKeyspace($keyspace);
});

it('Using Cassandra user defined types from schema metadata', function () use($keyspace, $table) {

    if (isScyllaRustBackend()) {
        $this->markTestSkipped('Schema introspection (keyspace meta) is intentionally unsupported by cpp-rs-driver');
    }

    $session = scyllaDbConnection($keyspace);

    $keyspace = $session->schema()->keyspace('user_defined_types');
    $addressType = $keyspace->userType("address");
    $addressesType = $keyspace->userType("addresses");

    $users = [
        [
            new Uuid('56357d2b-4586-433c-ad24-afa9918bc415'),
            'Arthur Canhassi',
            $addressesType->create(
                'home', $addressType->create(
                'city', 'Phoenix',
                'street', '9042 Cassandra Lane',
                'zip', 85023))
        ],
        [
            new Uuid('ce359590-8528-4682-a9f3-add53fc9aa09'),
            'Kevin Malone',
            $addressesType->create(
                'home', $addressType->create(
                'city', 'New York',
                'street', '1000 Database Road',
                'zip', 10025),
                'work', $addressType->create(
                'city', 'New York',
                'street', '60 SSTable Drive',
                'zip', 10024)
            )
        ],
    ];

    foreach ($users as $user) {
        $options = array('arguments' => $user);
        $session->execute("INSERT INTO users (id, name, addresses) VALUES (?, ?, ?)", $options);
    }

    $result = $session->execute("SELECT * FROM $table");

    $row = $result->first();
    expect($row['id'])
        ->toBeInstanceOf(Uuid::class)
        ->and((string) $row['id'])
        ->toBe('56357d2b-4586-433c-ad24-afa9918bc415')
        ->and($row['name'])
        ->toBe('Arthur Canhassi');

    $addresses = $row['addresses'];
    expect($addresses)
        ->toHaveCount(2)
        ->and($addresses->values())
        ->toHaveCount(2)
        ->and($addresses->values()['home'])
        ->toHaveCount(3)
        ->and($addresses->values()['work'])
        ->toBeNull();

    $address = $addresses->values()['home']->values();
    expect($address)
        ->toHaveCount(3)
        ->and($address['street'])
        ->toBe('9042 Cassandra Lane')
        ->and($address['city'])
        ->toBe('Phoenix')
        ->and($address['zip'])
        ->toBe(85023);

    $row = $result->offsetGet(1);
    expect($row['id'])
        ->toBeInstanceOf(Uuid::class)
        ->and((string) $row['id'])
        ->toBe('ce359590-8528-4682-a9f3-add53fc9aa09')
        ->and($row['name'])
        ->toBe('Kevin Malone');

    $addresses = $row['addresses'];
    expect($addresses)
        ->toHaveCount(2)
        ->and($addresses->values())
        ->toHaveCount(2)
        ->and($addresses->values()['home'])
        ->toHaveCount(3)
        ->and($addresses->values()['work'])
        ->toHaveCount(3);

    $address = $addresses->values()['home']->values();
    expect($address)
        ->toHaveCount(3)
        ->and($address['street'])
        ->toBe('1000 Database Road')
        ->and($address['city'])
        ->toBe('New York')
        ->and($address['zip'])
        ->toBe(10025);

    $workAddress = $addresses->values()['work']->values();
    expect($workAddress)
        ->toHaveCount(3)
        ->and($workAddress['street'])
        ->toBe('60 SSTable Drive')
        ->and($workAddress['city'])
        ->toBe('New York')
        ->and($workAddress['zip'])
        ->toBe(10024);
});

it('binds a hand-built user type that names its keyspace and type', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $addressType = \Cassandra\Type::userType(
        'street', \Cassandra\Type::text(),
        'city', \Cassandra\Type::text(),
        'zip', \Cassandra\Type::int(),
    )->withName('address')->withKeyspace($keyspace);

    $addressesType = \Cassandra\Type::userType(
        'home', $addressType,
        'work', $addressType,
    )->withName('addresses')->withKeyspace($keyspace);

    expect($addressType->name())->toBe('address')
        ->and($addressType->keyspace())->toBe($keyspace);

    $id = new Uuid('7f8f9c8e-1f2a-4f1b-9c3d-0a1b2c3d4e5f');

    $prepared = $session->prepare("INSERT INTO $table (id, name, addresses) VALUES (?, ?, ?)");

    $session->execute(
        $prepared,
        ['arguments' => [
            $id,
            'Named UDT',
            $addressesType->create(
                'home', $addressType->create('street', '1 Main St', 'city', 'Springfield', 'zip', 11111),
                'work', $addressType->create('street', '2 Side St', 'city', 'Shelbyville', 'zip', 22222),
            ),
        ]],
    );

    $row = $session->execute("SELECT * FROM $table WHERE id = ?", ['arguments' => [$id]])->first();

    expect($row['name'])->toBe('Named UDT')
        ->and($row['addresses']->values()['home']->values()['city'])->toBe('Springfield')
        ->and($row['addresses']->values()['work']->values()['zip'])->toBe(22222);
});
