<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Collections;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Tuple;
use Cassandra\Type;
use Cassandra\Uuid;
use Cassandra\Varint;
use InvalidArgumentException;

const TUPLE_FLOAT_CLASS = '\\Cassandra\\Float';

it('supports only Cassandra types in tuple', function () {
    new Tuple(['custom type']);
})->throws(InvalidArgumentException::class, "Unsupported type 'custom type'");

dataset('tuple_cassandra_types', [
    [[Cassandra::TYPE_TEXT]],
    [[Cassandra::TYPE_ASCII]],
    [[Cassandra::TYPE_VARCHAR]],
    [[Cassandra::TYPE_BIGINT]],
    [[Cassandra::TYPE_BOOLEAN]],
    [[Cassandra::TYPE_COUNTER]],
    [[Cassandra::TYPE_DECIMAL]],
    [[Cassandra::TYPE_DOUBLE]],
    [[Cassandra::TYPE_FLOAT]],
    [[Cassandra::TYPE_INT]],
    [[Cassandra::TYPE_TIMESTAMP]],
    [[Cassandra::TYPE_UUID]],
    [[Cassandra::TYPE_VARINT]],
    [[Cassandra::TYPE_TIMEUUID]],
    [[Cassandra::TYPE_INET]],
    [[Cassandra::TYPE_TIMEUUID, Cassandra::TYPE_UUID]],
    [[Cassandra::TYPE_INT, Cassandra::TYPE_BIGINT, Cassandra::TYPE_VARINT]],
    [[Cassandra::TYPE_INT, Cassandra::TYPE_BIGINT, Cassandra::TYPE_VARINT]],
    [[Cassandra::TYPE_ASCII, Cassandra::TYPE_TEXT, Cassandra::TYPE_VARCHAR]],
]);

it('supports all Cassandra types for tuple', function ($types) {
    $tuple = new Tuple($types);
    expect($tuple)->toBeInstanceOf(Tuple::class);
})->with('tuple_cassandra_types');

it('returns its tuple type', function ($types) {
    $tuple = new Tuple($types);
    expect(count($tuple->type()->types()))->toEqual(count($types))
        ->and($tuple->type()->types())->toEqual($types);
})->with('tuple_cassandra_types');

it('supports scalar tuple types', function ($type, $value) {
    $tuple = Type::tuple($type)->create();
    $tuple->set(0, $value);
    expect(count($tuple))->toEqual(1)
        ->and($tuple->get(0))->toEqual($value);
})->with(function () {
    $cls = TUPLE_FLOAT_CLASS;
    return [
        [Type::ascii(), 'ascii'],
        [Type::bigint(), new Bigint('9223372036854775807')],
        [Type::blob(), new Blob('blob')],
        [Type::boolean(), true],
        [Type::counter(), new Bigint(123)],
        [Type::decimal(), new Decimal('3.14159265359')],
        [Type::double(), 3.14159],
        [Type::float(), new $cls(3.14159)],
        [Type::inet(), new Inet('127.0.0.1')],
        [Type::int(), 123],
        [Type::text(), 'text'],
        [Type::timestamp(), new Timestamp(123)],
        [Type::timeuuid(), new Timeuuid(0)],
        [Type::uuid(), new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c2')],
        [Type::varchar(), 'varchar'],
        [Type::varint(), new Varint('9223372036854775808')],
        [Type::duration(), new Duration(1, 2, 3)],
    ];
});

it('supports composite tuple keys', function ($type, $value) {
    $tuple = Type::tuple($type)->create($value);
    expect($tuple->get(0))->toEqual($value);
})->with(function () {
    $map_type   = Type::map(Type::varchar(), Type::varchar());
    $set_type   = Type::set(Type::varchar());
    $list_type  = Type::collection(Type::varchar());
    $tuple_type = Type::tuple(Type::varchar(), Type::int());
    return [
        [$map_type, $map_type->create('a', '1', 'b', '2')],
        [$set_type, $set_type->create('a', 'b', 'c')],
        [$list_type, $list_type->create('a', 'b', 'c')],
        [$tuple_type, $tuple_type->create('a', 42)],
    ];
});

it('validates types of tuple elements', function () {
    $tuple = new Tuple([Cassandra::TYPE_VARINT]);
    $tuple->set(0, new Decimal('123'));
})->throws(
    InvalidArgumentException::class,
    'argument must be an instance of Cassandra\\Varint, an instance of Cassandra\\Decimal given',
);

it('sets all elements', function () {
    $tuple = new Tuple([
        Cassandra::TYPE_BOOLEAN,
        Cassandra::TYPE_INT,
        Cassandra::TYPE_BIGINT,
        Cassandra::TYPE_TEXT,
    ]);

    expect($tuple->count())->toEqual(4);

    $tuple->set(0, true);
    $tuple->set(1, 42);
    $tuple->set(2, new Bigint('123'));
    $tuple->set(3, 'abc');

    expect($tuple->count())->toEqual(4)
        ->and($tuple->get(0))->toEqual(true)
        ->and($tuple->get(1))->toEqual(42)
        ->and($tuple->get(2))->toEqual(new Bigint('123'))
        ->and($tuple->get(3))->toEqual('abc');
});

it('throws on invalid set index', function () {
    $tuple = new Tuple([Cassandra::TYPE_TEXT]);
    $tuple->set(1, 'invalid index');
})->throws(InvalidArgumentException::class, 'Index out of bounds');

it('throws on invalid get index', function () {
    $tuple = new Tuple([Cassandra::TYPE_TEXT]);
    $tuple->set(0, 'invalid index');
    $tuple->get(1);
})->throws(InvalidArgumentException::class, 'Index out of bounds');

it('compares equal tuples as equal', function ($value1, $value2) {
    expect($value2)->toEqual($value1)
        ->and($value1 == $value2)->toBeTrue();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(),
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(),
        ],
        [
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(1, 'a', new Bigint(99)),
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(1, 'a', new Bigint(99)),
        ],
        [
            Type::tuple($setType, Type::varchar())->create($setType->create(1, 2, 3), 'a'),
            Type::tuple($setType, Type::varchar())->create($setType->create(1, 2, 3), 'a'),
        ],
    ];
});

it('compares non-equal tuples as not equal', function ($value1, $value2) {
    expect($value2)->not->toEqual($value1)
        ->and($value1 == $value2)->toBeFalse();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [
            Type::tuple(Type::int(), Type::varchar(), Type::varint())->create(),
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(),
        ],
        [
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(1, 'a', new Bigint(99)),
            Type::tuple(Type::int(), Type::varchar(), Type::bigint())->create(2, 'b', new Bigint(99)),
        ],
        [
            Type::tuple($setType, Type::varchar())->create($setType->create(1, 2, 3), 'a'),
            Type::tuple($setType, Type::varchar())->create($setType->create(4, 5, 6), 'a'),
        ],
    ];
});
