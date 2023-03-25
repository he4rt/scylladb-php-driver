<?php
declare(strict_types=1);

namespace Cassandra\Tests\Feature;

use Cassandra;
use Cassandra\Session;
use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;

final class Utils
{

    static function env(string $key, $default = null)
    {
        $e = getenv($key);

        if ($e !== false) {
            return $e;
        }

        return $default;
    }

    static function dropKeyspace(
        string  $keyspace,
        ?array  $hosts = [],
        ?int    $port = null,
        ?string $username = null,
        ?string $password = null,
    ): void
    {
        $envHosts = self::env('SCYLLADB_HOSTS', $hosts);

        if (is_string($envHosts)) {
            $hosts = explode(',', $envHosts);
        }

        if (empty($hosts)) {
            $hosts = ['127.0.0.1'];
        }

        $process = new Process([
            'cqlsh',
            '-u',
            self::env('SCYLLADB_USERNAME', $username ?? 'cassandra'),
            '-p',
            self::env('SCYLLADB_PASSWORD', $password ?? 'cassandra'),
            '--execute',
            'DROP KEYSPACE IF EXISTS ' . $keyspace,
            $hosts[0],
            (int)self::env('SCYLLADB_PORT', $port ?? 9042),
        ]);

        $process->run();

        if (!$process->isSuccessful()) {
            throw new ProcessFailedException($process);
        }
    }

    static function migrateKeyspace(
        string  $schema,
        ?array  $hosts = [],
        ?int    $port = null,
        ?string $username = null,
        ?string $password = null,
    ): void
    {
        $envHosts = self::env('SCYLLADB_HOSTS', $hosts);

        if (is_string($envHosts)) {
            $hosts = explode(',', $envHosts);
        }

        if (empty($hosts)) {
            $hosts = ['127.0.0.1'];
        }

        $process = new Process([
            self::env('CQLSH_BINARY', 'cqlsh'),
            '-u',
            self::env('SCYLLADB_USERNAME', $username ?? 'cassandra'),
            '-p',
            self::env('SCYLLADB_PASSWORD', $password ?? 'cassandra'),
            '--execute',
            $schema,
            $hosts[0],
            (int)self::env('SCYLLADB_PORT', $port ?? 9042),
        ]);

        $process->run();

        if (!$process->isSuccessful()) {
            throw new ProcessFailedException($process);
        }
    }

    static function scyllaDbConnection(
        ?string $keyspace = null,
        ?array  $hosts = [],
        ?int    $port = null,
        ?string $username = null,
        ?string $password = null
    ): Session
    {
        $envHosts = self::env('SCYLLADB_HOSTS', $hosts);

        if (is_string($envHosts)) {
            $hosts = explode(',', $envHosts);
        }

        if (empty($hosts)) {
            $hosts = ['127.0.0.1'];
        }

        $builder = Cassandra::cluster()
            ->withContactPoints(...$hosts)
            ->withPort((int)self::env('SCYLLADB_PORT', $port ?? 9042))
            ->withCredentials(self::env('SCYLLADB_USERNAME', $username ?? 'cassandra'), self::env('SCYLLADB_USERNAME', $password ?? 'cassandra'))
            ->withPersistentSessions(true)
            ->withTokenAwareRouting(true)
            ->build();

        return $builder->connect(self::env('SCYLLADB_KEYSPACE', $keyspace ?? 'simplex'));
    }

}