<?php

declare(strict_types=1);

use Cassandra\Session;

/**
 * Test-suite helpers loaded eagerly via composer's autoload-dev.files.
 *
 * Kept separate from Pest.php so the helper functions are available even
 * at composer autoload time (before Pest's bootstrap defines uses() /
 * expect()->extend()). Pest.php contains only Pest-specific registration
 * calls.
 */

if (! function_exists('env')) {
    function env(string $key, mixed $default = null): mixed
    {
        $value = $_SERVER[$key] ?? $_ENV[$key] ?? getenv($key);
        return $value === false || $value === null ? $default : $value;
    }
}

if (! function_exists('scyllaDbConnection')) {
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
}

if (! function_exists('migrateKeyspace')) {
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
}

if (! function_exists('dropKeyspace')) {
    function dropKeyspace(string $keyspace): void
    {
        scyllaDbConnection()->execute("DROP KEYSPACE IF EXISTS $keyspace");
    }
}
