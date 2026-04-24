<?php

declare(strict_types=1);

use Carbon\Carbon;
use Carbon\CarbonImmutable;
use Cassandra\Date;
use Cassandra\Time;
use Cassandra\Type;
use Cassandra\Exception\InvalidArgumentException;

describe('Cassandra\Date', function () {

    it('creates a date for today when constructed with no arguments', function () {
        $date = new Date();

        expect($date)->toBeInstanceOf(Date::class);
    });

    it('creates a date from a unix timestamp integer', function () {
        $ts   = mktime(0, 0, 0, 6, 15, 2020); // 2020-06-15 00:00:00 UTC
        $date = new Date($ts);

        expect($date)->toBeInstanceOf(Date::class)
            ->and($date->seconds())->toBe($ts);
    });

    it('creates a date from a numeric string', function () {
        $ts   = (string) mktime(0, 0, 0, 1, 1, 2000);
        $date = new Date($ts);

        expect($date)->toBeInstanceOf(Date::class);
    });

    it('throws on a non-numeric string', function () {
        new Date('not-a-timestamp');
    })->throws(InvalidArgumentException::class);

    it('returns seconds at start-of-day for today', function () {
        $expected = Carbon::now()->startOfDay()->getTimestamp();
        $date     = new Date();

        expect($date->seconds())->toBe($expected);
    });

    it('exposes the CQL date type', function () {
        $date = new Date();

        expect($date->type())
            ->toBeInstanceOf(Type::class)
            ->and($date->type()->name())->toBe('date');
    });

    it('converts to a DateTime object', function () {
        $date = new Date();

        expect($date->toDateTime())->toBeInstanceOf(DateTime::class);
    });

    it('converts to a DateTime with an explicit Time argument', function () {
        $date = new Date();
        $time = new Time();

        expect($date->toDateTime($time))->toBeInstanceOf(DateTime::class);
    });

    it('formats correctly with __toString', function () {
        $date     = new Date();
        $expected = 'Cassandra\\Date(seconds=' . CarbonImmutable::now()->startOfDay()->getTimestamp() . ')';

        expect((string) $date)->toBe($expected);
    });

    describe('fromDateTime factory', function () {
        it('accepts a native DateTime', function () {
            expect(Date::fromDateTime(new DateTime()))->toBeInstanceOf(Date::class);
        });

        it('accepts a DateTimeImmutable', function () {
            expect(Date::fromDateTime(new DateTimeImmutable()))->toBeInstanceOf(Date::class);
        });

        it('accepts a Carbon instance', function () {
            expect(Date::fromDateTime(Carbon::now()))->toBeInstanceOf(Date::class);
        });

        it('accepts a CarbonImmutable instance', function () {
            expect(Date::fromDateTime(CarbonImmutable::now()))->toBeInstanceOf(Date::class);
        });
    });
});
