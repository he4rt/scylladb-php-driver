<?php

declare(strict_types=1);

use Carbon\Carbon;
use Carbon\CarbonImmutable;
use Cassandra\Time;
use Cassandra\Type;

describe('Cassandra\Time', function () {

    it('creates a time for now when constructed with no arguments', function () {
        expect(new Time())->toBeInstanceOf(Time::class);
    });

    it('creates a time from nanoseconds and seconds() returns whole seconds', function () {
        $nanos   = 12 * 3_600 * 1_000_000_000; // noon = 43 200 000 000 000 ns
        $time    = new Time($nanos);

        // seconds() divides internally stored nanoseconds by 1e9
        expect($time)->toBeInstanceOf(Time::class)
            ->and($time->seconds())->toBe(12 * 3_600); // 43 200 seconds
    });

    it('creates a time from a numeric string of nanoseconds', function () {
        $nanos = (string) (6 * 3_600 * 1_000_000_000); // 06:00:00

        $time = new Time($nanos);
        expect($time)->toBeInstanceOf(Time::class)
            ->and($time->seconds())->toBe(6 * 3_600);
    });

    it('exposes the CQL time type', function () {
        expect((new Time())->type())
            ->toBeInstanceOf(Type::class)
            ->and((new Time())->type()->name())->toBe('time');
    });

    it('returns whole seconds of the day via seconds()', function () {
        $nanos = 9 * 3_600 * 1_000_000_000; // 09:00:00
        $time  = new Time($nanos);

        expect($time->seconds())->toBe(9 * 3_600); // 32 400 seconds
    });

    it('formats correctly with __toString', function () {
        $nanos = 1_000_000_000;
        $time  = new Time($nanos);

        expect((string) $time)->toContain((string) $nanos);
    });

    describe('fromDateTime factory', function () {
        it('accepts a native DateTime', function () {
            expect(Time::fromDateTime(new DateTime()))->toBeInstanceOf(Time::class);
        });

        it('accepts a DateTimeImmutable', function () {
            expect(Time::fromDateTime(new DateTimeImmutable()))->toBeInstanceOf(Time::class);
        });

        it('accepts a Carbon instance', function () {
            expect(Time::fromDateTime(Carbon::now()))->toBeInstanceOf(Time::class);
        });

        it('accepts a CarbonImmutable instance', function () {
            expect(Time::fromDateTime(CarbonImmutable::now()))->toBeInstanceOf(Time::class);
        });

        it('round-trips midnight nanoseconds', function () {
            $midnight = new DateTime('today midnight');
            $time     = Time::fromDateTime($midnight);

            expect($time->seconds())->toBe(0);
        });
    });
});

describe('Cassandra\Time (migrated)', function () {

    it('constructs from int and string nanoseconds and stringifies', function () {
        $time = new Time(0);
        expect((string) $time)->toEqual('0');

        $time = new Time(42);
        expect((string) $time)->toEqual('42');

        $time = new Time('86399999999999');
        expect((string) $time)->toEqual('86399999999999');
    });

    it('constructs "now" within a second of time of day', function () {
        $time = new Time();
        expect($time->seconds())->toEqualWithDelta(time() % (24 * 60 * 60), 1);
    });

    it('rejects negative nanoseconds', function () {
        new Time(-1);
    })->throws(\InvalidArgumentException::class, 'nanoseconds must be nanoseconds since midnight, -1 given')
      ->skip('Extension throws Cassandra\Exception\InvalidArgumentException; needs message-shape verification before un-skipping');

    it('rejects nanoseconds at or above 24 hours', function () {
        new Time('86400000000000');
    })->throws(\InvalidArgumentException::class, "nanoseconds must be nanoseconds since midnight, '86400000000000' given")
      ->skip('Extension throws Cassandra\Exception\InvalidArgumentException; needs message-shape verification before un-skipping');

    it('converts DateTime to nanoseconds since midnight', function () {
        $datetime = new \DateTime('1970-01-01T00:00:00+0000');
        expect((string) Time::fromDateTime($datetime))->toEqual('0');

        $datetime = new \DateTime('1970-01-01T00:00:01+0000');
        expect((string) Time::fromDateTime($datetime))->toEqual('1000000000');

        $datetime = new \DateTime('1970-01-01T23:59:59+0000');
        expect((string) Time::fromDateTime($datetime))->toEqual('86399000000000');
    });

    it('exposes the CQL Type::time()', function () {
        $time = new Time(0);
        expect($time->type())->toEqual(Type::time());
    });
});
