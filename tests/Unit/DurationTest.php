<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use BadFunctionCallException;
use Cassandra\Bigint;
use Cassandra\Decimal;
use Cassandra\Duration;
use InvalidArgumentException;
use RangeException;
use RuntimeException;

function spaceship(mixed $left, mixed $right): int
{
    if ($left < $right) {
        return -1;
    } elseif ($left == $right) {
        return 0;
    }
    return 1;
}

describe('Cassandra\Duration', function () {

    it('rejects bool months argument', function () {
        new Duration(true, 2, 3);
    })->throws(InvalidArgumentException::class, 'months must be a long, a double, a numeric string or a Cassandra\\Bigint, 1 given');

    it('rejects bool days argument', function () {
        new Duration(1, true, 3);
    })->throws(InvalidArgumentException::class, 'days must be a long, a double, a numeric string or a Cassandra\\Bigint, 1 given');

    it('rejects bool nanos argument', function () {
        new Duration(1, 2, true);
    })->throws(InvalidArgumentException::class, 'nanos must be a long, a double, a numeric string or a Cassandra\\Bigint, 1 given');

    it('rejects unparseable string nanos', function () {
        new Duration(1, 2, 'ab');
    })->throws(InvalidArgumentException::class, "Invalid integer value: 'ab'");

    it('rejects 64-bit string overflow', function () {
        new Duration(1, 2, '9223372036854775808');
    })->throws(RuntimeException::class, 'value must be between -9223372036854775808 and 9223372036854775807, 9223372036854775808 given');

    it('rejects 64-bit string underflow', function () {
        new Duration(1, 2, '-9223372036854775809');
    })->throws(RangeException::class, 'value must be between -9223372036854775808 and 9223372036854775807, -9223372036854775809 given');

    it('rejects 64-bit double overflow', function () {
        new Duration(1, 2, pow(2, 64));
    })->throws(InvalidArgumentException::class, 'nanos must be between -9223372036854775808 and 9223372036854775807, 1.84467e+19 given');

    it('rejects 64-bit double underflow', function () {
        new Duration(1, 2, -pow(2, 64));
    })->throws(InvalidArgumentException::class, 'nanos must be between -9223372036854775808 and 9223372036854775807, -1.84467e+19 given');

    it('rejects 32-bit string overflow on days', function () {
        new Duration(1, '2147483648', 0);
    })->throws(InvalidArgumentException::class, 'days must be between -2147483648 and 2147483647, 2147483648 given');

    it('rejects 32-bit string underflow on days', function () {
        new Duration(1, '-2147483649', 0);
    })->throws(InvalidArgumentException::class, 'days must be between -2147483648 and 2147483647, -2147483649 given');

    it('rejects 32-bit long overflow on days', function () {
        expect(fn () => new Duration(1, 8589934592, 2))
            ->toThrow(
                InvalidArgumentException::class,
                '/days must be between -2147483648 and 2147483647, 8\\.?58993.* given/'
            );
    })->skip('matches /.../ regex pattern — see PHPUnit assertExceptionMessageMatches');

    it('rejects 32-bit long underflow on days', function () {
        expect(fn () => new Duration(1, -8589934592, 2))
            ->toThrow(
                InvalidArgumentException::class,
                '/days must be between -2147483648 and 2147483647, -8\\.?58993.* given/'
            );
    })->skip('matches /.../ regex pattern — see PHPUnit assertExceptionMessageMatches');

    it('rejects 32-bit double overflow on months', function () {
        new Duration(8589934592.5, 1, 2);
    })->throws(InvalidArgumentException::class, 'months must be between -2147483648 and 2147483647, 8.58993e+9 given');

    it('rejects 32-bit double underflow on months', function () {
        new Duration(-8589934592.5, 1, 2);
    })->throws(InvalidArgumentException::class, 'months must be between -2147483648 and 2147483647, -8.58993e+9 given');

    it('rejects mixed-sign attributes', function ($months, $days, $nanos) {
        new Duration($months, $days, $nanos);
    })->with([
        [1, 2, -3],
        [1, -2, -3],
        [1, -2, 3],
        [-1, 2, 3],
        [-1, 2, -3],
        [-1, -2, 3],
    ])->throws(BadFunctionCallException::class, 'A duration must have all non-negative or non-positive attributes');

    it('formats correctly with __toString', function ($months, $days, $nanos, $expected) {
        $duration = new Duration($months, $days, $nanos);
        expect($duration->__toString())->toEqual($expected);
    })->with([
        [1, 2.5, 3, '1mo2d3ns'],
        [-1.0, -2, -3, '-1mo2d3ns'],
        [0, -2, -3, '-0mo2d3ns'],
    ]);

    it('exposes accessors that mirror the constructor arguments', function ($months, $days, $nanos) {
        $duration = new Duration($months, $days, $nanos);
        expect($duration->months())->toEqual((int) $months)
            ->and($duration->days())->toEqual((int) $days)
            ->and($duration->nanos())->toEqual((string) $nanos)
            ->and($duration->type()->name())->toEqual('duration');
    })->with([
        [1, 2.5, '5'],
        [1, 2, 0],
        [1, 0, 2],
        [0, 1, 3],
        [0, -1, -3],
        [-1, 0, -3],
        [-1, -1, 0],
        [2147483647, 2147483647.0, new Bigint('9223372036854775807')],
        [-2147483648, -2147483648.0, new Bigint('-9223372036854775808')],
        ['2147483647', 0, 0],
        ['-2147483648', 0, 0],
    ]);

    it('compares two Durations consistently', function ($left, $right, $expected) {
        expect(\Cassandra\Tests\Unit\spaceship($left, $right))->toEqual($expected)
            ->and(\Cassandra\Tests\Unit\spaceship($right, $left))->toEqual(-$expected);
    })->with([
        [new Duration(0, 9, 9), new Duration(9, 0, 0), -1],
        [new Duration(9, 0, 9), new Duration(9, 9, 0), -1],
        [new Duration(9, 9, 0), new Duration(9, 9, 9), -1],
        [new Duration(1, 2, 3), new Duration(1, 2, 3), 0],
    ]);

    it('compares Duration against a different type as greater', function () {
        expect(\Cassandra\Tests\Unit\spaceship(new Duration(1, 2, 3), new Decimal('1.2')))->toEqual(1);
    });

    it('exposes its attributes as object properties', function () {
        $duration = new Duration(1, 2, 3);
        $props    = get_object_vars($duration);
        expect($props)->toEqual(['months' => 1, 'days' => 2, 'nanos' => 3]);
    });
});
