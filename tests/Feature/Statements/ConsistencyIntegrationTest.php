<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

$keyspace = 'consistency_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('uses LOCAL_ONE as the default consistency level for executed statements', function () use ($keyspace) {
    $table = 'default_consistency';
    migrateKeyspace("CREATE TABLE $keyspace.$table (key int PRIMARY KEY)");

    $session = scyllaDbConnection($keyspace);

    $insertQuery = "INSERT INTO $keyspace.$table (key) VALUES (1)";
    $session->execute($insertQuery);

    // Inspect the system_traces.sessions table to determine the consistency
    // level used by the driver. The original test toggled tracing at the
    // cluster level via CCM; here we rely on whatever traces ScyllaDB has
    // already collected (tracing must be enabled server-side).
    $rows = $session->execute('SELECT parameters FROM system_traces.sessions');

    $isAsserted = false;
    foreach ($rows as $row) {
        $parameters = $row['parameters'];
        if (! is_array($parameters) || ! isset($parameters['query'])) {
            continue;
        }
        if ($parameters['query'] === $insertQuery) {
            expect($parameters['consistency_level'])->toEqual('LOCAL_ONE');
            $isAsserted = true;
            break;
        }
    }

    expect($isAsserted)->toBeTrue();
})->skip(
    'Requires server-side tracing toggle (was driven by CCM in the legacy test). '
    . 'Re-enable when an in-driver tracing hook is wired up.'
);
