<?php

declare(strict_types=1);

use Cassandra\Uuid;
use Cassandra\Exception\InvalidArgumentException;

describe('Cassandra\Uuid', function () {

    it('generates a new UUID v4 when constructed with no arguments', function () {
        expect(new Uuid())->toBeInstanceOf(Uuid::class);
    });

    it('parses a valid UUID string', function () {
        $uuid = new Uuid('2a5072fa-7da4-4ccd-a9b4-f017a3872304');

        expect($uuid)->toBeInstanceOf(Uuid::class);
    });

    it('throws on an invalid UUID string', function () {
        new Uuid('not-an-uuid-btw');
    })->throws(InvalidArgumentException::class, "Invalid UUID: 'not-an-uuid-btw'");

    it('produces equal UUIDs for the same input string', function () {
        $raw   = '2a5072fa-7da4-4ccd-a9b4-f017a3872304';
        $uuid1 = new Uuid($raw);
        $uuid2 = new Uuid($raw);

        expect($uuid1)->toEqual($uuid2)
            ->and($uuid1 == $uuid2)->toBeTrue();
    });

    it('produces unequal UUIDs for different input strings', function () {
        $uuid1 = new Uuid('2a5072fa-7da4-4ccd-a9b4-f017a3872304');
        $uuid2 = new Uuid('3b5072fa-7da4-4ccd-a9b4-f017a3872304');

        expect($uuid1)->not->toEqual($uuid2)
            ->and($uuid1 == $uuid2)->toBeFalse();
    });

    it('generates unique UUIDs on every construction', function () {
        $uuids = array_map(fn () => (string) new Uuid(), range(0, 999));

        expect(count(array_unique($uuids)))->toBe(1000);
    });

    it('returns the string representation via uuid()', function () {
        $raw  = '2a5072fa-7da4-4ccd-a9b4-f017a3872304';
        $uuid = new Uuid($raw);

        expect($uuid->uuid())->toBe($raw);
    });

    it('returns the string representation via __toString', function () {
        $raw  = '2a5072fa-7da4-4ccd-a9b4-f017a3872304';
        $uuid = new Uuid($raw);

        expect((string) $uuid)->toBe($raw);
    });

    it('generates unique uuids over 10000 iterations', function () {
        for ($i = 0; $i < 10000; $i++) {
            expect((string) new Uuid())->not->toEqual((string) new Uuid());
        }
    });

    it('compares two equal Uuids', function (Uuid $value1, Uuid $value2) {
        expect($value2)->toEqual($value1)
            ->and($value1 == $value2)->toBeTrue();
    })->with([
        [new Uuid('2a5072fa-7da4-4ccd-a9b4-f017a3872304'), new Uuid('2a5072fa-7da4-4ccd-a9b4-f017a3872304')],
    ]);

    it('compares two non-equal Uuids', function (Uuid $value1, Uuid $value2) {
        expect($value2)->not->toEqual($value1)
            ->and($value1 == $value2)->toBeFalse();
    })->with([
        [new Uuid('2a5072fa-7da4-4ccd-a9b4-f017a3872304'), new Uuid('3b5072fa-7da4-4ccd-a9b4-f017a3872304')],
    ]);

    it('produces unique UUIDs across separate processes', function () {
        // Use child PHP processes rather than pcntl_fork so this test runs on
        // ZTS builds and platforms without pcntl. Each child generates one
        // UUID and prints it; the parent collects stdout and checks uniqueness.
        $numProcesses = 64;
        $snippet      = 'echo (new \Cassandra\Uuid())->uuid(), "\n";';
        $cmd          = sprintf(
            '%s -d extension=cassandra -r %s',
            escapeshellarg(PHP_BINARY),
            escapeshellarg($snippet),
        );

        $procs = [];
        foreach (range(1, $numProcesses) as $i) {
            $pipes = [];
            $proc  = proc_open($cmd, [1 => ['pipe', 'w'], 2 => ['pipe', 'w']], $pipes);
            if (!is_resource($proc)) {
                foreach ($procs as $p) { proc_close($p['proc']); }
                $this->fail('Unable to spawn PHP subprocess: unique UUID test cannot complete');
            }
            $procs[] = ['proc' => $proc, 'pipes' => $pipes];
        }

        $uuids = [];
        foreach ($procs as $p) {
            $stdout = stream_get_contents($p['pipes'][1]);
            $stderr = stream_get_contents($p['pipes'][2]);
            fclose($p['pipes'][1]);
            fclose($p['pipes'][2]);
            $rc = proc_close($p['proc']);
            if ($rc !== 0) {
                $this->fail("UUID child exited with $rc: $stderr");
            }
            $uuids[] = trim($stdout);
        }

        expect(count(array_unique($uuids)))->toEqual($numProcesses);
    });

    it('rejects a UUID that would parse only up to an embedded NUL byte', function () {
        expect(fn () => new Uuid("65f9e722-036a-4029-b03b-a9046b23b4c9\0junk"))
            ->toThrow(InvalidArgumentException::class);
    });
});
