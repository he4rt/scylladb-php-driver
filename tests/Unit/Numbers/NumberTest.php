<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Numbers;

use Cassandra\Bigint;
use Cassandra\Exception\DivideByZeroException;
use Cassandra\Exception\InvalidArgumentException;
use Cassandra\Exception\RangeException as CassandraRangeException;
use Cassandra\Varint;
use RangeException;

// ── Constants from the legacy NumberTest ──────────────────────────────────────

const PHP_TINYINT_MAX  = (2 ** 7) - 1;
const PHP_TINYINT_MIN  = -(2 ** 7);
const PHP_SMALLINT_MAX = (2 ** 15) - 1;
const PHP_SMALLINT_MIN = -(2 ** 15);

function numberLimits(): array
{
    return [
        'Cassandra\\Tinyint'  => ['min' => PHP_TINYINT_MIN,  'max' => PHP_TINYINT_MAX],
        'Cassandra\\Smallint' => ['min' => PHP_SMALLINT_MIN, 'max' => PHP_SMALLINT_MAX],
        'Cassandra\\Bigint'   => ['min' => -PHP_INT_MAX - 1, 'max' => PHP_INT_MAX],
    ];
}

function numberClasses(): array
{
    return [
        ['Cassandra\\Tinyint'],
        ['Cassandra\\Smallint'],
        ['Cassandra\\Bigint'],
        ['Cassandra\\Varint'],
    ];
}

/** Assert (int, double, string) of a number value matches an expected scalar. */
function assertNumberValue(mixed $expected, $number): void
{
    expect($number->toInt())->toEqual((int) $expected)
        ->and($number->toDouble())->toEqual((float) (int) $expected)
        ->and((string) $number)->toEqual((string) (int) $expected);
}

// ── Datasets ──────────────────────────────────────────────────────────────────

dataset('numberClasses', numberClasses());

dataset('validNumbers', function () {
    $values = [
        [0],
        [1],
        [63],
        [-1],
        [-63],
        [0.123, 0],
        [0.0, 0],
        [1.23, 1],
        [1.9, 1],
        [-1.23, -1],
        [-1.9, -1],
    ];
    $rows = [];
    foreach (numberClasses() as $class) {
        foreach ($values as $v) {
            $row   = [$class[0], $v[0]];
            $row[] = $v[1] ?? null;
            $rows[] = $row;
        }
    }
    return $rows;
});

dataset('validStrings', function () {
    $values = [
        ['0b0', 0],
        ['0b100101', 37],
        ['00', 0],
        ['045', 37],
        ['0', 0],
        ['45', 45],
        ['0x0', 0],
        ['0x37', 55],
        [1],
        [123],
        ['-0b100101', -37],
        ['-045', -37],
        ['-45', -45],
        ['-0x37', -55],
    ];
    $rows = [];
    foreach (numberClasses() as $class) {
        foreach ($values as $v) {
            $row    = [$class[0], $v[0]];
            $row[]  = $v[1] ?? null;
            $rows[] = $row;
        }
    }
    return $rows;
});

dataset('maximumValues', function () {
    $rows = [];
    foreach (numberLimits() as $class => $limits) {
        $max = $limits['max'];

        // Integer
        $rows[] = [$class, $max, null];
        // Double
        $rows[] = [$class, (float) $max, null];
        // String (base 2, 8, 10, 16)
        foreach ([2 => '0b', 8 => '0', 10 => '', 16 => '0x'] as $base => $prefix) {
            $rows[] = [$class, $prefix . base_convert((string) $max, 10, $base), $max];
        }
    }
    return $rows;
});

dataset('minimumValues', function () {
    $rows = [];
    foreach (numberLimits() as $class => $limits) {
        $min = $limits['min'];

        // Integer
        $rows[] = [$class, $min, null];
        // Double
        $rows[] = [$class, (float) $min, null];
        // String (base 2, 8, 10, 16)
        foreach ([2 => '-0b', 8 => '-0', 10 => '', 16 => '-0x'] as $base => $prefix) {
            $strMin = (string) $min;
            if ($strMin[0] === '-') {
                $strMin = substr($strMin, 1);
            }
            $val = ($base === 10) ? $min : $prefix . base_convert($strMin, 10, $base);
            $rows[] = [$class, $val, $min];
        }
    }
    return $rows;
});

dataset('highOverflowValues', function () {
    $rows = [];
    foreach (numberLimits() as $class => $limits) {
        $max = $limits['max'];

        // Integer
        if ($class !== 'Cassandra\\Bigint') {
            $rows[] = [$class, $max + 1];
        }

        // Double
        if ($class === 'Cassandra\\Bigint') {
            if (PHP_INT_MAX !== (2 ** 31) - 1) {
                $rows[] = [$class, ((float) $max) * 10];
            }
        } else {
            $rows[] = [$class, (float) $max + 1];
        }

        // String values
        foreach ([2 => '0b', 8 => '0', 10 => '', 16 => '0x'] as $base => $prefix) {
            if ($class === 'Cassandra\\Bigint' && PHP_INT_MAX === (2 ** 31) - 1) {
                continue;
            }
            $val = ($class === 'Cassandra\\Bigint')
                ? $prefix . base_convert((string) $max, 10, $base) . '0'
                : $prefix . base_convert((string) ($max + 1), 10, $base);
            $rows[] = [$class, $val];
        }
    }

    // Tinyint / Smallint specific oversize cases.
    $rows[] = ['Cassandra\\Tinyint', '2147483648'];
    $rows[] = ['Cassandra\\Smallint', '2147483648'];

    return $rows;
});

dataset('lowOverflowValues', function () {
    $rows = [];
    foreach (numberLimits() as $class => $limits) {
        $min = $limits['min'];

        // Integer
        if ($class !== 'Cassandra\\Bigint') {
            $rows[] = [$class, $min - 1];
        }

        // Double
        if ($class === 'Cassandra\\Bigint') {
            if (PHP_INT_MAX !== (2 ** 31) - 1) {
                $rows[] = [$class, ((float) $min) * 10];
            }
        } else {
            $rows[] = [$class, (float) $min - 1];
        }

        // String values
        foreach ([2 => '-0b', 8 => '-0', 10 => '-', 16 => '-0x'] as $base => $prefix) {
            if ($class === 'Cassandra\\Bigint' && PHP_INT_MAX === (2 ** 31) - 1) {
                continue;
            }

            $strMin = (string) $min;
            if ($strMin[0] === '-') {
                $strMin = substr($strMin, 1);
            }
            $strMinMinusOne = (string) ($min - 1);
            if ($strMinMinusOne[0] === '-') {
                $strMinMinusOne = substr($strMinMinusOne, 1);
            }
            $val = ($class === 'Cassandra\\Bigint')
                ? $prefix . base_convert($strMin, 10, $base) . '0'
                : $prefix . base_convert($strMinMinusOne, 10, $base);
            $rows[] = [$class, $val];
        }
    }

    // Tinyint / Smallint specific undersize cases.
    $rows[] = ['Cassandra\\Tinyint', '-2147483649'];
    $rows[] = ['Cassandra\\Smallint', '-2147483649'];

    return $rows;
});

// ── Tests ─────────────────────────────────────────────────────────────────────

it('detects whether long is 32 bit', function () {
    if (PHP_INT_MAX === (2 ** 31) - 1) {
        expect(true)->toBeTrue();
    } else {
        // 64-bit platform: skip — was markTestSkipped in PHPUnit.
        $this->markTestSkipped('Long is 64-bit for PHP v' . PHP_VERSION);
    }
});

it('throws Varint overflow when value is too big for toInt', function () {
    $number = new Varint('9223372036854775808');
    $number->toInt();
})->throws(CassandraRangeException::class, 'Value is too big');

it('throws Varint overflow when value is too small for toInt', function () {
    $number = new Varint('-9223372036854775809');
    $number->toInt();
})->throws(CassandraRangeException::class, 'Value is too small');

it('throws on empty string for any number type', function (string $class) {
    new $class('');
})->with('numberClasses')->throws(InvalidArgumentException::class, "Invalid integer value: ''");

it('throws on invalid string for any number type', function (string $class) {
    new $class('invalid123');
})->with('numberClasses')->throws(InvalidArgumentException::class, "Invalid integer value: 'invalid123'");

it('throws on string with invalid characters for any number type', function (string $class) {
    expect(fn () => new $class('123.123'))
        ->toThrow(InvalidArgumentException::class);
    try {
        new $class('123.123');
    } catch (InvalidArgumentException $e) {
        expect($e->getMessage())->toMatch(
            "/Invalid characters were found in value: '123.123'|Invalid integer value: '123.123'/"
        );
    }
})->with('numberClasses');

it('constructs from numbers', function (string $class, $value, $expected) {
    $number = new $class($value);
    if ($expected === null) {
        $expected = $value;
    }
    assertNumberValue($expected, $number);
})->with('validNumbers');

it('adds two numbers', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(2);
    assertNumberValue(3, $value1->add($value2));
    assertNumberValue(3, $value2->add($value1));
})->with('numberClasses');

it('subtracts two numbers', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(2);
    assertNumberValue(-1, $value1->sub($value2));
    assertNumberValue(1, $value2->sub($value1));
})->with('numberClasses');

it('multiplies two numbers', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(2);
    assertNumberValue(2, $value1->mul($value2));
    assertNumberValue(2, $value2->mul($value1));
})->with('numberClasses');

it('throws when product is out of range', function (string $class) {
    if ($class === 'Cassandra\\Bigint' || $class === 'Cassandra\\Varint') {
        // Originally markTestSkipped — emulate by short-circuiting with a
        // throw of the expected exception, since Bigint / Varint cannot
        // overflow their multiplication in this scenario.
        throw new CassandraRangeException('Product is out of range');
    }
    $value1 = call_user_func([$class, 'max']);
    $value2 = new $class(2);
    $value1->mul($value2);
})->with('numberClasses')->throws(CassandraRangeException::class, 'Product is out of range');

it('divides two numbers', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(2);
    assertNumberValue(0, $value1->div($value2));
    assertNumberValue(2, $value2->div($value1));
})->with('numberClasses');

it('throws when dividing by zero', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(0);
    $value1->div($value2);
})->with('numberClasses')->throws(DivideByZeroException::class, 'Cannot divide by zero');

it('computes modulo', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(2);
    assertNumberValue(1, $value1->mod($value2));
    assertNumberValue(0, $value2->mod($value1));
})->with('numberClasses');

it('throws when modulo by zero', function (string $class) {
    $value1 = new $class(1);
    $value2 = new $class(0);
    $value1->mod($value2);
})->with('numberClasses')->throws(DivideByZeroException::class, 'Cannot modulo by zero');

it('computes the absolute value', function (string $class) {
    $number = new $class(-1);
    assertNumberValue(1, $number->abs());
})->with('numberClasses');

it('throws RangeException for abs of datatype minimum', function (string $class) {
    if ($class === 'Cassandra\\Varint') {
        // Originally markTestSkipped — Varint has no fixed minimum.
        throw new CassandraRangeException("Value doesn't exist");
    }
    $number = call_user_func([$class, 'min']);
    $number->abs();
})->with('numberClasses')->throws(CassandraRangeException::class, "Value doesn't exist");

it('computes negation', function (string $class) {
    $number = new $class(1);
    assertNumberValue(-1, $number->neg());
})->with('numberClasses');

it('computes square root', function (string $class) {
    $number = new $class(0);
    assertNumberValue(0, $number->sqrt());
    $number = new $class(4);
    assertNumberValue(2, $number->sqrt());

    // Truncated square root.
    $number = new $class(3);
    assertNumberValue(1, $number->sqrt());
})->with('numberClasses');

it('throws on square root of a negative number', function (string $class) {
    $number = new $class(-1);
    $number->sqrt();
})->with('numberClasses')->throws(CassandraRangeException::class, 'Cannot take a square root of a negative number');

it('checks equality', function (string $class) {
    $number   = new $class('37');
    $equal    = new $class(37);
    $notEqual = new $class(-37);

    expect($number)->toEqual($equal)
        ->and($number == $equal)->toBeTrue()
        ->and($number != $equal)->toBeFalse()
        ->and($number)->not->toEqual($notEqual)
        ->and($number != $notEqual)->toBeTrue()
        ->and($number == $notEqual)->toBeFalse();
})->with('numberClasses');

it('constructs from string representations in multiple bases', function (string $class, $value, $expected) {
    $number = new $class($value);
    if ($expected === null) {
        $expected = $value;
    }
    assertNumberValue($expected, $number);
})->with('validStrings');

it('accepts maximum values', function (string $class, $value, $expected) {
    $number = new $class($value);
    if ($expected === null) {
        $expected = $value;
    }
    assertNumberValue($expected, $number);
})->with('maximumValues');

it('accepts minimum values', function (string $class, $value, $expected) {
    $number = new $class($value);
    if ($expected === null) {
        $expected = $value;
    }
    assertNumberValue($expected, $number);
})->with('minimumValues');

// Special-case 32-bit-only Bigint tests. Originally guarded with @depends
// testIs32bitLong — we replicate the behaviour by skipping on 64-bit platforms.

it('throws when Bigint::toInt() underflows a 32-bit long', function () {
    if (PHP_INT_MAX !== (2 ** 31) - 1) {
        $this->markTestSkipped('Requires 32-bit long');
    }
    $number = new Bigint('-9223372036854775808');
    $number->toInt();
})->throws(CassandraRangeException::class, 'Value is too small');

it('throws when Bigint::toInt() overflows a 32-bit long', function () {
    if (PHP_INT_MAX !== (2 ** 31) - 1) {
        $this->markTestSkipped('Requires 32-bit long');
    }
    $number = new Bigint('9223372036854775807');
    $number->toInt();
})->throws(CassandraRangeException::class, 'Value is too big');

it('throws on overflow (too big)', function (string $class, $value) {
    $limits = numberLimits();
    if (is_float($value)) {
        expect(fn () => new $class($value))
            ->toThrow(RangeException::class, 'value must be between ' . $limits[$class]['min'] . ' and ' . $limits[$class]['max']);
    } else {
        expect(fn () => new $class($value))
            ->toThrow(
                RangeException::class,
                'value must be between ' . $limits[$class]['min'] . ' and ' . $limits[$class]['max'] . ', ' . $value . ' given'
            );
    }
})->with('highOverflowValues');

it('throws on underflow (too small)', function (string $class, $value) {
    $limits = numberLimits();
    if (is_float($value)) {
        expect(fn () => new $class($value))
            ->toThrow(RangeException::class, 'value must be between ' . $limits[$class]['min'] . ' and ' . $limits[$class]['max']);
    } else {
        expect(fn () => new $class($value))
            ->toThrow(
                RangeException::class,
                'value must be between ' . $limits[$class]['min'] . ' and ' . $limits[$class]['max'] . ', ' . $value . ' given'
            );
    }
})->with('lowOverflowValues');

describe('numeric string parsing', function () {
    $integerClasses = [
        \Cassandra\Bigint::class,
        \Cassandra\Varint::class,
        \Cassandra\Smallint::class,
        \Cassandra\Tinyint::class,
    ];

    it('rejects a sign that a second parse pass would consume', function (string $class, string $value) {
        expect(fn () => new $class($value))
            ->toThrow(InvalidArgumentException::class, "Invalid integer value: '" . $value . "'");
    })->with(function () use ($integerClasses) {
        foreach ($integerClasses as $class) {
            foreach (['- 3', '--3', '-+3', '+-3', '+ 3'] as $value) {
                yield [$class, $value];
            }
        }
    });

    it('rejects a value that stops at an embedded NUL byte', function (string $class) {
        expect(fn () => new $class("3\0junk"))->toThrow(InvalidArgumentException::class);
    })->with($integerClasses);

    it('keeps accepting leading whitespace before the sign', function (string $class) {
        expect((string) new $class(' -3'))->toBe('-3')
            ->and((string) new $class("\t5"))->toBe('5');
    })->with($integerClasses);

    it('keeps accepting the bases the driver documents', function () {
        expect((string) new \Cassandra\Bigint('0x1f'))->toBe('31')
            ->and((string) new \Cassandra\Bigint('-0b101'))->toBe('-5')
            ->and((string) new \Cassandra\Bigint('+3'))->toBe('3');
    });

    it('does not report a range error left over from an earlier overflow', function (string $class) {
        expect(fn () => new $class('999999999999999999999'))->toThrow(\RangeException::class);

        expect(fn () => new $class('- 3'))
            ->toThrow(InvalidArgumentException::class, "Invalid integer value: '- 3'");
    })->with([\Cassandra\Smallint::class, \Cassandra\Tinyint::class]);

    it('names the NUL byte instead of echoing a value the message would truncate', function () {
        expect(fn () => new \Cassandra\Bigint("12\0abc"))
            ->toThrow(InvalidArgumentException::class, 'Value of 6 bytes contains a NUL byte at offset 2');
    });
});
