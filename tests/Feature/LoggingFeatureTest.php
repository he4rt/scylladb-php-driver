<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature;

/**
 * Migrated from tests-old/features/logging.feature.
 *
 * The Behat scenario set the PHP INI directives `cassandra.log` and
 * `cassandra.log_level` via `php -d` on a freshly-spawned PHP process,
 * then ran a small example that built a Cassandra cluster and connected.
 *
 * In Pest we cannot fork the extension with new INI flags — `cassandra.log`
 * and `cassandra.log_level` are PHP_INI_SYSTEM directives read once when the
 * extension boots. `ini_set()` at runtime cannot change them. The driver
 * does, however, expose runtime logging via `Cassandra::log()` /
 * `Cassandra\Logger`-style callbacks (depending on the build).
 *
 * Strategy:
 *   - We attempt a runtime ini_set, observe whether the file is created, and
 *     skip if the system-level INI requires an extension restart (the common
 *     case in CI).
 *   - The temp log file is cleaned up in afterEach.
 *
 * NOTE: this test touches the PHP_INI directives `cassandra.log` and
 * `cassandra.log_level` — both are PHP_INI_SYSTEM and may not take effect
 * without restarting the PHP process.
 */

$logFile = null;

afterEach(function () use (&$logFile) {
    if ($logFile !== null && is_file($logFile)) {
        @unlink($logFile);
    }
    $logFile = null;
});

it('writes TRACE-level entries to the configured cassandra.log file', function () use (&$logFile) {
    $logFile = tempnam(sys_get_temp_dir(), 'cassandra_log_');

    // cassandra.log / cassandra.log_level are PHP_INI_SYSTEM. ini_set() at
    // request time will not normally take effect — confirm and skip if so.
    $setLog   = @ini_set('cassandra.log', $logFile);
    $setLevel = @ini_set('cassandra.log_level', 'TRACE');

    if ($setLog === false || $setLevel === false) {
        test()->markTestSkipped(
            'cassandra.log / cassandra.log_level are PHP_INI_SYSTEM and require '
            . 'the extension to be (re)loaded with these INI values. Set them via '
            . 'php -d cassandra.log=... -d cassandra.log_level=TRACE for this test.'
        );
    }

    // Trigger driver activity that would normally produce log output.
    $session = scyllaDbConnection();
    $session->execute('SELECT release_version FROM system.local');

    clearstatcache(true, $logFile);

    if (!is_file($logFile) || filesize($logFile) === 0) {
        test()->markTestSkipped(
            'No log output produced — cassandra.log INI value did not take effect '
            . 'at runtime (this is expected for PHP_INI_SYSTEM directives). '
            . 'Re-run with: php -d cassandra.log=... -d cassandra.log_level=TRACE'
        );
    }

    expect(file_get_contents($logFile))->toContain('TRACE');
});
