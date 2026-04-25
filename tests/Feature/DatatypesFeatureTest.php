<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature;

use Cassandra\Date;
use Cassandra\Smallint;
use Cassandra\Time;
use Cassandra\Tinyint;

/*
 * Migrated from tests-old/features/datatypes.feature
 *
 * The original output-matching expectations relied on specific stringification
 * formats produced by the legacy Behat harness. Here we assert the round-trip
 * value semantics directly via the driver's typed objects, which is a stronger
 * check than substring-matching stdout.
 */

$keyspace = 'datatypes_feat_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace("CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}");
});

afterAll(fn () => dropKeyspace($keyspace));

it('round-trips the core Cassandra value types', function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE TABLE $keyspace.values (
        id int PRIMARY KEY,
        bigint_value bigint,
        decimal_value decimal,
        double_value double,
        float_value float,
        int_value int,
        varint_value varint,
        timestamp_value timestamp,
        blob_value blob,
        uuid_value uuid,
        timeuuid_value timeuuid,
        inet_value inet
    );
    INSERT INTO $keyspace.values (
        id, bigint_value, decimal_value, double_value, float_value, int_value,
        varint_value, timestamp_value, blob_value, uuid_value, timeuuid_value, inet_value
    ) VALUES (
        0,
        -765438000,
        1313123123.234234234234234234123,
        3.141592653589793,
        3.14,
        4,
        67890656781923123918798273492834712837198237,
        1425691864001,
        varcharAsBlob('0x000000'),
        ab3352d9-4f7f-4007-a35a-e62aa7ab0b19,
        maxTimeuuid('2015-03-11 14:47:10+0000'),
        '200.199.198.197'
    )
    CQL);

    $session = scyllaDbConnection($keyspace);
    $row = $session->execute('SELECT * FROM values')->first();

    expect((string) $row['bigint_value'])->toBe('-765438000')
        ->and((string) $row['int_value'])->toBe('4')
        ->and((string) $row['varint_value'])->toBe('67890656781923123918798273492834712837198237')
        ->and((string) $row['inet_value'])->toBe('200.199.198.197')
        ->and((string) $row['uuid_value'])->toBe('ab3352d9-4f7f-4007-a35a-e62aa7ab0b19')
        ->and((string) $row['decimal_value'])->toBe('1313123123.234234234234234234123');

    expect($row['double_value'])->toEqualWithDelta(3.141592653589793, 1e-12);
    expect($row['float_value'])->toEqualWithDelta(3.14, 1e-5);
});

it('supports tinyint and smallint', function () use ($keyspace) {
    migrateKeyspace("CREATE TABLE $keyspace.tinysmall (id int PRIMARY KEY, tinyint_value tinyint, smallint_value smallint)");

    $session = scyllaDbConnection($keyspace);
    $session->execute(
        "INSERT INTO $keyspace.tinysmall (id, tinyint_value, smallint_value) VALUES (?, ?, ?)",
        ['arguments' => [1, new Tinyint(127), new Smallint(32767)]]
    );

    $row = $session->execute("SELECT * FROM $keyspace.tinysmall")->first();
    expect((string) $row['tinyint_value'])->toBe('127')
        ->and((string) $row['smallint_value'])->toBe('32767');
});

it('supports the date type', function () use ($keyspace) {
    migrateKeyspace("CREATE TABLE $keyspace.date_values (id int PRIMARY KEY, date_value date)");

    $session = scyllaDbConnection($keyspace);
    $session->execute(
        "INSERT INTO $keyspace.date_values (id, date_value) VALUES (?, ?)",
        ['arguments' => [1, new Date(0)]]
    );

    $row = $session->execute("SELECT * FROM $keyspace.date_values")->first();
    expect($row['date_value']->toDateTime()->format('Y-m-d H:i:s'))->toBe('1970-01-01 00:00:00');
});

it('supports the time type', function () use ($keyspace) {
    migrateKeyspace("CREATE TABLE $keyspace.time_values (id int PRIMARY KEY, time_value time)");

    $session = scyllaDbConnection($keyspace);
    $datetime = new \DateTime('1970-01-01T00:00:01+0000');
    $session->execute(
        "INSERT INTO $keyspace.time_values (id, time_value) VALUES (?, ?)",
        ['arguments' => [1, Time::fromDateTime($datetime)]]
    );

    $row = $session->execute("SELECT * FROM $keyspace.time_values")->first();
    expect((string) $row['time_value'])->toBe('1000000000');
});
