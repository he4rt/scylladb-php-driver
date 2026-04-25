<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Numbers;

use Cassandra\Exception\DivideByZeroException;
use InvalidArgumentException;
use RangeException;

// "Float" is a reserved word in modern PHP, so we resolve the class by its
// fully-qualified string name to avoid a parse error.
const FLOAT_CLASS = '\\Cassandra\\Float';

const EPSILON = 0.00001;

it('throws when creating from empty string', function () {
    $cls = FLOAT_CLASS;
    new $cls('');
})->throws(InvalidArgumentException::class, "Invalid float value: ''");

it('throws when creating from invalid string', function () {
    $cls = FLOAT_CLASS;
    new $cls('invalid');
})->throws(InvalidArgumentException::class, "Invalid float value: 'invalid'");

it('throws when creating from string with trailing characters', function () {
    $cls = FLOAT_CLASS;
    new $cls('123.123    ');
})->throws(InvalidArgumentException::class, "Invalid characters were found in value: '123.123    '");

it('throws when creating out of range value', function (string $value) {
    $cls = FLOAT_CLASS;
    new $cls($value);
})->with([
    ['10e308'],
    ['-10e308'],
])->throws(RangeException::class);

it('correctly parses strings', function ($input, $expected) {
    $cls    = FLOAT_CLASS;
    $number = new $cls($input);
    expect((float) $number)->toEqualWithDelta((float) $expected, EPSILON);
})->with([
    ['123', 123],
    ['123.123', 123.123],
    ['.123', 0.123],
    ['0.123', 0.123],
    ['1e10', 1e10],
]);

it('creates from numbers', function ($number) {
    $cls   = FLOAT_CLASS;
    $float = new $cls($number);
    expect((float) $float)->toEqualWithDelta((float) $number, EPSILON)
        ->and($float->toInt())->toEqual((int) $number)
        ->and((float) (string) $float)->toEqualWithDelta((float) $number, EPSILON);
})->with([
    [0.123],
    [123],
]);

it('adds two floats', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('1');
    $float2 = new $cls('0.5');
    expect((float) $float1->add($float2))->toEqualWithDelta(1.5, EPSILON);
});

it('subtracts two floats', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('1');
    $float2 = new $cls('0.5');
    expect((float) $float1->sub($float2))->toEqualWithDelta(0.5, EPSILON);
});

it('multiplies two floats', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('2');
    $float2 = new $cls('0.5');
    expect((float) $float1->mul($float2))->toEqualWithDelta(1.0, EPSILON);
});

it('divides two floats', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('1');
    $float2 = new $cls('0.5');
    expect((float) $float1->div($float2))->toEqualWithDelta(2.0, EPSILON);
});

it('throws when dividing by zero', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('1');
    $float2 = new $cls('0');
    $float1->div($float2);
})->throws(DivideByZeroException::class);

it('computes modulo', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('1');
    $float2 = new $cls('2');
    expect((float) $float1->mod($float2))->toEqualWithDelta(1.0, EPSILON);
});

it('computes the absolute value', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('-123.123');
    expect((float) $float1->abs())->toEqualWithDelta(123.123, EPSILON);
});

it('computes the negation', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('123.123');
    expect((float) $float1->neg())->toEqualWithDelta(-123.123, EPSILON);
});

it('computes the square root', function () {
    $cls    = FLOAT_CLASS;
    $float1 = new $cls('4.0');
    expect((float) $float1->sqrt())->toEqualWithDelta(2.0, EPSILON);
});

it('considers equal floats equal', function ($value1, $value2) {
    expect($value1)->toEqual($value2)
        ->and($value1 == $value2)->toBeTrue();
})->with(function () {
    $cls = FLOAT_CLASS;
    return [
        [new $cls('3.14159'), new $cls('3.14159')],
        [new $cls(1.1), new $cls(1.1)],
    ];
});

it('considers different floats not equal', function ($value1, $value2) {
    expect($value1)->not->toEqual($value2)
        ->and($value1 == $value2)->toBeFalse();
})->with(function () {
    $cls = FLOAT_CLASS;
    return [
        [new $cls('3.14159'), new $cls('3.1415')],
        [new $cls(1.1), new $cls(2.2)],
    ];
});
