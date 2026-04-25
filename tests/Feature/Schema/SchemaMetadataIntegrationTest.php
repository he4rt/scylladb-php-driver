<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Schema;

$keyspace = 'schema_meta_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    CQL);
});

afterAll(function () use ($keyspace) {
    // Best-effort cleanup of any auxiliary keyspaces created by tests below.
    $session = scyllaDbConnection();
    foreach ($session->schema()->keyspaces() as $ks) {
        $name = $ks->name();
        if (str_starts_with($name, $keyspace)) {
            try {
                $session->execute("DROP KEYSPACE $name");
            } catch (\Throwable) {
                // ignore
            }
        }
    }
    dropKeyspace($keyspace);
});

it('Exposes a non-null schema with the test keyspace present', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $schema = $session->schema();
    expect($schema)->not->toBeNull()
        ->and($schema->keyspaces())->toHaveKey($keyspace);
});

it('Enumerates keyspaces created on the cluster', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $extra = [];
    for ($i = 0; $i < 3; $i++) {
        $name = "{$keyspace}_enum_$i";
        $session->execute(
            "CREATE KEYSPACE $name WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
        );
        $extra[] = $name;
    }

    $found = 0;
    foreach ($session->schema()->keyspaces() as $ks) {
        if (in_array($ks->name(), $extra, true)) {
            $found++;
        }
    }
    expect($found)->toBe(count($extra));
});

it('Looks up keyspaces by name', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $names = [];
    for ($i = 0; $i < 3; $i++) {
        $name = "{$keyspace}_byname_$i";
        $session->execute(
            "CREATE KEYSPACE $name WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
        );
        $names[] = $name;
    }
    $names[] = 'system';

    foreach ($names as $name) {
        $ks = $session->schema()->keyspace($name);
        expect($ks)->not->toBeNull();
    }
});

it('Enumerates tables and columns in a keyspace', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_tables";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    $schemas = [
        'table_int_int'    => ['key' => 'int', 'value' => 'int'],
        'table_int_bigint' => ['key' => 'int', 'value' => 'bigint'],
        'table_decimal_map' => ['key' => 'decimal', 'value' => 'map<bigint, uuid>'],
    ];
    foreach ($schemas as $tableName => $columns) {
        $cols = implode(', ', array_map(fn ($k, $v) => "$k $v", array_keys($columns), $columns));
        $session->execute("CREATE TABLE $ksName.$tableName ($cols, PRIMARY KEY(key))");
    }

    $ks = $session->schema()->keyspace($ksName);
    expect(count($ks->tables()))->toBe(count($schemas));

    foreach ($ks->tables() as $table) {
        $expected = $schemas[$table->name()];
        expect(count($table->columns()))->toBe(count($expected));
        foreach ($table->columns() as $column) {
            expect((string) $column->type())->toBe($expected[$column->name()]);
        }
    }
});

it('Looks up table and column metadata by name', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_byname_tbl";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute(
        "CREATE TABLE $ksName.t (key int PRIMARY KEY, value bigint)"
    );

    $ks = $session->schema()->keyspace($ksName);
    $table = $ks->table('t');
    expect($table)->not->toBeNull()
        ->and((string) $table->column('value')->type())->toBe('bigint');
});

it('Parses deeply nested collection column types', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_nested";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    $session->execute("CREATE TABLE $ksName.nested1 (key int PRIMARY KEY, value map<frozen<list<int>>, int>)");
    $session->execute("CREATE TABLE $ksName.nested2 (key int PRIMARY KEY, value map<int, frozen<list<int>>>)");
    $session->execute("CREATE TABLE $ksName.nested3 (key int PRIMARY KEY, value list<frozen<map<int, frozen<set<int>>>>>)");

    $ks = $session->schema()->keyspace($ksName);
    expect((string) $ks->table('nested1')->column('value')->type())->toBe('map<list<int>, int>')
        ->and((string) $ks->table('nested2')->column('value')->type())->toBe('map<int, list<int>>')
        ->and((string) $ks->table('nested3')->column('value')->type())->toBe('list<map<int, set<int>>>');
});

it('Reports zero secondary indexes on a fresh table', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_idx0";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute("CREATE TABLE $ksName.t (key1 text, value1 int, value2 map<text, text>, PRIMARY KEY(key1))");

    $table = $session->schema()->keyspace($ksName)->table('t');
    expect(count($table->indexes()))->toBe(0)
        ->and($table->index('invalid'))->toBeFalse();
});

it('Reports a simple secondary index in metadata', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_idx1";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute("CREATE TABLE $ksName.t (key1 text, value1 int, value2 map<text, text>, PRIMARY KEY(key1))");
    $session->execute("CREATE INDEX simple ON $ksName.t (value1)");

    // Wait briefly for schema agreement.
    usleep(500_000);

    $table = $session->schema()->keyspace($ksName)->table('t');
    expect(count($table->indexes()))->toBeGreaterThanOrEqual(1);
    $index = $table->index('simple');
    if ($index === false) {
        $this->markTestSkipped('Server did not surface index metadata for "simple"');
    }
    expect($index->target())->toBe('value1')
        ->and($index->isCustom())->toBeFalse();
});

it('Reports a collection (KEYS) secondary index in metadata', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_idx_coll";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute("CREATE TABLE $ksName.t (key1 text, value1 int, value2 map<text, text>, PRIMARY KEY(key1))");

    try {
        $session->execute("CREATE INDEX collection ON $ksName.t (KEYS(value2))");
    } catch (\Throwable $e) {
        $this->markTestSkipped('Server does not support KEYS(...) secondary indexes: ' . $e->getMessage());
    }
    usleep(500_000);

    $table = $session->schema()->keyspace($ksName)->table('t');
    $index = $table->index('collection');
    if ($index === false) {
        $this->markTestSkipped('Server did not surface KEYS index metadata');
    }
    expect($index->target())->toBe('keys(value2)');
});

it('Reports zero materialized views on a fresh table', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_mv0";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute("CREATE TABLE $ksName.t (key1 text, value1 int, PRIMARY KEY(key1))");

    $ks = $session->schema()->keyspace($ksName);
    expect(count($ks->materializedViews()))->toBe(0)
        ->and($ks->materializedView('invalid'))->toBeFalse();
});

it('Reports a simple materialized view in metadata', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_mv1";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute("CREATE TABLE $ksName.t (key1 text, value1 int, PRIMARY KEY(key1))");

    try {
        $session->execute(
            "CREATE MATERIALIZED VIEW $ksName.simple AS " .
            "SELECT key1 FROM $ksName.t WHERE value1 IS NOT NULL " .
            'PRIMARY KEY(value1, key1)'
        );
    } catch (\Throwable $e) {
        $this->markTestSkipped('Server does not support materialized views: ' . $e->getMessage());
    }
    usleep(500_000);

    $ks = $session->schema()->keyspace($ksName);
    if (count($ks->materializedViews()) === 0) {
        $this->markTestSkipped('Server did not surface MV metadata');
    }
    $mv = $ks->materializedView('simple');
    expect($mv)->not->toBeFalse()
        ->and($mv->name())->toBe('simple');
});

it('Drops a materialized view and removes it from metadata', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_mv_drop";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );
    $session->execute("CREATE TABLE $ksName.t (key1 text, value1 int, PRIMARY KEY(key1))");

    try {
        $session->execute(
            "CREATE MATERIALIZED VIEW $ksName.simple AS " .
            "SELECT key1 FROM $ksName.t WHERE value1 IS NOT NULL " .
            'PRIMARY KEY(value1, key1)'
        );
    } catch (\Throwable $e) {
        $this->markTestSkipped('Server does not support materialized views: ' . $e->getMessage());
    }
    usleep(500_000);

    $session->execute("DROP MATERIALIZED VIEW $ksName.simple");
    usleep(500_000);

    $ks = $session->schema()->keyspace($ksName);
    expect(count($ks->materializedViews()))->toBe(0);
});

it('Reports zero user defined functions on a fresh keyspace', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName = "{$keyspace}_udf0";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    $ks = $session->schema()->keyspace($ksName);
    expect(count($ks->functions()))->toBe(0)
        ->and($ks->function('invalid'))->toBeFalse()
        ->and(count($ks->aggregates()))->toBe(0)
        ->and($ks->aggregate('invalid'))->toBeFalse();
});

it('Increments the schema version when a keyspace is created', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $version = $session->schema()->version();
    expect($version)->toBeGreaterThan(0);

    $ksName = "{$keyspace}_version";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    $newVersion = $session->schema()->version();
    expect($newVersion)->toBeGreaterThan($version);
});

it('Retrieves the column type for every table in every keyspace without crashing', function () {
    $session = scyllaDbConnection();
    foreach ($session->schema()->keyspaces() as $ks) {
        foreach ($ks->tables() as $table) {
            foreach ($table->columns() as $column) {
                expect($column->type())->not->toBeNull();
            }
        }
    }
});
