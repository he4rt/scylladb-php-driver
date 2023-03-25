<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Duration;

use DateTime;
use Cassandra\Map;
use Cassandra\Set;
use Cassandra\Inet;
use Cassandra\Uuid;
use DateTimeInterface;
use Cassandra\Timestamp;
use Cassandra\Collection;
use Cassandra\SimpleStatement;
use Cassandra\Tests\Feature\Utils;

$collections = 'collections_testing';
$nestedCollections = 'nested_collections_testing';

beforeAll(function () use ($collections, $nestedCollections) {
    Utils::migrateKeyspace(
        <<<CQL
    CREATE KEYSPACE $collections WITH replication = {
        'class': 'SimpleStrategy',
        'replication_factor': 1
    };
    USE $collections;
    CREATE TABLE user (
        id int PRIMARY KEY,
        logins list<timestamp>,
        locations map<timestamp, double>,
        ip_addresses set<inet>
    );
    INSERT INTO user (id, logins, locations, ip_addresses) VALUES (
        0,
        ['2014-09-11 10:09:08+0000', '2014-09-12 10:09:00+0000'],
        {'2014-09-11 10:09:08+0000': 37.397357},
        {'200.199.198.197', '192.168.1.15'}
    );
    CQL
    );

    Utils::migrateKeyspace(
        <<<CQL
        CREATE KEYSPACE $nestedCollections WITH replication = {
            'class': 'SimpleStrategy',
            'replication_factor': 1
        };
        USE $nestedCollections;
        CREATE TABLE users (
            id uuid PRIMARY KEY,
            name text,
            addresses map<text, frozen<map<text, text>>>
        );
    CQL
    );
});

afterAll(function () use ($collections, $nestedCollections) {
    Utils::dropKeyspace($collections);
    Utils::dropKeyspace($nestedCollections);
});

test('Using Cassandra collections', function () use ($collections) {
    $session = Utils::scyllaDbConnection($collections);

    $statement = new SimpleStatement('SELECT * FROM user');

    $result = $session->execute($statement, []);
    $row = $result->first();

    /** @var Collection $logins */
    $logins = $row['logins'];
    expect($logins)
        ->toBeInstanceOf(Collection::class)
        ->toHaveCount(2)
        ->and($logins->get(0))
        ->toBeInstanceOf(Timestamp::class)
        ->map(fn(Timestamp $value) => $value->time())
        ->toBe(DateTime::createFromFormat(DateTimeInterface::ISO8601, '2014-09-11T10:09:08+0000')->getTimestamp())
        ->and($logins->get(1))
        ->toBeInstanceOf(Timestamp::class)
        ->map(fn(Timestamp $value) => $value->time())
        ->toBe(DateTime::createFromFormat(DateTimeInterface::ISO8601, '2014-09-12T10:09:00+0000')->getTimestamp());

    /** @var Map */
    $locations = $row['locations'];
    expect($locations)
        ->toBeInstanceOf(Map::class)
        ->toHaveCount(1)
        ->and($locations->keys())
        ->toHaveCount(1)
        ->and($locations->keys()[0])
        ->toBeInstanceOf(Timestamp::class)
        ->map(fn(Timestamp $value) => $value->time())
        ->toBe(DateTime::createFromFormat(DateTimeInterface::ISO8601, '2014-09-11T10:09:08+0000')->getTimestamp())
        ->and($locations->values()[0])
        ->toBe(37.397357);


    /** @var Set $ipAddresses */
    $ipAddresses = $row['ip_addresses'];
    expect($ipAddresses)
        ->toBeInstanceOf(Set::class)
        ->toHaveCount(2)
        ->and($ipAddresses->values())
        ->toHaveCount(2)
        ->and($ipAddresses->values()[0])
        ->toBeInstanceOf(Inet::class)
        ->and($ipAddresses->values()[1])
        ->toBeInstanceOf(Inet::class)
        ->and($ipAddresses->values()[0])
        ->toBeInstanceOf(Inet::class)
        ->and($ipAddresses->values()[1])
        ->toBeInstanceOf(Inet::class)
        ->and((string)$ipAddresses->values()[0]->address())
        ->toBe('192.168.1.15')
        ->and((string)$ipAddresses->values()[1]->address())
        ->toBe('200.199.198.197');
});


it('Using Cassandra nested collections', function () use ($nestedCollections) {
    $session = Utils::scyllaDbConnection($nestedCollections);

    $address = new Map(\Cassandra\Type::text(), \Cassandra\Type::text());
    $addresses = new Map(\Cassandra\Type::text(), $address->type());

    $address->set('city', 'Phoenix');
    $address->set('street', '9042 Cassandra Lane');
    $address->set('zip', '85023');

    $addresses->set('home', $address);

    $users = [
        [
            new \Cassandra\Uuid('56357d2b-4586-433c-ad24-afa9918bc415'),
            'Charles Wallace',
            $addresses,
        ]
    ];

    foreach ($users as $user) {
        $session->execute("INSERT INTO users (id, name, addresses) VALUES (?, ?, ?)", [
            'arguments' => $user,
        ]);
    }

    $result = $session->execute("SELECT * FROM users", []);

    expect($result)
        ->not()
        ->toBeNull()
        ->and($result)
        ->toHaveCount(1);

    $row = $result->first();

    expect($row['id'])
        ->toBeInstanceOf(Uuid::class)
        ->map(fn(Uuid $value) => (string)$value)
        ->toBe('56357d2b-4586-433c-ad24-afa9918bc415')
        ->and($row['name'])
        ->toBe('Charles Wallace');

    /** @var Map $addresses */
    $addresses = $row['addresses'];
    expect($addresses)
        ->toBeInstanceOf(Map::class)
        ->toHaveCount(1)
        ->and($addresses->keys())
        ->toHaveCount(1)
        ->and($addresses->keys()[0])
        ->toBe('home');

    /** @var Map $address */
    $address = $addresses->values()[0];
    expect($address)
        ->toBeInstanceOf(Map::class)
        ->toHaveCount(3)
        ->and($address->keys())
        ->toHaveCount(3)
        ->and($address->keys()[0])
        ->toBe('city')
        ->and($address->keys()[1])
        ->toBe('street')
        ->and($address->keys()[2])
        ->toBe('zip')
        ->and($address->values())
        ->toHaveCount(3)
        ->and($address->values()[0])
        ->toBe('Phoenix')
        ->and($address->values()[1])
        ->toBe('9042 Cassandra Lane')
        ->and($address->values()[2])
        ->toBe('85023');
});
