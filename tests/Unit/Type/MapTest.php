<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use InvalidArgumentException;

require_once __DIR__ . '/UnsupportedType.php';

it('defines map type', function () {
    $type = Type::map(Type::varchar(), Type::int());
    expect($type->name())->toEqual('map')
        ->and((string) $type)->toEqual('map<varchar, int>')
        ->and($type->keyType())->toEqual(Type::varchar())
        ->and($type->valueType())->toEqual(Type::int());
});

it('creates map from values', function () {
    $map = Type::map(Type::varchar(), Type::int())->create('a', 1, 'b', 2, 'c', 3, 'd', 4, 'e', 5);
    expect($map->keys())->toEqual(['a', 'b', 'c', 'd', 'e'])
        ->and($map->values())->toEqual([1, 2, 3, 4, 5])
        ->and($map['a'])->toEqual(1)
        ->and($map['b'])->toEqual(2)
        ->and($map['c'])->toEqual(3)
        ->and($map['d'])->toEqual(4)
        ->and($map['e'])->toEqual(5);
});

it('creates empty map', function () {
    $map = Type::map(Type::varchar(), Type::int())->create();
    expect(count($map))->toEqual(0);
});

it('prevents creating map without enough values', function () {
    Type::map(Type::varchar(), Type::int())
        ->create('a', 1, 'b', 2, 'c', 3, 'd', 4, 'e');
})->throws(
    InvalidArgumentException::class,
    'Not enough values, maps can only be created from an even number of values, ' .
    'where each odd value is a key and each even value is a value, ' .
    'e.g create(key, value, key, value, key, value)',
);

it('prevents creating map with unsupported types', function () {
    Type::map(Type::varchar(), Type::int())->create(1, 'a');
})->throws(InvalidArgumentException::class, 'argument must be a string, 1 given');

it('prevents defining maps with unsupported types', function () {
    Type::map(new UnsupportedType(), Type::varchar());
})->throws(
    InvalidArgumentException::class,
    'keyType must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('compares equal map types', function ($type1, $type2) {
    expect($type2)->toEqual($type1)
        ->and($type1 == $type2)->toBeTrue();
})->with(function () {
    return [
        [Type::map(Type::int(), Type::varchar()), Type::map(Type::int(), Type::varchar())],
        [Type::map(Type::varchar(), Type::collection(Type::int())), Type::map(Type::varchar(), Type::collection(Type::int()))],
        [Type::map(Type::collection(Type::int()), Type::varchar()), Type::map(Type::collection(Type::int()), Type::varchar())],
        [Type::map(Type::map(Type::int(), Type::varchar()), Type::varchar()), Type::map(Type::map(Type::int(), Type::varchar()), Type::varchar())],
    ];
});

it('compares not-equal map types', function ($type1, $type2) {
    expect($type2)->not->toEqual($type1)
        ->and($type1 == $type2)->toBeFalse();
})->with(function () {
    return [
        [Type::map(Type::int(), Type::varchar()), Type::map(Type::varchar(), Type::int())],
        [Type::map(Type::collection(Type::varchar()), Type::int()), Type::map(Type::collection(Type::int()), Type::int())],
        [Type::map(Type::map(Type::int(), Type::varchar()), Type::varchar()), Type::map(Type::map(Type::varchar(), Type::int()), Type::varchar())],
    ];
});
