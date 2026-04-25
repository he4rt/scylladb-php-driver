<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

use Cassandra;
use Cassandra\BatchStatement;
use Cassandra\Session;
use Cassandra\SimpleStatement;
use Cassandra\Statement;
use Cassandra\TimestampGenerator;

const TS_SECONDS_TO_MICROSECONDS = 1000000;

$keyspace = 'timestamp_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

function tsClientSession(string $keyspace): Session
{
    $hosts = env('SCYLLADB_HOSTS', '127.0.0.1');
    if (is_string($hosts)) {
        $hosts = explode(',', $hosts);
    }

    return Cassandra::cluster()
        ->withContactPoints(...$hosts)
        ->withPort((int) env('SCYLLADB_PORT', 9042))
        ->withCredentials(
            env('SCYLLADB_USERNAME', 'cassandra'),
            env('SCYLLADB_PASSWORD', 'cassandra'),
        )
        ->withPersistentSessions(false)
        ->withTokenAwareRouting(true)
        ->withTimestampGenerator(new TimestampGenerator\Monotonic())
        ->build()
        ->connect($keyspace);
}

function tsNowMicroseconds(): int
{
    return (int) round(microtime(true) * TS_SECONDS_TO_MICROSECONDS);
}

function tsServerNow(Session $session): int
{
    $rows = $session->execute('SELECT dateOf(now()) FROM system.local');
    return current($rows->first())->time() * TS_SECONDS_TO_MICROSECONDS;
}

/**
 * @param int|string|null $key
 * @param int|string|null $value
 * @param int|string|null $timestamp
 */
function tsInsert(Session $session, Statement $statement, $key = null, $value = null, $timestamp = null): void
{
    $parameters = [];
    if (isset($key) || isset($value)) {
        $arguments = [];
        if (isset($key)) {
            $arguments['key'] = $key;
        }
        if (isset($value)) {
            $arguments['value_int'] = $value;
        }
        $parameters['arguments'] = $arguments;
    }
    if (isset($timestamp)) {
        $parameters['timestamp'] = $timestamp;
    }
    $session->execute($statement, $parameters);
}

function tsGetTimestamp(Session $session, string $keyspace, string $table, int $key): int
{
    $rows = $session->execute(
        "SELECT writeTime(value_int) FROM $keyspace.$table WHERE key = ?",
        ['arguments' => ['key' => $key]]
    );
    $row = $rows->first();
    expect($row)->toHaveKey('writetime(value_int)');
    return current($row)->value();
}

function tsAssertTimestamp(
    Session $session,
    string $keyspace,
    string $table,
    int $key,
    int $expectedTimestamp,
    bool $isEqual = true
): void {
    $timestamp = tsGetTimestamp($session, $keyspace, $table, $key);
    if ($isEqual) {
        expect($timestamp)->toEqual($expectedTimestamp);
    } else {
        expect($timestamp)->toBeGreaterThan($expectedTimestamp);
    }
}

it('supports timestamps with simple statements', function () use ($keyspace) {
    $table = 'ts_simple';
    migrateKeyspace("CREATE TABLE $keyspace.$table (key int PRIMARY KEY, value_int int)");

    $session = scyllaDbConnection($keyspace);
    $clientSession = tsClientSession($keyspace);

    $insertQuery = "INSERT INTO $keyspace.$table (key, value_int) VALUES (?, ?)";

    $statement = new SimpleStatement($insertQuery);
    tsInsert($session, $statement, 0, 1, 12345);
    tsAssertTimestamp($session, $keyspace, $table, 0, 12345);

    tsInsert($session, $statement, 1, 2, 12345);
    tsAssertTimestamp($session, $keyspace, $table, 1, 12345);

    $now = tsNowMicroseconds();
    sleep(1);
    tsInsert($clientSession, $statement, 2, 3);
    tsAssertTimestamp($session, $keyspace, $table, 2, $now, false);

    $serverNow = tsServerNow($session);
    sleep(1);
    tsInsert($session, $statement, 3, 4);
    tsAssertTimestamp($session, $keyspace, $table, 3, $serverNow, false);

    $statementUsing = new SimpleStatement("$insertQuery USING TIMESTAMP 30");
    tsInsert($session, $statementUsing, 4, 5, 12345);
    tsAssertTimestamp($session, $keyspace, $table, 4, 30);
})->skip(version_compare(Cassandra::CPP_DRIVER_VERSION, '2.1') < 0, 'Requires C/C++ driver >= 2.1');

it('supports timestamps with prepared statements', function () use ($keyspace) {
    $table = 'ts_prepared';
    migrateKeyspace("CREATE TABLE $keyspace.$table (key int PRIMARY KEY, value_int int)");

    $session = scyllaDbConnection($keyspace);
    $clientSession = tsClientSession($keyspace);

    $insertQuery = "INSERT INTO $keyspace.$table (key, value_int) VALUES (?, ?)";

    $statement = $session->prepare($insertQuery);
    tsInsert($session, $statement, 0, 1, 54321);
    tsAssertTimestamp($session, $keyspace, $table, 0, 54321);

    tsInsert($session, $statement, 1, 2, '54321');
    tsAssertTimestamp($session, $keyspace, $table, 1, 54321);

    $now = tsNowMicroseconds();
    sleep(1);
    tsInsert($clientSession, $statement, 2, 3);
    tsAssertTimestamp($session, $keyspace, $table, 2, $now, false);

    $serverNow = tsServerNow($session);
    sleep(1);
    tsInsert($session, $statement, 3, 4);
    tsAssertTimestamp($session, $keyspace, $table, 3, $serverNow, false);

    $statementUsing = $session->prepare("$insertQuery USING TIMESTAMP 60");
    tsInsert($session, $statementUsing, 4, 5, 54321);
    tsAssertTimestamp($session, $keyspace, $table, 4, 60);
})->skip(version_compare(Cassandra::CPP_DRIVER_VERSION, '2.1') < 0, 'Requires C/C++ driver >= 2.1');

it('supports timestamps with batch statements', function () use ($keyspace) {
    $table = 'ts_batch';
    migrateKeyspace("CREATE TABLE $keyspace.$table (key int PRIMARY KEY, value_int int)");

    $session = scyllaDbConnection($keyspace);
    $clientSession = tsClientSession($keyspace);

    $insertQuery = "INSERT INTO $keyspace.$table (key, value_int) VALUES (?, ?)";

    $batch = new BatchStatement(Cassandra::BATCH_UNLOGGED);
    $prepare = $session->prepare($insertQuery);

    $batch->add($insertQuery, [0, 1]);
    $batch->add($prepare, ['key' => 1, 'value_int' => 2]);
    $batch->add("$insertQuery USING TIMESTAMP 90", [2, 3]);

    tsInsert($session, $batch, null, null, 11111);
    tsAssertTimestamp($session, $keyspace, $table, 0, 11111);
    tsAssertTimestamp($session, $keyspace, $table, 1, 11111);
    tsAssertTimestamp($session, $keyspace, $table, 2, 90);

    $now = tsNowMicroseconds();
    sleep(1);
    tsInsert($clientSession, $batch);
    tsAssertTimestamp($session, $keyspace, $table, 0, $now, false);
    tsAssertTimestamp($session, $keyspace, $table, 1, $now, false);
    tsAssertTimestamp($session, $keyspace, $table, 2, 90);

    $serverNow = tsServerNow($session);
    sleep(1);
    tsInsert($session, $batch);
    tsAssertTimestamp($session, $keyspace, $table, 0, $serverNow, false);
    tsAssertTimestamp($session, $keyspace, $table, 1, $serverNow, false);
    tsAssertTimestamp($session, $keyspace, $table, 2, 90);
})->skip(version_compare(Cassandra::CPP_DRIVER_VERSION, '2.1') < 0, 'Requires C/C++ driver >= 2.1');
