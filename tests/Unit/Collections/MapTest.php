<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Collections;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Map;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use Cassandra\Uuid;
use Cassandra\Varint;
use InvalidArgumentException;
use stdClass;

require_once __DIR__ . '/../Type/UnsupportedType.php';

const MAP_FLOAT_CLASS = '\\Cassandra\\Float';

it('throws on invalid key type', function () {
    new Map(new stdClass(), Cassandra::TYPE_VARCHAR);
})->throws(
    InvalidArgumentException::class,
    'keyType must be a string or an instance of Cassandra\\Type, an instance of stdClass given',
);

it('throws on unsupported string key type', function () {
    new Map('custom type', Cassandra::TYPE_VARCHAR);
})->throws(InvalidArgumentException::class, "Unsupported type 'custom type'");

it('throws on unsupported key type', function () {
    new Map(new UnsupportedType(), Type::varchar());
})->throws(
    InvalidArgumentException::class,
    'keyType must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('throws on invalid value type', function () {
    new Map(Cassandra::TYPE_VARCHAR, new stdClass());
})->throws(
    InvalidArgumentException::class,
    'valueType must be a string or an instance of Cassandra\\Type, an instance of stdClass given',
);

it('throws on unsupported string value type', function () {
    new Map(Cassandra::TYPE_VARCHAR, 'custom type');
})->throws(InvalidArgumentException::class, "Unsupported type 'custom type'");

it('throws on unsupported value type', function () {
    new Map(Type::varchar(), new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'valueType must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('supports key-based access', function () {
    $map = Type::map(Type::varint(), Type::varchar())->create();
    expect(count($map))->toEqual(0);
    $map->set(new Varint('123'), 'value');
    expect(count($map))->toEqual(1)
        ->and($map->has(new Varint('123')))->toBeTrue()
        ->and($map->get(new Varint('123')))->toEqual('value');
    $map->set(new Varint('123'), 'another value');
    expect(count($map))->toEqual(1)
        ->and($map->get(new Varint('123')))->toEqual('another value');
});

it('supports scalar keys', function ($keyType, $values) {
    $map = Type::map($keyType, Type::varchar())->create();
    foreach ($values as $index => $keyValue) {
        $map->set($keyValue, "value$index");
    }
    expect(count($map))->toEqual(count($values));

    foreach ($values as $index => $keyValue) {
        expect($map->has($keyValue))->toBeTrue()
            ->and($map->get($keyValue))->toEqual("value$index");
    }

    foreach ($values as $keyValue) {
        $map->remove($keyValue);
    }
    expect(count($map))->toEqual(0);
})->with(function () {
    $cls = MAP_FLOAT_CLASS;
    return [
        [Type::ascii(), ['ascii']],
        [Type::bigint(), [new Bigint('9223372036854775807')]],
        [Type::blob(), [new Blob('blob')]],
        [Type::boolean(), [true, false]],
        [Type::counter(), [new Bigint(123)]],
        [Type::decimal(), [new Decimal('3.14159265359')]],
        [Type::double(), [3.14159]],
        [Type::float(), [new $cls(3.14159)]],
        [Type::inet(), [new Inet('127.0.0.1')]],
        [Type::int(), [123]],
        [Type::text(), ['text']],
        [Type::timestamp(), [new Timestamp(123)]],
        [Type::timeuuid(), [new Timeuuid(0)]],
        [Type::uuid(), [new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c2')]],
        [Type::varchar(), ['varchar']],
        [Type::varint(), [new Varint('9223372036854775808')]],
        [Type::duration(), [new Duration(1, 2, 3)]],
    ];
});

it('supports composite keys for map', function ($keyType) {
    $map = Type::map($keyType, Type::varchar())->create();

    $map->set($keyType->create('a', '1', 'b', '2'), 'value1');
    expect($map->get($keyType->create('a', '1', 'b', '2')))->toEqual('value1')
        ->and(count($map))->toEqual(1);

    $map->set($keyType->create('c', '3', 'd', '4', 'e', '5'), 'value2');
    expect($map->get($keyType->create('c', '3', 'd', '4', 'e', '5')))->toEqual('value2')
        ->and(count($map))->toEqual(2);

    $map->remove($keyType->create('a', '1', 'b', '2'));
    expect($map->has($keyType->create('a', '1', 'b', '2')))->toBeFalse()
        ->and(count($map))->toEqual(1);

    $map->remove($keyType->create('c', '3', 'd', '4', 'e', '5'));
    expect($map->has($keyType->create('c', '3', 'd', '4', 'e', '5')))->toBeFalse()
        ->and(count($map))->toEqual(0);
})->with([
    [Type::map(Type::varchar(), Type::varchar())],
    [Type::set(Type::varchar())],
    [Type::collection(Type::varchar())],
]);

it('supports only Cassandra types for keys', function () {
    new Map('custom type', Cassandra::TYPE_VARINT);
})->throws(InvalidArgumentException::class, "Unsupported type 'custom type'");

it('supports only Cassandra types for values', function () {
    new Map(Cassandra::TYPE_VARINT, 'another custom type');
})->throws(InvalidArgumentException::class, "Unsupported type 'another custom type'");

it('rejects null values in map', function () {
    $map = new Map(Cassandra::TYPE_VARCHAR, Cassandra::TYPE_VARCHAR);
    $map->set('test', null);
})->throws(InvalidArgumentException::class, 'Invalid value: null is not supported inside maps');

it('rejects null keys', function () {
    $map = new Map(Cassandra::TYPE_VARCHAR, Cassandra::TYPE_VARCHAR);
    $map->set(null, 'test');
})->throws(InvalidArgumentException::class, 'Invalid key: null is not supported inside maps');

it('supports foreach iteration on map', function () {
    $keys = [
        new Varint('1'), new Varint('2'), new Varint('3'), new Varint('4'),
        new Varint('5'), new Varint('6'), new Varint('7'), new Varint('8'),
    ];
    $values = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'];
    $map    = new Map(Cassandra::TYPE_VARINT, Cassandra::TYPE_VARCHAR);

    for ($i = 0; $i < count($keys); $i++) {
        $map->set($keys[$i], $values[$i]);
    }

    $index = 0;
    foreach ($map as $value) {
        expect($value)->toEqual($values[$index]);
        $index++;
    }

    $index = 0;
    foreach ($map as $key => $value) {
        expect($key)->toEqual($keys[$index])
            ->and($value)->toEqual($values[$index]);
        $index++;
    }
});

it('supports retrieving keys and values', function () {
    $keys = [
        new Varint('1'), new Varint('2'), new Varint('3'), new Varint('4'),
        new Varint('5'), new Varint('6'), new Varint('7'), new Varint('8'),
    ];
    $values = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'];
    $map    = new Map(Cassandra::TYPE_VARINT, Cassandra::TYPE_VARCHAR);
    for ($i = 0; $i < count($keys); $i++) {
        $map->set($keys[$i], $values[$i]);
    }
    expect($map->keys())->toEqual($keys)
        ->and($map->values())->toEqual($values);
});

it('compares equal maps as equal', function ($value1, $value2) {
    expect($value2)->toEqual($value1)
        ->and($value1 == $value2)->toBeTrue();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [Type::map(Type::int(), Type::varchar())->create(), Type::map(Type::int(), Type::varchar())->create()],
        [
            Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b', 3, 'c'),
            Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b', 3, 'c'),
        ],
        [
            Type::map($setType, Type::varchar())->create($setType->create(1, 2, 3), 'a', $setType->create(4, 5, 6), 'b'),
            Type::map($setType, Type::varchar())->create($setType->create(1, 2, 3), 'a', $setType->create(4, 5, 6), 'b'),
        ],
    ];
});

it('compares non-equal maps as not equal', function ($value1, $value2) {
    expect($value2)->not->toEqual($value1)
        ->and($value1 == $value2)->toBeFalse();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [Type::map(Type::int(), Type::int())->create(), Type::map(Type::int(), Type::varchar())->create()],
        [
            Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b', 3, 'c'),
            Type::map(Type::int(), Type::varchar())->create(1, 'a'),
        ],
        [
            Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b', 3, 'c'),
            Type::map(Type::int(), Type::varchar())->create(4, 'a', 5, 'b', 6, 'c'),
        ],
        [
            Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b', 3, 'c'),
            Type::map(Type::int(), Type::varchar())->create(1, 'd', 2, 'e', 3, 'f'),
        ],
        [
            Type::map($setType, Type::varchar())->create($setType->create(4, 5, 6), 'a', $setType->create(7, 8, 9), 'b'),
            Type::map($setType, Type::varchar())->create($setType->create(1, 2, 3), 'a', $setType->create(4, 5, 6), 'b'),
        ],
    ];
});

it('compares maps of different counts using lt/gt', function () {
    expect(
        Type::map(Type::int(), Type::varchar())->create(1, 'a') <
        Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b'),
    )->toBeTrue()
        ->and(
            Type::map(Type::int(), Type::varchar())->create(1, 'a', 2, 'b') >
            Type::map(Type::int(), Type::varchar())->create(1, 'a'),
        )->toBeTrue();
});
