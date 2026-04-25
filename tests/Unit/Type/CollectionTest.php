<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use InvalidArgumentException;

require_once __DIR__ . '/UnsupportedType.php';

it('defines collection type', function () {
    $type = Type::collection(Type::varchar());
    expect($type->name())->toEqual('list')
        ->and((string) $type)->toEqual('list<varchar>')
        ->and($type->valueType())->toEqual(Type::varchar());
});

it('creates collection from values', function () {
    $list = Type::collection(Type::varchar())->create('a', 'b', 'c', 'd', 'e');
    expect($list->values())->toEqual(['a', 'b', 'c', 'd', 'e'])
        ->and($list->get(0))->toEqual('a')
        ->and($list->get(1))->toEqual('b')
        ->and($list->get(2))->toEqual('c')
        ->and($list->get(3))->toEqual('d')
        ->and($list->get(4))->toEqual('e');
});

it('creates empty collection', function () {
    $list = Type::collection(Type::varchar())->create();
    expect(count($list))->toEqual(0);
});

it('prevents creating collection with unsupported types', function () {
    Type::collection(Type::varchar())->create(1);
})->throws(InvalidArgumentException::class, 'argument must be a string, 1 given');

it('prevents defining collections with unsupported types', function () {
    Type::collection(new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'type must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('compares equal collection types', function ($type1, $type2) {
    expect($type2)->toEqual($type1)
        ->and($type1 == $type2)->toBeTrue();
})->with(function () {
    return [
        [Type::collection(Type::int()), Type::collection(Type::int())],
        [Type::collection(Type::collection(Type::int())), Type::collection(Type::collection(Type::int()))],
        [Type::collection(Type::set(Type::int())), Type::collection(Type::set(Type::int()))],
    ];
});

it('compares not-equal collection types', function ($type1, $type2) {
    expect($type2)->not->toEqual($type1)
        ->and($type1 == $type2)->toBeFalse();
})->with(function () {
    return [
        [Type::collection(Type::varchar()), Type::collection(Type::int())],
        [Type::collection(Type::collection(Type::varchar())), Type::collection(Type::collection(Type::int()))],
        [Type::collection(Type::collection(Type::int())), Type::collection(Type::set(Type::int()))],
    ];
});
