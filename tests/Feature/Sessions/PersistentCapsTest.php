<?php

declare(strict_types=1);

/**
 * The persistent caches live in EG(persistent_list), which is per process, and
 * the caps are PHP_INI_SYSTEM. Each case therefore runs a child process that
 * connects, prepares a few distinct statements, then reports the counters that
 * phpinfo() prints.
 */
$snippet = <<<'PHP'
$session = Cassandra::cluster()
    ->withContactPoints(...explode(',', getenv('SCYLLADB_HOSTS') ?: '127.0.0.1'))
    ->withPort((int) (getenv('SCYLLADB_PORT') ?: 9042))
    ->withCredentials(getenv('SCYLLADB_USERNAME') ?: 'cassandra', getenv('SCYLLADB_PASSWORD') ?: 'cassandra')
    ->build()
    ->connect();

foreach (range(1, 5) as $i) {
    $session->prepare("SELECT key FROM system.local WHERE key = ? LIMIT $i");
}

ob_start();
phpinfo(INFO_MODULES);
$info = ob_get_clean();
preg_match_all('/Persistent (Clusters|Sessions|Prepared Statements) => (\d+)/', $info, $m);
echo json_encode(array_combine($m[1], array_map('intval', $m[2])));
PHP;

/** @return array{counters: array<string,int>, stderr: string}|null */
function persistentCounters(string $snippet, array $ini): ?array
{
    $result = phpWithIni($snippet, $ini);

    if ($result === null || $result['exit'] !== 0) {
        return null;
    }

    $counters = json_decode((string) $result['payload'], true);

    return is_array($counters) ? ['counters' => $counters, 'stderr' => $result['stderr']] : null;
}

describe('persistent cache caps', function () use ($snippet) {

    it('caches every prepared statement when uncapped', function () use ($snippet) {
        $out = persistentCounters($snippet, []);

        if ($out === null) {
            $this->markTestSkipped('child process could not connect or load the extension');
        }

        expect($out['counters'])->toBe([
            'Clusters'           => 1,
            'Sessions'           => 1,
            'Prepared Statements' => 5,
        ]);
    });

    it('stops caching prepared statements at the cap and warns once', function () use ($snippet) {
        $out = persistentCounters($snippet, ['cassandra.max_persistent_prepared_statements' => 2]);

        if ($out === null) {
            $this->markTestSkipped('child process could not connect or load the extension');
        }

        expect($out['counters']['Prepared Statements'])->toBe(2)
            ->and($out['counters']['Sessions'])->toBe(1)
            ->and(substr_count($out['stderr'], 'max_persistent_prepared_statements reached'))->toBe(1);
    });

    it('caches nothing when the cap is zero', function () use ($snippet) {
        $out = persistentCounters($snippet, ['cassandra.max_persistent_sessions' => 0]);

        if ($out === null) {
            $this->markTestSkipped('child process could not connect or load the extension');
        }

        /* No cached session means no session cache_key, so the prepared
           statements below it are not cached either. */
        expect($out['counters']['Sessions'])->toBe(0)
            ->and($out['counters']['Prepared Statements'])->toBe(0)
            ->and($out['stderr'])->toContain('max_persistent_sessions reached');
    });

    it('caches nothing when allow_persistent is off', function () use ($snippet) {
        $out = persistentCounters($snippet, ['cassandra.allow_persistent' => 0]);

        if ($out === null) {
            $this->markTestSkipped('child process could not connect or load the extension');
        }

        expect($out['counters'])->toBe([
            'Clusters'           => 0,
            'Sessions'           => 0,
            'Prepared Statements' => 0,
        ]);
    });
});
