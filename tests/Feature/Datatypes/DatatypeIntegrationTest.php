<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Datatypes;

use Cassandra\Decimal;
use Cassandra\Timeuuid;
use Cassandra\Varint;

$keyspace = 'datatype_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    CREATE TABLE $keyspace.byte_boundary (key timeuuid PRIMARY KEY, value_decimal decimal, value_varint varint);
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('Encodes Decimal and Varint correctly on byte boundaries', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    foreach (range(1, 20) as $i) {
        $key = new Timeuuid();
        $valueVarint = (2 ** (8 * $i)) - 1;
        $valueDecimal = $valueVarint / 100;

        $session->execute(
            "INSERT INTO $keyspace.byte_boundary (key, value_decimal, value_varint) VALUES (?, ?, ?)",
            ['arguments' => [$key, new Decimal($valueDecimal), new Varint($valueVarint)]]
        );

        $rows = $session->execute(
            "SELECT value_decimal, value_varint FROM $keyspace.byte_boundary WHERE key = ?",
            ['arguments' => [$key]]
        );

        expect($rows->count())->toBe(1);
        $row = $rows->first();
        expect($row)->toHaveKey('value_decimal')
            ->and($row['value_decimal'])->toEqual(new Decimal($valueDecimal))
            ->and($row)->toHaveKey('value_varint')
            ->and($row['value_varint'])->toEqual(new Varint($valueVarint));
    }
});
