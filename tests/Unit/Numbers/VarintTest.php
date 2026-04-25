<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Numbers;

use Cassandra\Varint;

it('adds large varints', function () {
    $varint1 = new Varint('9223372036854775807');
    $varint2 = new Varint('1');
    expect((string) $varint1->add($varint2))->toEqual('9223372036854775808');
});

it('subtracts large varints', function () {
    $varint1 = new Varint('-9223372036854775808');
    $varint2 = new Varint('1');
    expect((string) $varint1->sub($varint2))->toEqual('-9223372036854775809');
});

it('multiplies large varints', function () {
    $varint1 = new Varint('9223372036854775807');
    $varint2 = new Varint('2');
    expect((string) $varint1->mul($varint2))->toEqual('18446744073709551614');
});

it('divides large varints', function () {
    $varint1 = new Varint('18446744073709551614');
    $varint2 = new Varint('9223372036854775807');
    expect((int) $varint1->div($varint2))->toEqual(2);
});

it('computes modulo for large varints', function () {
    $varint1 = new Varint('1');
    $varint2 = new Varint('18446744073709551614');
    expect((int) $varint1->mod($varint2))->toEqual(1);
});

it('computes the absolute value for large varints', function () {
    $varint = new Varint('-18446744073709551614');
    expect((string) $varint->abs())->toEqual('18446744073709551614');
});

it('computes the negation for large varints', function () {
    $varint = new Varint('18446744073709551614');
    expect((string) $varint->neg())->toEqual('-18446744073709551614');
});

it('computes the square root for large varints', function () {
    $varint = new Varint('340282366920938463389587631136930004996');
    expect((string) $varint->sqrt())->toEqual('18446744073709551614');
});

it('considers equal large varints equal', function () {
    $value1 = new Varint('123456789123456789123456789');
    $value2 = new Varint('123456789123456789123456789');
    expect($value1)->toEqual($value2)
        ->and($value1 == $value2)->toBeTrue();
});

it('considers different large varints not equal', function () {
    $value1 = new Varint('123456789123456789123456789');
    $value2 = new Varint('999999999999999999999999999');
    expect($value1)->not->toEqual($value2)
        ->and($value1 == $value2)->toBeFalse();
});
