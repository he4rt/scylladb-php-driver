<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Collections;

use Cassandra\Set;
use Cassandra\Type;

require_once __DIR__ . '/_helpers.php';

$keyspace = 'set_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('creates a set with scalar types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    // Duration cannot be a set element.
    $scalarTypes = array_filter(
        scalarCassandraTypes(),
        fn ($t) => (string) $t[0] !== 'duration'
    );

    $rows = [];
    foreach ($scalarTypes as $cassandraType) {
        $setType = Type::set($cassandraType[0]);
        $set = $setType->create();
        foreach ($cassandraType[1] as $value) {
            $set->add($value);
        }
        $rows[(string) $setType] = [$setType, $set];
    }
    foreach (constantScalarCassandraTypes() as $cassandraType) {
        $set = new Set($cassandraType[0]);
        foreach ($cassandraType[1] as $value) {
            $set->add($value);
        }
        $rows['const_' . (string) $set->type()] = [$set->type(), $set];
    }
    return $rows;
});

it('creates a set with composite types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsEnsureUserTypes($keyspace, compositeCassandraTypes());
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];
    foreach (compositeCassandraTypes() as $cassandraType) {
        $setType = Type::set($cassandraType[0]);
        $set = $setType->create();
        foreach ($cassandraType[1] as $value) {
            $set->add($value);
        }
        $rows[(string) $setType] = [$setType, $set];
    }
    return $rows;
});

it('creates a set with nested composite types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsEnsureUserTypes($keyspace, compositeCassandraTypes());
    collectionsEnsureUserTypes($keyspace, nestedCassandraTypes());
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];
    foreach (nestedCassandraTypes() as $cassandraType) {
        $setType = Type::set($cassandraType[0]);
        $set = $setType->create();
        foreach ($cassandraType[1] as $value) {
            $set->add($value);
        }
        $rows[(string) $setType] = [$setType, $set];
    }
    return $rows;
});

it('binds an empty set', function () use ($keyspace) {
    $setType = Type::set(Type::int());
    collectionsCreateAndVerify($keyspace, $setType, $setType->create());
});

it('binds a null set', function () use ($keyspace) {
    $setType = Type::set(Type::int());
    collectionsCreateAndVerify($keyspace, $setType, null);
});
