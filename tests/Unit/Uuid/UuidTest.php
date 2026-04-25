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

    it('produces unique UUIDs across forked child processes', function () {
        if (!function_exists('pcntl_fork')) {
            $this->markTestSkipped('pcntl_fork() does not exist');
        }

        $script = <<<'EOF'
<?php
$uuidsFilename = $_SERVER['argv'][1];
$numberOfForks = $_SERVER['argv'][2];

$children = [];
foreach (range(1, $numberOfForks) as $i) {
    $pid = pcntl_fork();
    if ($pid < 0) {
        die("Unable to Create Fork: Unique UUID test cannot complete");
    } elseif ($pid === 0) {
        $uuid = new \Cassandra\Uuid();
        file_put_contents($uuidsFilename, $uuid->uuid() . PHP_EOL, FILE_APPEND);
        exit(0);
    } else {
        $children[] = $pid;
    }
}
foreach ($children as $pid) {
    pcntl_waitpid($pid, $status);
}
EOF;
        $numProcesses   = 64;
        $uuidsFilename  = tempnam(sys_get_temp_dir(), 'uuid');
        $scriptFilename = tempnam(sys_get_temp_dir(), 'uuid');
        file_put_contents($scriptFilename, $script, FILE_APPEND);
        exec(PHP_BINARY . " {$scriptFilename} {$uuidsFilename} $numProcesses");
        unlink($scriptFilename);

        $uuids = file($uuidsFilename);
        unlink($uuidsFilename);

        expect(count(array_unique($uuids)))->toEqual($numProcesses);
    });
});
