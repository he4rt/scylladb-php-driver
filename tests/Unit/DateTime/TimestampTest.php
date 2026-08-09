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

    describe('constructor with a DateTimeInterface', function () {
        it('accepts a native DateTime', function () {
            expect(new Timestamp(new DateTime('@1700000000')))
                ->toBeInstanceOf(Timestamp::class)
                ->and((new Timestamp(new DateTime('@1700000000')))->time())->toBe(1700000000);
        });

        it('keeps the millisecond part', function () {
            $dt = new DateTimeImmutable('2020-01-02 03:04:05.678 UTC');

            expect((string) new Timestamp($dt))->toBe('1577934245678');
        });

        it('accepts a Carbon instance', function () {
            expect(new Timestamp(CarbonImmutable::parse('@1700000000')))
                ->toBeInstanceOf(Timestamp::class);
        });

        it('matches the fromDateTime factory', function () {
            $dt = new DateTimeImmutable('2021-06-07 08:09:10.123 UTC');

            expect((string) new Timestamp($dt))->toBe((string) Timestamp::fromDateTime($dt));
        });

        it('rejects microseconds together with a DateTimeInterface', function () {
            expect(fn () => new Timestamp(new DateTime(), 5))
                ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
        });

        it('still accepts an integer first argument', function () {
            expect((new Timestamp(1700000000, 500000))->microtime(true))->toBe(1700000000.5);
        });
    });

    describe('now factories', function () {
        it('returns a Timestamp for the current time', function () {
            $before = (int) (microtime(true) * 1000);
            $value  = (int) (string) Timestamp::now();
            $after  = (int) (microtime(true) * 1000);

            expect(Timestamp::now())->toBeInstanceOf(Timestamp::class)
                ->and($value)->toBeGreaterThanOrEqual($before)->toBeLessThanOrEqual($after);
        });

        it('makes nowUtc an alias of now', function () {
            expect(Timestamp::nowUtc())->toBeInstanceOf(Timestamp::class)
                ->and((int) (string) Timestamp::nowUtc() - (int) (string) Timestamp::now())
                ->toBeLessThanOrEqual(1000);
        });
    });
});
