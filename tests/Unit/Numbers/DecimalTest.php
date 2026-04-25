<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Numbers;

use Cassandra\Decimal;
use InvalidArgumentException;
use RuntimeException;

it('throws when creating from a non-numeric string', function () {
    new Decimal('qwe');
})->throws(InvalidArgumentException::class, "Unrecognized character 'q' at position 0");

it('correctly parses strings and numbers', function ($input, $value, $scale, $string) {
    $number = new Decimal($input);
    expect($number->value())->toEqual($value)
        ->and($number->scale())->toEqual($scale)
        ->and((string) $number)->toEqual($string)
        ->and(abs((float) $string - (float) $number))->toBeLessThanOrEqual(0.01);
})->with([
    ['123', '123', 0, '123'],
    ['0123', '83', 0, '83'],
    ['0x123', '291', 0, '291'],
    ['0b1010101', '85', 0, '85'],
    ['-123', '-123', 0, '-123'],
    ['-0123', '-83', 0, '-83'],
    ['-0x123', '-291', 0, '-291'],
    ['-0b1010101', '-85', 0, '-85'],
    ['1313123123.234234234234234234123', '1313123123234234234234234234123', 21, '1313123123.234234234234234234123'],
    ['123.1', '1231', 1, '123.1'],
    ['55.55', '5555', 2, '55.55'],
    ['-123.123', '-123123', 3, '-123.123'],
    ['0.5', '5', 1, '0.5'],
    // The following matches the legacy expectation for double-to-decimal conversion.
    [0.123, '1229999999999999982236431605997495353221893310546875', 52, '0.1229999999999999982236431605997495353221893310546875'],
    [123, '123', 0, '123'],
    [123.5, '1235', 1, '123.5'],
    [-123, '-123', 0, '-123'],
    ['9.5', '95', 1, '9.5'],
    ['-9.5', '-95', 1, '-9.5'],
    ['0.00001', '1', 5, '0.00001'],
    ['-0.00001', '-1', 5, '-0.00001'],
    ['0.00000001', '1', 8, '1E-8'],
    ['-0.00000001', '-1', 8, '-1E-8'],
    ['0.000000095', '95', 9, '9.5E-8'],
    ['-0.000000095', '-95', 9, '-9.5E-8'],
    ['0.000000015', '15', 9, '1.5E-8'],
    ['-0.000000015', '-15', 9, '-1.5E-8'],
]);

it('adds two decimals', function () {
    $decimal1 = new Decimal('1');
    $decimal2 = new Decimal('0.5');
    expect((string) $decimal1->add($decimal2))->toEqual('1.5');
});

it('subtracts two decimals', function () {
    $decimal1 = new Decimal('1');
    $decimal2 = new Decimal('0.5');
    expect((string) $decimal1->sub($decimal2))->toEqual('0.5');
});

it('multiplies two decimals', function () {
    $decimal1 = new Decimal('2');
    $decimal2 = new Decimal('0.5');
    expect((string) $decimal1->mul($decimal2))->toEqual('1.0');
});

it('throws RuntimeException when dividing decimals (not implemented)', function () {
    $decimal1 = new Decimal('1.0');
    $decimal2 = new Decimal('0.5');
    $decimal1->div($decimal2);
})->throws(RuntimeException::class, 'Not implemented');

it('throws RuntimeException when dividing by zero (not implemented)', function () {
    $decimal1 = new Decimal('1');
    $decimal2 = new Decimal('0');
    $decimal1->div($decimal2);
})->throws(RuntimeException::class, 'Not implemented');

it('throws RuntimeException when computing modulo (not implemented)', function () {
    $decimal1 = new Decimal('1');
    $decimal2 = new Decimal('2');
    $decimal1->mod($decimal2);
})->throws(RuntimeException::class, 'Not implemented');

it('computes the absolute value', function () {
    $decimal = new Decimal('-123.123');
    expect((string) $decimal->abs())->toEqual('123.123');
});

it('computes the negation', function () {
    $decimal = new Decimal('123.123');
    expect((string) $decimal->neg())->toEqual('-123.123');
});

it('throws RuntimeException when computing square root (not implemented)', function () {
    $decimal = new Decimal('4');
    $decimal->sqrt();
})->throws(RuntimeException::class, 'Not implemented');

it('considers equal decimals equal', function ($value1, $value2) {
    expect($value1)->toEqual($value2)
        ->and($value1 == $value2)->toBeTrue();
})->with([
    [new Decimal('3.14159'), new Decimal('3.14159')],
    [new Decimal(1.1), new Decimal(1.1)],
]);

it('considers different decimals not equal', function ($value1, $value2) {
    expect($value1)->not->toEqual($value2)
        ->and($value1 == $value2)->toBeFalse();
})->with([
    [new Decimal('3.14159'), new Decimal('3.1415')],
    [new Decimal(1.1), new Decimal(2.2)],
]);
