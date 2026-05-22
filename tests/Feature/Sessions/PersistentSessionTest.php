<?php

declare(strict_types=1);

use Cassandra\SimpleStatement;

/*
 * Persistent-mode regression coverage.
 *
 * Each test exercises EG(persistent_list) caching within the same PHP
 * process (CLI under Pest). The persistent_list survives the entire CLI
 * lifetime, so consecutive build()->connect() / prepare() calls hit the
 * cache the same way a fresh request inside a long-running FPM worker
 * does.
 *
 * Counters are inspected via persistentCounters() (parses phpinfo()); the
 * extension does not expose a userland accessor.
 */

$keyspace = 'persistent_sessions_test';

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE entries (id int PRIMARY KEY, value text)
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('reuses the cached CassCluster on identical build() calls', function () {
    $before = persistentCounters();

    $a = persistentScyllaDbBuilder()->build();
    $b = persistentScyllaDbBuilder()->build();

    $after = persistentCounters();

    // Two build()s on the same persistent config → exactly one CassCluster
    // ends up in the persistent_list, not two.
    expect($after['clusters'])->toBe($before['clusters'] + 1);

    unset($a, $b);
})->group('feature', 'persistent');

it('caches separate CassClusters for distinct configs', function () {
    $before = persistentCounters();

    $a = persistentScyllaDbBuilder()
        ->withConnectTimeout(5.0)
        ->build();
    $b = persistentScyllaDbBuilder()
        ->withConnectTimeout(7.5)
        ->build();

    $after = persistentCounters();

    // Different config → cache miss on the second build → two entries.
    expect($after['clusters'])->toBe($before['clusters'] + 2);

    unset($a, $b);
})->group('feature', 'persistent');

it('reuses a cached CassSession on identical connect() calls', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();
    $before  = persistentCounters();

    $s1 = $cluster->connect($keyspace);
    $s2 = $cluster->connect($keyspace);

    $after = persistentCounters();

    expect($after['sessions'])->toBe($before['sessions'] + 1);

    // Both should be usable.
    expect($s1->execute(new SimpleStatement('SELECT id FROM entries'))->count())->toBe(0);
    expect($s2->execute(new SimpleStatement('SELECT id FROM entries'))->count())->toBe(0);

    unset($s1, $s2, $cluster);
})->group('feature', 'persistent');

it('caches separate CassSessions for distinct keyspaces', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();
    $before  = persistentCounters();

    $s1 = $cluster->connect($keyspace);
    $s2 = $cluster->connect();  // no keyspace — different cache key

    $after = persistentCounters();

    expect($after['sessions'])->toBe($before['sessions'] + 2);

    unset($s1, $s2, $cluster);
})->group('feature', 'persistent');

it('reuses cached prepared statements on identical prepare() calls', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();
    $session = $cluster->connect($keyspace);
    $before  = persistentCounters();

    $cql = 'SELECT value FROM entries WHERE id = ?';
    $p1  = $session->prepare($cql);
    $p2  = $session->prepare($cql);

    $after = persistentCounters();

    expect($after['prepared'])->toBe($before['prepared'] + 1);

    // Both should bind+execute cleanly.
    expect($session->execute($p1, ['arguments' => [1]])->count())->toBe(0);
    expect($session->execute($p2, ['arguments' => [2]])->count())->toBe(0);

    unset($p1, $p2, $session, $cluster);
})->group('feature', 'persistent');

it('caches separate prepared statements for distinct CQL', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();
    $session = $cluster->connect($keyspace);
    $before  = persistentCounters();

    $p1 = $session->prepare('SELECT value FROM entries WHERE id = ?');
    $p2 = $session->prepare('SELECT id FROM entries WHERE value = ? ALLOW FILTERING');

    $after = persistentCounters();

    expect($after['prepared'])->toBe($before['prepared'] + 2);

    unset($p1, $p2, $session, $cluster);
})->group('feature', 'persistent');

it('does not poison the cluster cache when a connect() fails', function () {
    // A cluster built against an unreachable host should not leave a
    // usable cached session that would feed subsequent connects.
    $bad = Cassandra::cluster()
        ->withContactPoints('127.0.0.1')
        ->withPort(1)            // closed port
        ->withConnectTimeout(0.5)
        ->withPersistentSessions(true)
        ->build();

    $caught = null;
    try {
        $bad->connect();
    } catch (\Throwable $e) {
        $caught = $e;
    }

    expect($caught)->not->toBeNull();

    // The good builder should still produce a working session — i.e. the
    // broken-future invalidation path removed the stale entry rather than
    // letting it leak into other cache keys.
    $good = persistentScyllaDbBuilder()->build();
    $s    = $good->connect();
    expect($s)->not->toBeNull();

    unset($s, $good, $bad);
})->group('feature', 'persistent');

it('keeps a session alive after its originating $cluster is unset (hash_key UAF regression)', function () use ($keyspace) {
    // Regression for the hash_key UAF fixed in this branch: the session
    // used to alias the cluster's hash_key as a raw char*, and dropping
    // $cluster would efree() the buffer out from under the session,
    // dangling on the next prepare().
    $cluster = persistentScyllaDbBuilder()->build();
    $session = $cluster->connect($keyspace);

    unset($cluster);  // drop the cluster zval — used to dangle session->hash_key

    // Force a prepare(), which is the path that spprintf's against the
    // hash_key. Must not crash/segfault.
    $stmt = $session->prepare('SELECT id FROM entries WHERE id = ?');
    expect($stmt)->not->toBeNull();

    expect($session->execute($stmt, ['arguments' => [1]])->count())->toBe(0);
})->group('feature', 'persistent');
