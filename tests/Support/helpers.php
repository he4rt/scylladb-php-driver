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

        $cluster = Cassandra::cluster()
            ->withContactPoints(...$hosts)
            ->withPort((int) env('SCYLLADB_PORT', 9042))
            ->withCredentials(
                env('SCYLLADB_USERNAME', 'cassandra'),
                env('SCYLLADB_PASSWORD', 'cassandra'),
            )
            ->withPersistentSessions(false)
            ->withTokenAwareRouting(true)
            ->build();

        // cpp-rs-driver rejects empty keyspace strings in `USE KEYSPACE`;
        // the legacy cpp-driver tolerated them. Only pass a keyspace if we
        // actually have one to use.
        $ks = (string) env('SCYLLADB_KEYSPACE', $keyspace ?? '');
        return $ks !== '' ? $cluster->connect($ks) : $cluster->connect();
    }
}

if (! function_exists('migrateKeyspace')) {
    /**
     * Runs the setup CQL of a test suite.
     *
     * Every suite creates its keyspace with a plain CREATE KEYSPACE, so a run
     * that dies before its afterAll() leaves the keyspace behind and the next
     * run fails with "Cannot add existing keyspace". That exception is thrown
     * from beforeAll(), which PHPUnit counts as an error: the shell exit code
     * becomes 2 while the summary still reports every test as passed. See
     * tests/Support/HookErrorReporter.php.
     *
     * Dropping the keyspace first makes each suite start from a known state,
     * whatever the run before it did.
     */
    function migrateKeyspace(string $cql): void
    {
        $session = scyllaDbConnection();

        if (preg_match('/CREATE\s+KEYSPACE\s+(?:IF\s+NOT\s+EXISTS\s+)?"?(\w+)"?/i', $cql, $m) === 1) {
            $session->execute("DROP KEYSPACE IF EXISTS {$m[1]}");
        }

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

if (! function_exists('driverBackend')) {
    /**
     * Returns the underlying C/C++ driver backend the extension was built
     * against: 'cassandra', 'scylla-cpp', or 'scylla-rust'. Driven by the
     * SCYLLADB_PHP_BACKEND env var set by CI; defaults to 'scylla-cpp'.
     */
    function driverBackend(): string
    {
        return (string) env('SCYLLADB_PHP_BACKEND', 'scylla-cpp');
    }
}

if (! function_exists('isScyllaRustBackend')) {
    function isScyllaRustBackend(): bool
    {
        return driverBackend() === 'scylla-rust';
    }
}

if (! function_exists('persistentScyllaDbBuilder')) {
    /**
     * Builder configured for persistent sessions, contact points / port /
     * credentials matching scyllaDbConnection(). Use to exercise the
     * EG(persistent_list) caching path: build()->connect() reuses the
     * cached CassCluster/CassSession across calls within the same process.
     */
    function persistentScyllaDbBuilder(): \Cassandra\Cluster\Builder
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
            ->withPersistentSessions(true)
            ->withTokenAwareRouting(true);
    }
}


if (! function_exists('phpWithIni')) {
    /**
     * Runs a snippet in a fresh PHP process with php.ini overrides.
     *
     * Every driver directive is PHP_INI_SYSTEM, so a child process is the only
     * way to observe a non-default value. Returns null when the child cannot
     * load the same extension binary as the parent — set SCYLLADB_EXTENSION_PATH
     * to the built module when it is not in extension_dir.
     *
     * `payload` holds only what the snippet echoed. The snippet runs inside an
     * output buffer whose contents go to a temp file, because the C driver
     * writes a banner straight to file descriptor 1 and would otherwise
     * interleave with it. `payload` is null when the snippet died early.
     *
     * @param array<string, scalar> $ini
     * @return array{stdout: string, payload: ?string, stderr: string, exit: int}|null
     */
    function phpWithIni(string $snippet, array $ini = []): ?array
    {
        /* display_errors=stderr keeps warnings off stdout so a test can assert
         * on them without the snippet output in the way. */
        $args = [
            '-n',
            '-d', 'extension=' . (env('SCYLLADB_EXTENSION_PATH') ?: 'cassandra'),
            '-d', 'display_errors=stderr',
        ];

        foreach ($ini as $key => $value) {
            $args[] = '-d';
            $args[] = $key . '=' . $value;
        }

        $payloadFile = tempnam(sys_get_temp_dir(), 'scylladb-ini-');
        if ($payloadFile === false) {
            return null;
        }

        $args[] = '-r';
        $args[] = 'ob_start(); ' . $snippet
            . ' ; file_put_contents(' . var_export($payloadFile, true) . ', ob_get_clean());';

        $cmd = implode(' ', array_map('escapeshellarg', [PHP_BINARY, ...$args]));

        $pipes = [];
        $proc  = proc_open($cmd, [1 => ['pipe', 'w'], 2 => ['pipe', 'w']], $pipes);
        if (! is_resource($proc)) {
            unlink($payloadFile);
            return null;
        }

        $stdout = stream_get_contents($pipes[1]);
        $stderr = stream_get_contents($pipes[2]);
        fclose($pipes[1]);
        fclose($pipes[2]);
        $exit = proc_close($proc);

        $payload = file_get_contents($payloadFile);
        unlink($payloadFile);

        return [
            'stdout'  => $stdout,
            'payload' => $payload === false || $payload === '' ? null : $payload,
            'stderr'  => $stderr,
            'exit'    => $exit,
        ];
    }
}
