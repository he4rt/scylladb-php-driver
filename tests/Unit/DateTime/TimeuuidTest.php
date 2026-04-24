<?php

declare(strict_types=1);

use Cassandra\Timeuuid;
use Cassandra\Type;
use Cassandra\Exception\InvalidArgumentException;

describe('Cassandra\Timeuuid', function () {

    it('generates a new UUID v1 when constructed with no arguments', function () {
        expect(new Timeuuid())->toBeInstanceOf(Timeuuid::class);
    });

    it('parses a valid UUID v1 string', function () {
        $uuid = new Timeuuid('550e8400-e29b-11d4-a716-446655440000');

        expect($uuid)->toBeInstanceOf(Timeuuid::class);
    });

    it('throws on a malformed UUID string', function () {
        new Timeuuid('not-a-uuid');
    })->throws(InvalidArgumentException::class);

    it('is always version 1', function () {
        expect((new Timeuuid())->version())->toBe(1);
    });

    it('exposes the CQL timeuuid type', function () {
        $uuid = new Timeuuid();

        expect($uuid->type())
            ->toBeInstanceOf(Type::class)
            ->and($uuid->type()->name())->toBe('timeuuid');
    });

    it('returns a string representation via uuid()', function () {
        $raw  = '550e8400-e29b-11d4-a716-446655440000';
        $uuid = new Timeuuid($raw);

        expect($uuid->uuid())->toBe($raw);
    });

    it('returns unix timestamp seconds via time()', function () {
        $uuid = new Timeuuid();
        $now  = time();

        // Time embedded in the UUID should be within 2 seconds of now
        expect($uuid->time())->toBeGreaterThanOrEqual($now - 2)
            ->toBeLessThanOrEqual($now + 2);
    });

    it('generates unique values across many calls', function () {
        $uuids = array_map(fn () => new Timeuuid(), range(0, 999));
        $strings = array_map(fn (Timeuuid $u) => $u->uuid(), $uuids);

        expect(count(array_unique($strings)))->toBe(1000);
    });

    it('formats correctly with __toString', function () {
        $raw  = '550e8400-e29b-11d4-a716-446655440000';
        $uuid = new Timeuuid($raw);

        expect((string) $uuid)->toBe($raw);
    });
});
