<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Schema;

$keyspace = 'mv_feat_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    AND DURABLE_WRITES = false
    CQL);

    migrateKeyspace("CREATE TABLE $keyspace.users (id int PRIMARY KEY, name text)");

    // ScyllaDB MV is experimental — skip from a beforeEach inside the test if not surfaced.
    try {
        migrateKeyspace(
            "CREATE MATERIALIZED VIEW IF NOT EXISTS $keyspace.users_view AS " .
            "SELECT name, id FROM $keyspace.users WHERE name IS NOT NULL AND id IS NOT NULL " .
            'PRIMARY KEY(name, id)'
        );
    } catch (\Throwable) {
        // Ignored; the test below skips when materializedViews() is empty.
    }
});

afterAll(fn () => dropKeyspace($keyspace));

it('returns metadata for a materialized view (name, base table, options)', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ks = $session->schema()->keyspace($keyspace);

    if (count($ks->materializedViews()) === 0) {
        $this->markTestSkipped('Server (likely ScyllaDB without MV enabled) did not surface materialized view metadata');
    }

    $view = $ks->materializedView('users_view');
    if ($view === false) {
        $this->markTestSkipped('users_view not surfaced in metadata');
    }

    expect($view->name())->toBe('users_view')
        ->and($view->baseTable()->name())->toBe('users')
        ->and((int) $view->option('default_time_to_live'))->toBe(0);

    $compression = $view->option('compression');
    // Compression option may be a Cassandra\Map or a plain array depending on the server.
    expect($compression)->not->toBeNull();
});
