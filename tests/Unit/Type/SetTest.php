<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use InvalidArgumentException;

require_once __DIR__ . '/UnsupportedType.php';

it('defines set type', function () {
    $type = Type::set(Type::varchar());
    expect($type->name())->toEqual('set')
        ->and((string) $type)->toEqual('set<varchar>')
        ->and($type->valueType())->toEqual(Type::varchar());
});

it('creates set from values', function () {
    $set = Type::set(Type::varchar())->create('a', 'b', 'c', 'd', 'e');
    expect($set->values())->toEqual(['a', 'b', 'c', 'd', 'e']);
});

it('creates empty set', function () {
    $set = Type::set(Type::varchar())->create();
    expect(count($set))->toEqual(0);
});

it('prevents creating set with unsupported types', function () {
    Type::set(Type::varchar())->create(1);
})->throws(InvalidArgumentException::class, 'argument must be a string, 1 given');

it('prevents defining sets with unsupported types', function () {
    Type::set(new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'type must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('compares equal set types', function ($type1, $type2) {
    expect($type2)->toEqual($type1)
        ->and($type1 == $type2)->toBeTrue();
})->with(function () {
    return [
        [Type::set(Type::int()), Type::set(Type::int())],
        [Type::set(Type::collection(Type::int())), Type::set(Type::collection(Type::int()))],
        [Type::set(Type::set(Type::int())), Type::set(Type::set(Type::int()))],
    ];
});

it('compares not-equal set types', function ($type1, $type2) {
    expect($type2)->not->toEqual($type1)
        ->and($type1 == $type2)->toBeFalse();
})->with(function () {
    return [
        [Type::set(Type::varchar()), Type::set(Type::int())],
        [Type::set(Type::collection(Type::varchar())), Type::set(Type::collection(Type::int()))],
        [Type::set(Type::collection(Type::int())), Type::set(Type::set(Type::int()))],
    ];
});
