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

describe('Cassandra\Date (migrated)', function () {
    $secondsPerDay = 86400;

    it('constructs from seconds, truncating to whole days', function () use ($secondsPerDay) {
        $date = new Date(0);
        expect($date->seconds())->toEqual(0);

        $date = new Date(1);
        expect($date->seconds())->toEqual(0); // truncated

        $date = new Date($secondsPerDay);
        expect($date->seconds())->toEqual($secondsPerDay);
    });

    it('constructs "now" rounded to start of day', function () use ($secondsPerDay) {
        $date     = new Date();
        $expected = (int) (time() / $secondsPerDay) * $secondsPerDay;
        expect($date->seconds())->toEqualWithDelta($expected, 1);
    });

    it('round-trips DateTime via fromDateTime', function () use ($secondsPerDay) {
        $datetime = new \DateTime('1970-01-01T00:00:00+0000');
        $date     = Date::fromDateTime($datetime);
        expect($date->seconds())->toEqual(0)
            ->and($date->toDateTime())->toEqual($datetime);

        $datetime = new \DateTime('1970-01-02T00:00:00+0000');
        $date     = Date::fromDateTime($datetime);
        expect($date->seconds())->toEqual($secondsPerDay)
            ->and($date->toDateTime())->toEqual($datetime);

        if (version_compare(\Cassandra::CPP_DRIVER_VERSION, '2.4.2') >= 0) {
            $date = Date::fromDateTime(new \DateTime('1969-12-31T00:00:00'));
            expect($date->seconds())->toEqual(-1 * $secondsPerDay);
        }
    });

    it('converts to DateTime with an explicit Time component', function () {
        $datetime = new \DateTime('1970-01-01T00:00:01+0000');
        $date     = Date::fromDateTime($datetime);
        expect($date->seconds())->toEqual(0)
            ->and($date->toDateTime(new Time(1000 * 1000 * 1000)))->toEqual($datetime);
    });

    it('exposes the CQL Type::date()', function () {
        $date = new Date(0);
        expect($date->type())->toEqual(Type::date());
    });

    it('rejects a string with trailing characters', function ($value) {
        new Date($value);
    })->with(['12abc', '5 ', '1.5', '- 3'])
      ->throws(InvalidArgumentException::class);

    it('treats a negative argument as an epoch second, not as a request for today', function () {
        expect((new Date(-86400))->seconds())->toBe(-86400)
            ->and((new Date(-1))->seconds())->toBe(0);
    });

    it('reports the specific reason instead of a generic wrapper', function () {
        expect(fn () => Type::date()->create('12abc'))
            ->toThrow(InvalidArgumentException::class, "Invalid characters were found in value: '12abc'");
    });
});
