<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Tuples;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Exception\InvalidQueryException;
use Cassandra\Tuple;
use Cassandra\Type;
use Cassandra\Uuid;

$keyspace = 'tuple_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

/**
 * Helper: create a unique table for the given CQL value type, insert the given
 * value as both positional and named bound parameter, and assert the round-trip.
 */
function tupleCreateInsertVerify(string $keyspace, string $cqlValueType, mixed $value): void
{
    $session = scyllaDbConnection($keyspace);
    $table = 'tbl_' . str_replace('-', '', (string) (new Uuid()));

    $session->execute(
        "CREATE TABLE $keyspace.$table (key text PRIMARY KEY, value $cqlValueType)"
    );

    // Positional
    $session->execute(
        "INSERT INTO $keyspace.$table (key, value) VALUES (?, ?)",
        ['arguments' => ['k1', $value]]
    );

    // Named
    $session->execute(
        "INSERT INTO $keyspace.$table (key, value) VALUES (?, ?)",
        ['arguments' => ['key' => 'k2', 'value' => $value]]
    );

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    expect($rows->count())->toBe(2);
}

it('Round-trips a tuple<int> with a single scalar', function () use ($keyspace) {
    $tupleType = Type::tuple(Type::int());
    $tuple = $tupleType->create();
    $tuple->set(0, 42);

    tupleCreateInsertVerify($keyspace, 'frozen<tuple<int>>', $tuple);
});

it('Round-trips a tuple with multiple scalar components', function () use ($keyspace) {
    $tupleType = Type::tuple(Type::int(), Type::varchar(), Type::bigint());
    $tuple = $tupleType->create();
    $tuple->set(0, 99);
    $tuple->set(1, 'hello');
    $tuple->set(2, new Bigint('123456789012'));

    tupleCreateInsertVerify($keyspace, 'frozen<tuple<int, varchar, bigint>>', $tuple);
});

it('Stores a null tuple value', function () use ($keyspace) {
    tupleCreateInsertVerify($keyspace, 'frozen<tuple<int>>', null);
});

it('Round-trips a partial tuple (some components unset)', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);
    $table = 'partial_' . substr(bin2hex(random_bytes(4)), 0, 8);
    $session->execute(
        "CREATE TABLE $keyspace.$table (key text PRIMARY KEY, value frozen<tuple<int, varchar, bigint>>)"
    );

    $type = Type::tuple(Type::int(), Type::varchar(), Type::bigint());

    $a = $type->create();
    $a->set(0, 99);
    $b = $type->create();
    $b->set(1, 'abc');
    $c = $type->create();
    $c->set(2, new Bigint('999999999999'));

    foreach ([['a', $a], ['b', $b], ['c', $c]] as [$k, $v]) {
        $session->execute(
            "INSERT INTO $keyspace.$table (key, value) VALUES (?, ?)",
            ['arguments' => [$k, $v]]
        );
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    expect($rows->count())->toBe(3);
});

it('Throws InvalidQueryException when inserting a tuple of an incompatible type', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);
    $table = 'badtype_' . substr(bin2hex(random_bytes(4)), 0, 8);
    $session->execute(
        "CREATE TABLE $keyspace.$table (key text PRIMARY KEY, value frozen<tuple<int>>)"
    );

    $invalidType = Type::tuple(Type::varchar());
    $invalidValue = $invalidType->create('value');

    expect(fn () => $session->execute(
        "INSERT INTO $keyspace.$table (key, value) VALUES (?, ?)",
        ['arguments' => ['k', $invalidValue]]
    ))->toThrow(InvalidQueryException::class);
});

it('Builds a Tuple via the Tuple constructor', function () use ($keyspace) {
    $tuple = new Tuple([Type::int(), Type::varchar()]);
    $tuple->set(0, 7);
    $tuple->set(1, 'seven');

    tupleCreateInsertVerify($keyspace, 'frozen<tuple<int, varchar>>', $tuple);
});
