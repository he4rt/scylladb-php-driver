<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

use Cassandra;
use Cassandra\Exception\InvalidQueryException;
use Cassandra\SimpleStatement;
use Cassandra\Timeuuid;

$keyspace = 'simple_stmt_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

// Each test uses its own table inside the shared keyspace so they don't collide.

it('supports named parameters by name', function () use ($keyspace) {
    $table = 'by_name';
    migrateKeyspace(
        "CREATE TABLE $keyspace.$table (key timeuuid, value_int int, value_boolean boolean, value_text text, "
        . 'PRIMARY KEY (value_int, key)) WITH CLUSTERING ORDER BY (key DESC)'
    );

    $session = scyllaDbConnection($keyspace);

    $values = [
        [
            'value_text'    => 'This is row number one.',
            'value_boolean' => true,
            'value_int'     => 1,
            'key'           => new Timeuuid(),
        ],
        [
            'value_int'     => 2,
            'key'           => new Timeuuid(),
            'value_boolean' => false,
            'value_text'    => 'This is row number two.',
        ],
        [
            'key'           => new Timeuuid(),
            'value_int'     => 3,
            'value_boolean' => false,
            'value_text'    => 'This is row number three.',
        ],
    ];

    $statement = new SimpleStatement(
        "INSERT INTO $keyspace.$table (key, value_int, value_boolean, value_text) VALUES (?, ?, ?, ?)"
    );

    foreach ($values as $value) {
        $session->execute($statement, ['arguments' => $value]);
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    expect($rows->count())->toBe(count($values));

    foreach ($rows as $i => $row) {
        $value = $values[$i];
        expect($row)->toEqual($value);
        expect($row['key'])->toEqual($value['key']);
        expect($row['value_int'])->toEqual($value['value_int']);
        expect($row['value_boolean'])->toEqual($value['value_boolean']);
        expect($row['value_text'])->toEqual($value['value_text']);
    }
});

it('supports case-sensitive named parameters', function () use ($keyspace) {
    if (version_compare(Cassandra::CPP_DRIVER_VERSION, '2.2.3') < 0) {
        $this->markTestSkipped('Case sensitivity issue fixed in DataStax C/C++ v2.2.3');
    }

    $table = 'case_sensitive';
    migrateKeyspace(
        "CREATE TABLE $keyspace.$table (key timeuuid, value_int int, \"value_iNT\" int, value_boolean boolean, "
        . '"value_BooLeaN" boolean, PRIMARY KEY (value_int, key)) WITH CLUSTERING ORDER BY (key DESC)'
    );

    $session = scyllaDbConnection($keyspace);

    $values = [
        [
            '"value_BooLeaN"' => false,
            '"value_boolean"' => true,
            '"value_iNT"'     => 11,
            '"value_int"'     => 1,
            'key'             => new Timeuuid(),
        ],
        [
            '"value_int"'     => 2,
            '"value_BooLeaN"' => true,
            'key'             => new Timeuuid(),
            '"value_boolean"' => false,
            '"value_iNT"'     => 22,
        ],
        [
            'key'             => new Timeuuid(),
            '"value_int"'     => 3,
            '"value_iNT"'     => 33,
            '"value_boolean"' => false,
            '"value_BooLeaN"' => true,
        ],
    ];

    $statement = new SimpleStatement(
        "INSERT INTO $keyspace.$table (key, value_int, \"value_iNT\", value_boolean, \"value_BooLeaN\") "
        . 'VALUES (?, ?, ?, ?, ?)'
    );

    foreach ($values as $value) {
        $session->execute($statement, ['arguments' => $value]);
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$table");
    expect($rows->count())->toBe(count($values));

    foreach ($rows as $i => $row) {
        $expected = [];
        foreach ($values[$i] as $key => $value) {
            $expected[trim($key, '"')] = $value;
        }
        expect($row)->toEqual($expected);
    }
});

it('throws when binding by an invalid name', function () use ($keyspace) {
    $table = 'invalid_bind';
    migrateKeyspace("CREATE TABLE $keyspace.$table (key timeuuid PRIMARY KEY, value_int int)");

    $session = scyllaDbConnection($keyspace);

    $values = [
        'key'        => new Timeuuid(),
        'wrong_name' => 1,
    ];

    expect(fn () => $session->execute(
        new SimpleStatement("INSERT INTO $keyspace.$table (key, value_int) VALUES (?, ?)"),
        ['arguments' => $values]
    ))->toThrow(InvalidQueryException::class);
});

it('throws when binding by a case-sensitive invalid name', function () use ($keyspace) {
    if (version_compare(Cassandra::CPP_DRIVER_VERSION, '2.2.3') < 0) {
        $this->markTestSkipped('Case sensitivity issue fixed in DataStax C/C++ v2.2.3');
    }

    $table = 'case_invalid_bind';
    migrateKeyspace(
        "CREATE TABLE $keyspace.$table (key timeuuid PRIMARY KEY, \"value_iNT\" int, \"value_TeXT\" text)"
    );

    $session = scyllaDbConnection($keyspace);

    $values = [
        'key'        => new Timeuuid(),
        'value_iNT'  => 1,
        'value_text' => 'Exception will be thrown; case-sensitive',
    ];

    expect(fn () => $session->execute(
        new SimpleStatement(
            "INSERT INTO $keyspace.$table (key, \"value_TeXT\", \"value_iNT\") VALUES (?, ?, ?)"
        ),
        ['arguments' => $values]
    ))->toThrow(InvalidQueryException::class);
});

it('supports null values when using named parameters', function () use ($keyspace) {
    $table = 'null_values';
    migrateKeyspace(
        "CREATE TABLE $keyspace.$table (key timeuuid PRIMARY KEY, value_int int, value_boolean boolean, value_text text)"
    );

    $session = scyllaDbConnection($keyspace);

    $values = [
        'key'           => new Timeuuid(),
        'value_int'     => null,
        'value_boolean' => null,
        'value_text'    => 'Null values should exist for value_int and value_boolean',
    ];

    $session->execute(
        new SimpleStatement(
            "INSERT INTO $keyspace.$table (key, value_int, value_boolean, value_text) VALUES (?, ?, ?, ?)"
        ),
        ['arguments' => $values]
    );

    $rows = $session->execute(new SimpleStatement("SELECT * FROM $keyspace.$table"));
    expect($rows->count())->toBe(1);

    $row = $rows->first();
    expect($row)->toEqual($values);
    expect($row['key'])->toEqual($values['key']);
    expect($row['value_int'])->toBeNull();
    expect($row['value_boolean'])->toBeNull();
    expect($row['value_text'])->toEqual($values['value_text']);
});
