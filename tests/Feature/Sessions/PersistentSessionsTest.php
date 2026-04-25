<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Sessions;

/*
 * Migrated from tests-old/features/sessions/persistent_sessions.feature
 *
 * The original Behat feature relied on starting a PHP built-in web server,
 * making multiple HTTP requests, and asserting against `phpinfo()`'s
 * "Persistent Clusters / Persistent Sessions" rows. That cross-process state
 * cannot be observed inside a single Pest run (each test invocation is a fresh
 * SAPI lifecycle, so the persistent-session pool always reports zero from the
 * outside). All four scenarios are preserved as skipped tests so the inventory
 * stays accurate for when an out-of-process harness exists again.
 */

it('reports zero persistent clusters and sessions when none have been created', function () {
    // Original: spawn `php -S`, fetch /status.php, parse phpinfo() Cassandra
    // module table, assert "Persistent Clusters | 0" and "Persistent Sessions | 0".
})->skip('Requires a separate web-server process to inspect phpinfo() persistent-session counters.');

it('re-uses a single persistent session across multiple requests', function () {
    // Original: hit /connect.php (which builds a cluster with
    // ->withPersistentSessions(true) and connects) 5 times, then /status.php.
    // Expects exactly 1 persistent cluster + 1 persistent session.
})->skip('Requires a separate web-server process; persistent-session pool is per-SAPI.');

it('keeps separate persistent sessions per keyspace', function () {
    // Original: connect.php -> connect(); connect_system.php -> connect("system");
    // expects 1 persistent cluster, 2 persistent sessions.
})->skip('Requires a separate web-server process; persistent-session pool is per-SAPI.');

it('does not retain non-persistent sessions across requests', function () {
    // Original: build cluster with ->withPersistentSessions(false), connect,
    // expect "Persistent Clusters | 0", "Persistent Sessions | 0".
})->skip('Requires a separate web-server process; persistent-session pool is per-SAPI.');
