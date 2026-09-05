<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use InvalidArgumentException;

require_once __DIR__ . '/UnsupportedType.php';

it('defines user type', function () {
    $type = Type::userType('a', Type::varchar());
    expect((string) $type)->toEqual('userType<a:varchar>');
    $types = $type->types();
    expect($types['a'])->toEqual(Type::varchar());
});

it('creates user type from values', function () {
    $udt = Type::userType('a', Type::varchar(), 'b', Type::int())->create('a', 'xyz', 'b', 123);
    expect($udt->values())->toEqual(['a' => 'xyz', 'b' => 123])
        ->and($udt->get('a'))->toEqual('xyz')
        ->and($udt->get('b'))->toEqual(123);
});

it('creates empty user type', function () {
    $udt = Type::userType('a', Type::varchar())->create();
    expect(count($udt))->toEqual(1)
        ->and($udt->get('a'))->toBeNull();

    $udt = Type::userType('a', Type::varchar(), 'b', Type::int())->create();
    expect(count($udt))->toEqual(2)
        ->and($udt->get('a'))->toBeNull()
        ->and($udt->get('b'))->toBeNull();

    $udt = Type::userType('a', Type::varchar(), 'b', Type::int(), 'c', Type::bigint())->create();
    expect(count($udt))->toEqual(3)
        ->and($udt->get('a'))->toBeNull()
        ->and($udt->get('b'))->toBeNull()
        ->and($udt->get('c'))->toBeNull();
});

it('prevents creating UDT type with invalid name pairing', function () {
    Type::userType(Type::varchar());
})->throws(
    InvalidArgumentException::class,
    'Not enough name/type pairs, user types can only be created from an even number of name/type pairs, ' .
    'where each odd argument is a name and each even argument is a type, ' .
    'e.g userType(name, type, name, type, name, type)',
);

it('prevents creating UDT with invalid name', function () {
    Type::userType('a', Type::varchar())->create(1);
})->throws(
    InvalidArgumentException::class,
    'Not enough name/value pairs, user_types can only be created from an even number of name/value pairs, ' .
    'where each odd argument is a name and each even argument is a value, ' .
    'e.g user_type(name, value, name, value, name, value)',
);

it('prevents creating UDT with unsupported types', function () {
    Type::userType('a', Type::varchar())->create('a', 1);
})->throws(InvalidArgumentException::class, 'argument must be a string, 1 given');

it('prevents defining UDTs with unsupported types', function () {
    Type::userType('a', new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'type must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('supports withName and withKeyspace', function () {
    $userType = Type::userType('a', Type::int(), 'b', Type::varchar());
    expect($userType->name())->toBeNull()
        ->and($userType->keyspace())->toBeNull();

    $userType1 = $userType->withName('abc');
    $types     = $userType1->types();
    expect(count($types))->toEqual(2)
        ->and($types['a'])->toEqual(Type::int())
        ->and($types['b'])->toEqual(Type::varchar())
        ->and($userType1->name())->toEqual('abc')
        ->and($userType1->keyspace())->toBeNull()
        ->and($userType->name())->toBeNull()
        ->and($userType->keyspace())->toBeNull();

    $userType2 = $userType1->withKeyspace('xyz');
    $types     = $userType2->types();
    expect(count($types))->toEqual(2)
        ->and($types['a'])->toEqual(Type::int())
        ->and($types['b'])->toEqual(Type::varchar())
        ->and($userType2->name())->toEqual('abc')
        ->and($userType2->keyspace())->toEqual('xyz')
        ->and($userType->name())->toBeNull()
        ->and($userType->keyspace())->toBeNull();
});

it('compares equal user types', function ($type1, $type2) {
    expect($type2)->toEqual($type1)
        ->and($type1 == $type2)->toBeTrue();
})->with(function () {
    return [
        [Type::userType('a', Type::int()), Type::userType('a', Type::int())],
        [Type::userType('a', Type::int(), 'b', Type::varchar()), Type::userType('a', Type::int(), 'b', Type::varchar())],
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint()),
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint()),
        ],
        [
            Type::userType('a', Type::collection(Type::int()), 'b', Type::set(Type::int())),
            Type::userType('a', Type::collection(Type::int()), 'b', Type::set(Type::int())),
        ],
    ];
});

it('compares not-equal user types', function ($type1, $type2) {
    expect($type2)->not->toEqual($type1)
        ->and($type1 == $type2)->toBeFalse();
})->with(function () {
    return [
        [Type::userType('a', Type::int()), Type::userType('a', Type::varchar())],
        [Type::userType('a', Type::int(), 'b', Type::varchar()), Type::userType('a', Type::int(), 'b', Type::bigint())],
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::varint()),
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint()),
        ],
        [
            Type::userType('a', Type::collection(Type::int()), 'b', Type::set(Type::varchar())),
            Type::userType('a', Type::collection(Type::int()), 'b', Type::set(Type::int())),
        ],
        [Type::userType('a', Type::int()), Type::userType('b', Type::int())],
        [
            Type::userType('a', Type::int(), 'c', Type::varchar()),
            Type::userType('b', Type::int(), 'c', Type::varchar()),
        ],
    ];
});

it('treats two named user types with different names as different types', function () {
    $base = Type::userType('a', Type::int());

    expect($base->withName('x')->withKeyspace('ks') == $base->withName('y')->withKeyspace('ks'))->toBeFalse()
        ->and($base->withName('x')->withKeyspace('ks') == $base->withName('x')->withKeyspace('other'))->toBeFalse()
        ->and($base->withName('x')->withKeyspace('ks') == $base->withName('x')->withKeyspace('ks'))->toBeTrue();
});

it('lets an unnamed user type match a named one with the same layout', function () {
    $base  = Type::userType('a', Type::int());
    $named = $base->withName('x')->withKeyspace('ks');

    expect($base == $named)->toBeTrue()
        ->and($base == Type::userType('a', Type::text())->withName('x'))->toBeFalse();
});

it('shows the keyspace and name alongside the field types', function () {
    $named = Type::userType('a', Type::int())->withName('x')->withKeyspace('ks');

    expect((array) $named)->toHaveKeys(['keyspace', 'name', 'types'])
        ->and(((array) $named)['keyspace'])->toBe('ks')
        ->and(((array) $named)['name'])->toBe('x')
        ->and(((array) Type::userType('a', Type::int()))['name'])->toBeNull();
});
