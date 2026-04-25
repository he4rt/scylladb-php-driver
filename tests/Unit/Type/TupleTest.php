<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use InvalidArgumentException;

require_once __DIR__ . '/UnsupportedType.php';

it('defines tuple type', function () {
    $type = Type::tuple(Type::varchar(), Type::int());
    expect($type->name())->toEqual('tuple')
        ->and((string) $type)->toEqual('tuple<varchar, int>');
    $types = $type->types();
    expect($types[0])->toEqual(Type::varchar())
        ->and($types[1])->toEqual(Type::int());
});

it('creates tuple from values', function () {
    $tuple = Type::tuple(Type::varchar(), Type::int())->create('xyz', 123);
    expect($tuple->values())->toEqual(['xyz', 123])
        ->and($tuple->get(0))->toEqual('xyz')
        ->and($tuple->get(1))->toEqual(123);
});

it('creates empty tuple', function () {
    $tuple = Type::tuple(Type::varchar())->create();
    expect(count($tuple))->toEqual(1)
        ->and($tuple->get(0))->toBeNull();

    $tuple = Type::tuple(Type::varchar(), Type::int())->create();
    expect(count($tuple))->toEqual(2)
        ->and($tuple->get(0))->toBeNull()
        ->and($tuple->get(1))->toBeNull();

    $tuple = Type::tuple(Type::varchar(), Type::int(), Type::bigint())->create();
    expect(count($tuple))->toEqual(3)
        ->and($tuple->get(0))->toBeNull()
        ->and($tuple->get(1))->toBeNull()
        ->and($tuple->get(2))->toBeNull();
});

it('prevents creating tuple with invalid type', function () {
    Type::tuple(Type::varchar())->create(1);
})->throws(InvalidArgumentException::class, 'argument must be a string, 1 given');

it('prevents defining tuples with unsupported types', function () {
    Type::tuple(new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'type must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('compares equal tuple types', function ($type1, $type2) {
    expect($type2)->toEqual($type1)
        ->and($type1 == $type2)->toBeTrue();
})->with(function () {
    return [
        [Type::tuple(Type::int()), Type::tuple(Type::int())],
        [Type::tuple(Type::int(), Type::varchar()), Type::tuple(Type::int(), Type::varchar())],
        [Type::tuple(Type::int(), Type::varchar(), Type::bigint()), Type::tuple(Type::int(), Type::varchar(), Type::bigint())],
        [
            Type::tuple(Type::collection(Type::int()), Type::set(Type::int())),
            Type::tuple(Type::collection(Type::int()), Type::set(Type::int())),
        ],
    ];
});

it('compares not-equal tuple types', function ($type1, $type2) {
    expect($type2)->not->toEqual($type1)
        ->and($type1 == $type2)->toBeFalse();
})->with(function () {
    return [
        [Type::tuple(Type::int()), Type::tuple(Type::varchar())],
        [Type::tuple(Type::int(), Type::varchar()), Type::tuple(Type::int(), Type::bigint())],
        [Type::tuple(Type::int(), Type::varchar(), Type::varint()), Type::tuple(Type::int(), Type::varchar(), Type::bigint())],
        [
            Type::tuple(Type::collection(Type::int()), Type::set(Type::varchar())),
            Type::tuple(Type::collection(Type::int()), Type::set(Type::int())),
        ],
    ];
});
