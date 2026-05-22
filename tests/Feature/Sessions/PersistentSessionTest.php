<?php

declare(strict_types=1);

use Cassandra\SimpleStatement;

/*
 * Persistent-mode regression coverage.
 *
 * The EG(persistent_list) caching path is internal to the extension; PHP
 * has no userland accessor for the counters. These tests don't try to
 * measure the cache by counting — instead they verify the behavioural
 * properties that would break if the persistent_list integration regressed:
 *
 *  - consecutive build()/connect()/prepare() calls return usable handles
 *    on both the cache-hit and cache-miss paths,
 *  - a failed connect() to one cluster doesn't poison subsequent good
 *    builds (broken-future invalidation),
 *  - dropping the $cluster zval before its session is destroyed doesn't
 *    dangle the session's hash_key (UAF regression for #128).
 *
 * The smoke-level "the cache exists and isn't catastrophically broken"
 * assertion is that none of these segfault and the produced sessions can
 * round-trip a CQL statement.
 */

$keyspace = 'persistent_sessions_test';

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE cache_log (id int PRIMARY KEY, payload text)
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('builds successive clusters on identical configs without error', function () {
    $a = persistentScyllaDbBuilder()->build();
    $b = persistentScyllaDbBuilder()->build();

    expect($a)->toBeInstanceOf(\Cassandra\Cluster::class);
    expect($b)->toBeInstanceOf(\Cassandra\Cluster::class);

    unset($a, $b);
})->group('feature', 'persistent');

it('builds distinct clusters on distinct configs without error', function () {
    $a = persistentScyllaDbBuilder()->withConnectTimeout(5.0)->build();
    $b = persistentScyllaDbBuilder()->withConnectTimeout(7.5)->build();

    expect($a)->toBeInstanceOf(\Cassandra\Cluster::class);
    expect($b)->toBeInstanceOf(\Cassandra\Cluster::class);

    unset($a, $b);
})->group('feature', 'persistent');

it('returns usable sessions on successive connect() calls (cache-hit path)', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();

    $s1 = $cluster->connect($keyspace);
    $s2 = $cluster->connect($keyspace);

    expect($s1->execute(new SimpleStatement('SELECT id FROM cache_log'))->count())->toBe(0);
    expect($s2->execute(new SimpleStatement('SELECT id FROM cache_log'))->count())->toBe(0);

    unset($s1, $s2, $cluster);
})->group('feature', 'persistent');

it('returns distinct usable sessions on connect() to distinct keyspaces', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();

    $s1 = $cluster->connect($keyspace);
    $s2 = $cluster->connect();  // no keyspace — different cache key

    // s1 sees the keyspace, s2 doesn't (must qualify).
    expect($s1->execute(new SimpleStatement('SELECT id FROM cache_log'))->count())->toBe(0);
    expect($s2->execute(new SimpleStatement("SELECT id FROM $keyspace.cache_log"))->count())->toBe(0);

    unset($s1, $s2, $cluster);
})->group('feature', 'persistent');

it('returns usable prepared statements on successive prepare() calls (cache-hit path)', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();
    $session = $cluster->connect($keyspace);

    $cql = 'SELECT payload FROM cache_log WHERE id = ?';
    $p1  = $session->prepare($cql);
    $p2  = $session->prepare($cql);

    expect($session->execute($p1, ['arguments' => [1]])->count())->toBe(0);
    expect($session->execute($p2, ['arguments' => [2]])->count())->toBe(0);

    unset($p1, $p2, $session, $cluster);
})->group('feature', 'persistent');

it('prepares distinct CQL without interfering', function () use ($keyspace) {
    $cluster = persistentScyllaDbBuilder()->build();
    $session = $cluster->connect($keyspace);

    $p1 = $session->prepare('SELECT payload FROM cache_log WHERE id = ?');
    $p2 = $session->prepare('SELECT id FROM cache_log WHERE payload = ? ALLOW FILTERING');

    expect($session->execute($p1, ['arguments' => [1]])->count())->toBe(0);
    expect($session->execute($p2, ['arguments' => ['nope']])->count())->toBe(0);

    unset($p1, $p2, $session, $cluster);
})->group('feature', 'persistent');

it('does not poison the cluster cache when a connect() fails', function () {
    // Cluster built against an unreachable host. The connect() throws, but
    // the cluster's persistent_list entry shouldn't pollute subsequent
    // good-cluster builds.
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

    // A good builder should still produce a working session.
    $good = persistentScyllaDbBuilder()->build();
    $s    = $good->connect();
    expect($s)->not->toBeNull();

    unset($s, $good, $bad);
})->group('feature', 'persistent');

it('keeps a session alive after its originating $cluster is unset (hash_key UAF regression)', function () use ($keyspace) {
    // Regression for the hash_key UAF fixed in #128: session->hash_key
    // used to alias the cluster's hash_key as a raw char*, and dropping
    // $cluster would efree() the buffer out from under the session,
    // dangling on the next prepare()'s spprintf.
    $cluster = persistentScyllaDbBuilder()->build();
    $session = $cluster->connect($keyspace);

    unset($cluster);  // drop the cluster zval — used to dangle session->hash_key

    // Force a prepare(), which spprintfs against the hash_key.
    // Must not crash/segfault.
    $stmt = $session->prepare('SELECT id FROM cache_log WHERE id = ?');
    expect($stmt)->not->toBeNull();

    expect($session->execute($stmt, ['arguments' => [1]])->count())->toBe(0);
})->group('feature', 'persistent');
