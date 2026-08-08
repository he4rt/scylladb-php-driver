<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;

/**
 * cassandra.expose_credentials is PHP_INI_SYSTEM, so the "on" state can only be
 * observed in a fresh process. Returns null when the child cannot load the same
 * extension binary as the parent — set SCYLLADB_EXTENSION_PATH to the built
 * module when it is not in extension_dir.
 */
function childPhpWithIni(string $snippet, array $ini): ?string
{
    $args = ['-n', '-d', 'extension=' . (getenv('SCYLLADB_EXTENSION_PATH') ?: 'cassandra')];
    foreach ($ini as $key => $value) {
        $args[] = '-d';
        $args[] = "$key=$value";
    }
    $args[] = '-r';
    $args[] = $snippet;

    $cmd = implode(' ', array_map('escapeshellarg', [PHP_BINARY, ...$args]));

    $pipes = [];
    $proc  = proc_open($cmd, [1 => ['pipe', 'w'], 2 => ['pipe', 'w']], $pipes);
    if (!is_resource($proc)) {
        return null;
    }

    $stdout = stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    fclose($pipes[2]);

    return proc_close($proc) === 0 ? $stdout : null;
}

describe('Cassandra\Cluster\Builder credentials', function () {

    it('does not expose the password through object properties', function () {
        $builder = Cassandra::cluster()->withCredentials('bob', 'hunter2');

        $props = (array) $builder;

        expect($props)->toHaveKey('password');
        expect($props['password'])->toBe('***');
        expect($props['username'])->toBe('bob');
    });

    it('does not expose the password through print_r or var_export', function () {
        $builder = Cassandra::cluster()->withCredentials('bob', 'hunter2');

        expect(print_r($builder, true))->not->toContain('hunter2');
        expect(var_export($builder, true))->not->toContain('hunter2');
    });

    it('keeps password null when no credentials are set', function () {
        $props = (array) Cassandra::cluster();

        expect($props['username'])->toBeNull();
        expect($props['password'])->toBeNull();
    });

    it('defaults cassandra.expose_credentials to off', function () {
        expect(ini_get('cassandra.expose_credentials'))->toBe('0');
    });

    it('refuses to let ini_set turn the redaction off at runtime', function () {
        expect(@ini_set('cassandra.expose_credentials', '1'))->toBeFalse();

        $props = (array) Cassandra::cluster()->withCredentials('bob', 'hunter2');
        expect($props['password'])->toBe('***');
    });

    it('shows the real password when cassandra.expose_credentials is on', function () {
        $snippet = '$p = (array) Cassandra::cluster()->withCredentials("bob", "hunter2"); echo $p["password"];';

        $redacted = childPhpWithIni($snippet, []);
        if ($redacted === null) {
            $this->markTestSkipped('child PHP cannot load the extension under test');
        }

        expect($redacted)->toBe('***');
        expect(childPhpWithIni($snippet, ['cassandra.expose_credentials' => '1']))->toBe('hunter2');
    });
});
