<?php

declare(strict_types=1);

use Carbon\Carbon;
use Carbon\CarbonImmutable;
use Cassandra\Timestamp;
use Cassandra\Type;

describe('Cassandra\Timestamp', function () {

    it('creates a timestamp for now when constructed with no arguments', function () {
        expect(new Timestamp())->toBeInstanceOf(Timestamp::class);
    });

    it('exposes the CQL timestamp type', function () {
        expect((new Timestamp())->type())
            ->toBeInstanceOf(Type::class)
            ->and((new Timestamp())->type()->name())->toBe('timestamp');
    });

    it('returns the current unix second via time()', function () {
        $before    = time();
        $timestamp = new Timestamp();
        $after     = time();

        expect($timestamp->time())
            ->toBeGreaterThanOrEqual($before)
            ->toBeLessThanOrEqual($after);
    });

    it('returns a float from microtime(true)', function () {
        $ts = new Timestamp();

        expect($ts->microtime(true))->toBeFloat();
    });

    it('returns a microtime string from microtime(false)', function () {
        $ts = new Timestamp();

        // Expected format: "<fractional> <seconds>"
        expect($ts->microtime(false))->toMatch('/^[\d.]+ \d+$/');
    });

    it('converts to a DateTimeInterface via toDateTime()', function () {
        expect((new Timestamp())->toDateTime())->toBeInstanceOf(DateTimeInterface::class);
    });

    it('formats the millisecond epoch value with __toString', function () {
        $before = (int) (microtime(true) * 1000);
        $ts     = new Timestamp();
        $after  = (int) (microtime(true) * 1000);

        $value = (int) (string) $ts;

        expect($value)
            ->toBeGreaterThanOrEqual($before)
            ->toBeLessThanOrEqual($after);
    });

    describe('fromDateTime factory', function () {
        it('accepts a native DateTime', function () {
            $dt = new DateTime();
            expect(Timestamp::fromDateTime($dt))->toBeInstanceOf(Timestamp::class);
        });

        it('accepts a DateTimeImmutable', function () {
            expect(Timestamp::fromDateTime(new DateTimeImmutable()))->toBeInstanceOf(Timestamp::class);
        });

        it('accepts a Carbon instance', function () {
            expect(Timestamp::fromDateTime(Carbon::now()))->toBeInstanceOf(Timestamp::class);
        });

        it('accepts a CarbonImmutable instance', function () {
            expect(Timestamp::fromDateTime(CarbonImmutable::now()))->toBeInstanceOf(Timestamp::class);
        });

        it('preserves the unix second from a known DateTime', function () {
            $dt = new DateTime('@1700000000');
            $ts = Timestamp::fromDateTime($dt);

            expect($ts->time())->toBe(1700000000);
        });
    });
});
