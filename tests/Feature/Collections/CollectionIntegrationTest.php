<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Collections;

use Cassandra\Collection;
use Cassandra\Type;

require_once __DIR__ . '/_helpers.php';

$keyspace = 'collection_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('creates a list with scalar types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];
    foreach (scalarCassandraTypes() as $cassandraType) {
        $listType = Type::collection($cassandraType[0]);
        $list = $listType->create();
        foreach ($cassandraType[1] as $value) {
            $list->add($value);
        }
        $rows[(string) $listType] = [$listType, $list];
    }
    foreach (constantScalarCassandraTypes() as $cassandraType) {
        $list = new Collection($cassandraType[0]);
        foreach ($cassandraType[1] as $value) {
            $list->add($value);
        }
        $rows['const_' . (string) $list->type()] = [$list->type(), $list];
    }
    return $rows;
});

it('creates a list with composite types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsEnsureUserTypes($keyspace, compositeCassandraTypes());
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];
    foreach (compositeCassandraTypes() as $cassandraType) {
        $listType = Type::collection($cassandraType[0]);
        $list = $listType->create();
        foreach ($cassandraType[1] as $value) {
            $list->add($value);
        }
        $rows[(string) $listType] = [$listType, $list];
    }
    return $rows;
});

it('creates a list with nested composite types and verifies by index and by name', function ($type, $value) use ($keyspace) {
    collectionsEnsureUserTypes($keyspace, compositeCassandraTypes());
    collectionsEnsureUserTypes($keyspace, nestedCassandraTypes());
    collectionsCreateAndVerify($keyspace, $type, $value);
})->with(function () {
    $rows = [];
    foreach (nestedCassandraTypes() as $cassandraType) {
        $listType = Type::collection($cassandraType[0]);
        $list = $listType->create();
        foreach ($cassandraType[1] as $value) {
            $list->add($value);
        }
        $rows[(string) $listType] = [$listType, $list];
    }
    return $rows;
});

it('binds an empty list', function () use ($keyspace) {
    $listType = Type::collection(Type::int());
    collectionsCreateAndVerify($keyspace, $listType, $listType->create());
});

it('binds a null list', function () use ($keyspace) {
    $listType = Type::collection(Type::int());
    collectionsCreateAndVerify($keyspace, $listType, null);
});
