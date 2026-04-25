<?php

declare(strict_types=1);

use Cassandra\Session;

// ── Group assignments ─────────────────────────────────────────────────────────
uses()->group('unit')->in('tests/Unit');
uses()->group('feature')->in('tests/Feature');

// ── Environment helpers ───────────────────────────────────────────────────────

function env(string $key, mixed $default = null): mixed
{
    $value = getenv($key);
    return $value !== false ? $value : $default;
}

// ── ScyllaDB connection ───────────────────────────────────────────────────────

function scyllaDbConnection(?string $keyspace = null): Session
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
        ->build()
        ->connect(env('SCYLLADB_KEYSPACE', $keyspace ?? ''));
}

// ── Schema helpers ────────────────────────────────────────────────────────────
// Uses the PHP driver directly — no cqlsh binary required.

function migrateKeyspace(string $cql): void
{
    $session = scyllaDbConnection();

    foreach (explode(';', $cql) as $statement) {
        $statement = trim($statement);
        if ($statement !== '') {
            $session->execute($statement);
        }
    }
}

function dropKeyspace(string $keyspace): void
{
    scyllaDbConnection()->execute("DROP KEYSPACE IF EXISTS $keyspace");
}

// ── Custom expectations ───────────────────────────────────────────────────────

expect()->extend('map', function (Closure $fn) {
    return expect($fn($this->value));
});
