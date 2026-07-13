<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Support;

/**
 * Centralised environment configuration for the benchmark suite.
 *
 * Uses the same SCYLLADB_* variables as the Pest test-suite helpers so a
 * single docker-compose node serves both. Everything has a sane default so
 * the offline group runs with zero configuration.
 */
final class Env
{
    /** @return list<string> */
    public static function hosts(): array
    {
        $hosts = self::str('SCYLLADB_HOSTS', '127.0.0.1');

        return array_values(array_filter(array_map('trim', explode(',', $hosts))));
    }

    public static function port(): int
    {
        return self::int('SCYLLADB_PORT', 9042);
    }

    public static function username(): string
    {
        return self::str('SCYLLADB_USERNAME', 'cassandra');
    }

    public static function password(): string
    {
        return self::str('SCYLLADB_PASSWORD', 'cassandra');
    }

    /** Keyspace the live benchmarks create (IF NOT EXISTS) and reuse. */
    public static function keyspace(): string
    {
        return self::str('SCYLLADB_BENCH_KEYSPACE', 'bench_scylladb');
    }

    public static function str(string $key, string $default): string
    {
        $value = $_SERVER[$key] ?? $_ENV[$key] ?? getenv($key);

        return $value === false || $value === null || $value === '' ? $default : (string) $value;
    }

    public static function int(string $key, int $default): int
    {
        $value = $_SERVER[$key] ?? $_ENV[$key] ?? getenv($key);

        return $value === false || $value === null || $value === '' ? $default : (int) $value;
    }
}
