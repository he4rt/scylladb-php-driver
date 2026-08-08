<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Datatypes;

use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Date;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Smallint;
use Cassandra\Time;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Tinyint;
use Cassandra\Uuid;
use Cassandra\Varint;

$keyspace = 'datatype_batch_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

/**
 * Round-trip a single value of the given CQL scalar type via positional
 * and named bound parameters in a freshly created table.
 */
function roundTripScalar(string $keyspace, string $cqlType, mixed $value): void
{
    $session = scyllaDbConnection($keyspace);
    $table = 'tbl_' . str_replace('-', '', (string) (new Uuid()));
    $session->execute(
        "CREATE TABLE $keyspace.$table (key text PRIMARY KEY, value $cqlType)"
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

    $rows = $session->execute(
        "SELECT * FROM $keyspace.$table WHERE key = ?",
        ['arguments' => ['k1']]
    );
    expect($rows->count())->toBe(1);
    $row = $rows->first();
    if ($value !== null) {
        expect($row['value'])->toEqual($value);
    } else {
        expect($row['value'])->toBeNull();
    }
}

dataset('scalar_round_trip', [
    'ascii'     => ['ascii',    'hello'],
    'bigint'    => ['bigint',   new Bigint('123456789')],
    'blob'      => ['blob',     new Blob('binary-data')],
    'boolean_t' => ['boolean',  true],
    'boolean_f' => ['boolean',  false],
    'date'      => ['date',     new Date(0)],
    'decimal'   => ['decimal',  new Decimal('3.14')],
    // A positive exponent gives a negative scale, which the protocol allows.
    'decimal_negative_scale' => ['decimal', new Decimal('1e3')],
    'decimal_negative_scale_fraction' => ['decimal', new Decimal('-1.5e2')],
    'double'    => ['double',   3.14],
    'duration_zero' => ['duration', new Duration(0, 0, 0)],
    'duration_pos'  => ['duration', new Duration(1, 2, 3)],
    'int'       => ['int',      99],
    'inet'      => ['inet',     new Inet('127.0.0.1')],
    'smallint'  => ['smallint', new Smallint(74)],
    'text'      => ['text',     'hello world'],
    'time'      => ['time',     new Time(1234567890)],
    'tinyint'   => ['tinyint',  new Tinyint(37)],
    'timestamp' => ['timestamp', new Timestamp(123, 0)],
    'timeuuid'  => ['timeuuid', new Timeuuid(0)],
    'uuid'      => ['uuid',     new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c2')],
    'varchar'   => ['varchar',  'hello'],
    'varint'    => ['varint',   new Varint(99)],
]);

it('Round-trips each scalar Cassandra type', function (string $cqlType, mixed $value) use ($keyspace) {
    roundTripScalar($keyspace, $cqlType, $value);
})->with('scalar_round_trip');

it('Round-trips a null value for an int column', function () use ($keyspace) {
    roundTripScalar($keyspace, 'int', null);
});

it('Round-trips a null value for a varchar column', function () use ($keyspace) {
    roundTripScalar($keyspace, 'varchar', null);
});
