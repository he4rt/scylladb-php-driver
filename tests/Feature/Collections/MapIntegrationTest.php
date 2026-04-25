<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Collections;

use Cassandra\Map;
use Cassandra\Type;

require_once __DIR__ . '/_helpers.php';

$keyspace = 'map_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('creates a map with scalar types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    // Duration cannot be a map key.
    $scalarKeyTypes = array_filter(
        scalarCassandraTypes(),
        fn ($t) => (string) $t[0] !== 'duration'
    );

    $rows = [];

    foreach ($scalarKeyTypes as $cassandraType) {
        $mapType = Type::map($cassandraType[0], Type::int());
        $map = $mapType->create();
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($values[$i], $i);
        }
        $rows['key_' . (string) $cassandraType[0]] = [$mapType, $map];
    }

    foreach (scalarCassandraTypes() as $cassandraType) {
        $mapType = Type::map(Type::int(), $cassandraType[0]);
        $map = $mapType->create();
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($i, $values[$i]);
        }
        $rows['value_' . (string) $cassandraType[0]] = [$mapType, $map];
    }

    foreach (constantScalarCassandraTypes() as $cassandraType) {
        $map = new Map($cassandraType[0], $cassandraType[0]);
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($values[$i], $values[$i]);
        }
        $rows['const_' . (string) $map->type()] = [$map->type(), $map];
    }

    return $rows;
});

it('creates a map with composite types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsEnsureUserTypes($keyspace, compositeCassandraTypes());
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];

    foreach (compositeCassandraTypes() as $cassandraType) {
        $mapType = Type::map($cassandraType[0], Type::int());
        $map = $mapType->create();
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($values[$i], $i);
        }
        $rows['key_' . (string) $cassandraType[0]] = [$mapType, $map];
    }

    foreach (compositeCassandraTypes() as $cassandraType) {
        $mapType = Type::map(Type::int(), $cassandraType[0]);
        $map = $mapType->create();
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($i, $values[$i]);
        }
        $rows['value_' . (string) $cassandraType[0]] = [$mapType, $map];
    }

    return $rows;
});

it('creates a map with nested composite types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsEnsureUserTypes($keyspace, compositeCassandraTypes());
    collectionsEnsureUserTypes($keyspace, nestedCassandraTypes());
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];

    foreach (nestedCassandraTypes() as $cassandraType) {
        $mapType = Type::map($cassandraType[0], Type::int());
        $map = $mapType->create();
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($values[$i], $i);
        }
        $rows['key_' . (string) $cassandraType[0]] = [$mapType, $map];
    }

    foreach (nestedCassandraTypes() as $cassandraType) {
        $mapType = Type::map(Type::int(), $cassandraType[0]);
        $map = $mapType->create();
        $values = $cassandraType[1];
        for ($i = 0; $i < count($values); $i++) {
            $map->set($i, $values[$i]);
        }
        $rows['value_' . (string) $cassandraType[0]] = [$mapType, $map];
    }

    return $rows;
});

it('binds an empty map', function () use ($keyspace) {
    $mapType = Type::map(Type::int(), Type::int());
    collectionsCreateAndVerify($keyspace, $mapType, $mapType->create());
});

it('binds a null map', function () use ($keyspace) {
    $mapType = Type::map(Type::int(), Type::int());
    collectionsCreateAndVerify($keyspace, $mapType, null);
});
