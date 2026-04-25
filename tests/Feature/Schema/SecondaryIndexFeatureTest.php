<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Schema;

$keyspace = 'idx_feat_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    AND DURABLE_WRITES = false
    CQL);

    migrateKeyspace("CREATE TABLE $keyspace.users (id int PRIMARY KEY, name text)");
    migrateKeyspace("CREATE INDEX IF NOT EXISTS name_index ON $keyspace.users(name)");
});

afterAll(fn () => dropKeyspace($keyspace));

it('exposes basic secondary-index metadata: name, kind, target, isCustom', function () use ($keyspace) {
    $session = scyllaDbConnection();
    usleep(500_000);

    $table = $session->schema()->keyspace($keyspace)->table('users');
    $index = $table->index('name_index');
    if ($index === false) {
        $this->markTestSkipped('Server did not surface name_index metadata');
    }

    expect($index->name())->toBe('name_index')
        ->and($index->kind())->toBe('composites')
        ->and($index->target())->toBe('name')
        ->and($index->isCustom())->toBeFalse();
});

it('exposes index options() for a simple secondary index', function () use ($keyspace) {
    $session = scyllaDbConnection();
    usleep(500_000);

    $index = $session->schema()->keyspace($keyspace)->table('users')->index('name_index');
    if ($index === false) {
        $this->markTestSkipped('Server did not surface name_index metadata');
    }

    $opts = $index->options();
    expect($opts)->toBeArray()
        ->and($opts['target'] ?? null)->toBe('name');
});

it('exposes metadata for a CUSTOM secondary index', function () use ($keyspace) {
    $session = scyllaDbConnection();

    try {
        $session->execute(
            "CREATE CUSTOM INDEX name_custom_index ON $keyspace.users (name) " .
            "USING 'org.apache.cassandra.index.internal.composites.ClusteringColumnIndex'"
        );
    } catch (\Throwable $e) {
        $this->markTestSkipped('Server does not support CUSTOM INDEX (likely ScyllaDB): ' . $e->getMessage());
    }
    usleep(500_000);

    $index = $session->schema()->keyspace($keyspace)->table('users')->index('name_custom_index');
    if ($index === false) {
        $this->markTestSkipped('Server did not surface name_custom_index metadata');
    }

    expect($index->name())->toBe('name_custom_index')
        ->and($index->kind())->toBe('custom')
        ->and($index->target())->toBe('name')
        ->and($index->isCustom())->toBeTrue()
        ->and($index->className())->toContain('ClusteringColumnIndex');

    $opts = $index->options();
    expect($opts['class_name'] ?? null)->toContain('ClusteringColumnIndex')
        ->and($opts['target'] ?? null)->toBe('name');
});
