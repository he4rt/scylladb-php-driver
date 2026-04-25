<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Schema;

use Cassandra;

$keyspace = 'schema_meta_feat_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}
    AND DURABLE_WRITES = false
    CQL);

    // Note: ScyllaDB rejects/ignores some legacy table options Cassandra accepted
    // (populate_io_cache_on_flush, replicate_on_write, dclocal_read_repair_chance,
    // read_repair_chance). Stick to portable options here.
    migrateKeyspace(<<<CQL
    CREATE TABLE $keyspace.values (
        id int PRIMARY KEY,
        bigint_value bigint,
        decimal_value decimal,
        double_value double,
        float_value float,
        int_value int,
        varint_value varint,
        ascii_value ascii,
        text_value text,
        varchar_value varchar,
        timestamp_value timestamp,
        blob_value blob,
        uuid_value uuid,
        timeuuid_value timeuuid,
        inet_value inet,
        list_value list<text>,
        map_value map<timestamp, double>,
        set_value set<float>
    ) WITH
        bloom_filter_fp_chance = 0.5 AND
        comment = 'Schema Metadata Feature' AND
        compaction = {'class': 'LeveledCompactionStrategy', 'sstable_size_in_mb': 37} AND
        gc_grace_seconds = 3600
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('returns keyspace metadata: name, replication and durable writes', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ks = $session->schema()->keyspace($keyspace);

    expect($ks)->not->toBeNull()
        ->and($ks->name())->toBe($keyspace)
        ->and($ks->replicationClassName())->toContain('SimpleStrategy')
        ->and($ks->replicationOptions()['replication_factor'] ?? null)->toBe('1')
        ->and($ks->hasDurableWrites())->toBeFalse();
});

it('returns table metadata: bloom filter, comment, compaction, gc grace', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $table   = $session->schema()->keyspace($keyspace)->table('values');

    expect($table)->not->toBeNull()
        ->and($table->name())->toBe('values')
        ->and((float) $table->bloomFilterFPChance())->toBe(0.5)
        ->and($table->comment())->toBe('Schema Metadata Feature')
        ->and($table->compactionStrategyClassName())->toContain('LeveledCompactionStrategy')
        ->and((int) $table->gcGraceSeconds())->toBe(3600);
});

it('exposes column type metadata for every supported scalar/collection', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $table   = $session->schema()->keyspace($keyspace)->table('values');

    $expectedSubstr = [
        'ascii_value'    => 'ascii',
        'bigint_value'   => 'bigint',
        'blob_value'     => 'blob',
        'decimal_value'  => 'decimal',
        'double_value'   => 'double',
        'float_value'    => 'float',
        'id'             => 'int',
        'inet_value'     => 'inet',
        'int_value'      => 'int',
        'list_value'     => 'list<',
        'map_value'      => 'map<',
        'set_value'      => 'set<',
        'timestamp_value' => 'timestamp',
        'timeuuid_value' => 'timeuuid',
        'uuid_value'     => 'uuid',
        'varint_value'   => 'varint',
    ];

    foreach ($expectedSubstr as $name => $needle) {
        $col = $table->column($name);
        expect($col)->not->toBeNull()
            ->and((string) $col->type())->toContain($needle);
    }
});

it('exposes user-defined type metadata in the keyspace', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName  = "{$keyspace}_udt";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    try {
        $session->execute("CREATE TYPE $ksName.type1 (a int, b text)");
        $session->execute("CREATE TYPE $ksName.type2 (a map<text, int>, b bigint)");
        $session->execute("CREATE TYPE $ksName.type3 (a map<text, frozen<set<varint>>>, b list<uuid>)");
    } catch (\Throwable $e) {
        $session->execute("DROP KEYSPACE $ksName");
        $this->markTestSkipped('Server does not support UDTs: ' . $e->getMessage());
    }
    usleep(500_000);

    try {
        $ks = $session->schema()->keyspace($ksName);
        $userTypes = $ks->userTypes();
        $names = [];
        foreach ($userTypes as $name => $_type) {
            $names[] = $name;
        }
        expect($names)->toContain('type1')
            ->and($names)->toContain('type2')
            ->and($names)->toContain('type3');
    } finally {
        $session->execute("DROP KEYSPACE $ksName");
    }
});

it('returns no keyspaces when schema metadata is disabled', function () {
    $hosts = explode(',', (string) env('SCYLLADB_HOSTS', '127.0.0.1'));

    $session = Cassandra::cluster()
        ->withContactPoints(...$hosts)
        ->withPort((int) env('SCYLLADB_PORT', 9042))
        ->withCredentials(env('SCYLLADB_USERNAME', 'cassandra'), env('SCYLLADB_PASSWORD', 'cassandra'))
        ->withSchemaMetadata(false)
        ->withPersistentSessions(false)
        ->build()
        ->connect();

    expect(count($session->schema()->keyspaces()))->toBe(0);
});

it('skips legacy populate_io_cache_on_flush / replicate_on_write metadata (ScyllaDB)', function () {
    // ScyllaDB does not implement these legacy Cassandra options.
    $this->markTestSkipped('Legacy table options populateIOCacheOnFlush/replicateOnWrite are not supported by ScyllaDB');
});

it('skips index_interval metadata (Cassandra 2.0 only)', function () {
    $this->markTestSkipped('index_interval is a Cassandra 2.0-only option, removed in newer servers and not supported on ScyllaDB');
});

it('exposes default_time_to_live, memtable_flush_period, speculative_retry table options', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName  = "{$keyspace}_alter";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    try {
        $session->execute("CREATE TABLE $ksName.t (id int PRIMARY KEY, v int)");
        try {
            $session->execute(
                "ALTER TABLE $ksName.t WITH default_time_to_live = 10000 AND " .
                "memtable_flush_period_in_ms = 100 AND " .
                "speculative_retry = '10ms'"
            );
        } catch (\Throwable $e) {
            $this->markTestSkipped('Server rejected ALTER TABLE options: ' . $e->getMessage());
        }
        usleep(500_000);

        $table = $session->schema()->keyspace($ksName)->table('t');
        expect((int) $table->defaultTTL())->toBe(10000)
            ->and((int) $table->memtableFlushPeriodMs())->toBe(100)
            ->and((string) $table->speculativeRetry())->toContain('10');
    } finally {
        $session->execute("DROP KEYSPACE $ksName");
    }
});

it('exposes max/min index intervals on a table', function () use ($keyspace) {
    $session = scyllaDbConnection();
    $ksName  = "{$keyspace}_idxint";
    $session->execute(
        "CREATE KEYSPACE $ksName WITH REPLICATION = {'class':'SimpleStrategy', 'replication_factor':1}"
    );

    try {
        $session->execute("CREATE TABLE $ksName.t (id int PRIMARY KEY, v int)");
        try {
            $session->execute(
                "ALTER TABLE $ksName.t WITH max_index_interval = 16 AND min_index_interval = 4"
            );
        } catch (\Throwable $e) {
            $this->markTestSkipped('Server rejected min/max_index_interval: ' . $e->getMessage());
        }
        usleep(500_000);

        $table = $session->schema()->keyspace($ksName)->table('t');
        expect((int) $table->maxIndexInterval())->toBe(16)
            ->and((int) $table->minIndexInterval())->toBe(4);
    } finally {
        $session->execute("DROP KEYSPACE $ksName");
    }
});
