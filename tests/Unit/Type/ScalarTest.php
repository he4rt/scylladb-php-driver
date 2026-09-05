<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;

it('allows creating scalar values from types', function () {
    expect(Type::varchar()->create('some string'))->toEqual('some string');
});

dataset('scalarTypes', [
    'ascii', 'bigint', 'blob', 'boolean', 'counter', 'decimal', 'double', 'float',
    'inet', 'int', 'text', 'timestamp', 'timeuuid', 'uuid', 'varchar', 'varint',
]);

it('hands out one shared instance per scalar type', function (string $name) {
    expect(Type::$name())->toBe(Type::$name())
        ->and(Type::$name() == Type::$name())->toBeTrue();
})->with('scalarTypes');

it('compares two distinct objects of the same scalar type as equal', function (string $name) {
    $left  = Type::collection(Type::$name());
    $right = Type::collection(Type::$name());

    expect($left)->not->toBe($right)
        ->and($left == $right)->toBeTrue();
})->with('scalarTypes');

it('treats text and varchar as the same type without sharing an instance', function () {
    expect(Type::text())->not->toBe(Type::varchar())
        ->and(Type::text() == Type::varchar())->toBeTrue()
        ->and(Type::collection(Type::text()) == Type::collection(Type::varchar()))->toBeTrue();
});

it('compares distinct objects of different scalar types as unequal', function () {
    expect(Type::collection(Type::int()) == Type::collection(Type::bigint()))->toBeFalse()
        ->and(Type::collection(Type::int()) == Type::set(Type::int()))->toBeFalse()
        ->and(Type::map(Type::int(), Type::text()) == Type::map(Type::int(), Type::blob()))->toBeFalse();
});

it('compares not-equal scalar types', function () {
    expect(Type::ascii() != Type::bigint())->toBeTrue();
});
