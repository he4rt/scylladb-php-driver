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
});
