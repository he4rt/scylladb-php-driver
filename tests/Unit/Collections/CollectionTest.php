<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Collections;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Collection;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use Cassandra\Uuid;
use Cassandra\Varint;
use InvalidArgumentException;
use stdClass;

require_once __DIR__ . '/../Type/UnsupportedType.php';

// "Float" is a reserved word, so resolve via FQCN string.
const COLLECTION_FLOAT_CLASS = '\\Cassandra\\Float';

it('throws on invalid type', function () {
    new Collection(new stdClass());
})->throws(
    InvalidArgumentException::class,
    'type must be a string or an instance of Cassandra\\Type, an instance of stdClass given',
);

it('throws on unsupported string type', function () {
    new Collection('custom type');
})->throws(InvalidArgumentException::class, "Unsupported type 'custom type'");

it('throws on unsupported type', function () {
    new Collection(new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'type must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

dataset('collection_cassandra_types', [
    [Cassandra::TYPE_ASCII],
    [Cassandra::TYPE_BIGINT],
    [Cassandra::TYPE_BLOB],
    [Cassandra::TYPE_BOOLEAN],
    [Cassandra::TYPE_COUNTER],
    [Cassandra::TYPE_DECIMAL],
    [Cassandra::TYPE_DOUBLE],
    [Cassandra::TYPE_FLOAT],
    [Cassandra::TYPE_INT],
    [Cassandra::TYPE_TEXT],
    [Cassandra::TYPE_TIMESTAMP],
    [Cassandra::TYPE_UUID],
    [Cassandra::TYPE_VARCHAR],
    [Cassandra::TYPE_VARINT],
    [Cassandra::TYPE_TIMEUUID],
    [Cassandra::TYPE_INET],
]);

it('supports all Cassandra string types', function ($type) {
    $list = new Collection($type);
    expect($list->type()->valueType())->toEqual($type);
})->with('collection_cassandra_types');

it('supports scalar keys', function ($type, $value) {
    $list = Type::collection($type)->create();
    $list->add($value);
    expect(count($list))->toEqual(1)
        ->and($list->get(0))->toEqual($value)
        ->and($list->find($value))->toEqual(0);
})->with(function () {
    $cls = COLLECTION_FLOAT_CLASS;
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

it('supports composite keys', function ($type) {
    $list = Type::collection($type)->create();

    $list->add($type->create('a', '1', 'b', '2'));
    expect($list->find($type->create('a', '1', 'b', '2')))->toEqual(0)
        ->and(count($list))->toEqual(1);

    $list->add($type->create('c', '3', 'd', '4', 'e', '5'));
    expect($list->find($type->create('c', '3', 'd', '4', 'e', '5')))->toEqual(1)
        ->and(count($list))->toEqual(2);

    $list->remove(0);
    expect($list->find($type->create('a', '1', 'b', '2')))->toBeNull()
        ->and(count($list))->toEqual(1);

    $list->remove(1);
    expect($list->find($type->create('c', '3', 'd', '4', 'e', '5')))->toBeNull()
        ->and(count($list))->toEqual(0);
})->with([
    [Type::map(Type::varchar(), Type::varchar())],
    [Type::set(Type::varchar())],
    [Type::collection(Type::varchar())],
]);

it('validates types of elements', function () {
    $list = new Collection(Cassandra::TYPE_VARINT);
    $list->add(new Decimal('123'));
})->throws(
    InvalidArgumentException::class,
    'argument must be an instance of Cassandra\\Varint, an instance of Cassandra\\Decimal given',
);

it('rejects null values', function () {
    $list = new Collection(Cassandra::TYPE_VARINT);
    $list->add(null);
})->throws(
    InvalidArgumentException::class,
    'Invalid value: null is not supported inside collections',
);

it('adds all elements', function () {
    $list = new Collection(Cassandra::TYPE_VARINT);
    $list->add(
        new Varint('1'), new Varint('2'), new Varint('3'),
        new Varint('4'), new Varint('5'), new Varint('6'),
        new Varint('7'), new Varint('8'),
    );

    expect($list->count())->toEqual(8)
        ->and($list->values())->toEqual([
            new Varint('1'), new Varint('2'), new Varint('3'),
            new Varint('4'), new Varint('5'), new Varint('6'),
            new Varint('7'), new Varint('8'),
        ]);
});

it('returns null when cannot find index', function () {
    $list = new Collection(Cassandra::TYPE_VARINT);
    expect($list->find(new Varint('1')))->toBeNull();
});

it('finds index of an element', function () {
    $list = new Collection(Cassandra::TYPE_VARINT);
    $list->add(
        new Varint('1'), new Varint('2'), new Varint('3'),
        new Varint('4'), new Varint('5'), new Varint('6'),
        new Varint('7'), new Varint('8'),
    );

    for ($i = 1; $i <= 8; $i++) {
        expect($list->find(new Varint((string) $i)))->toEqual($i - 1);
    }
});

it('gets element by index', function () {
    $list = new Collection(Cassandra::TYPE_VARINT);
    $list->add(
        new Varint('1'), new Varint('2'), new Varint('3'),
        new Varint('4'), new Varint('5'), new Varint('6'),
        new Varint('7'), new Varint('8'),
    );

    for ($i = 1; $i <= 8; $i++) {
        expect($list->get($i - 1))->toEqual(new Varint((string) $i));
    }
});

it('supports foreach iteration', function () {
    $values = [
        new Varint('1'), new Varint('2'), new Varint('3'),
        new Varint('4'), new Varint('5'), new Varint('6'),
        new Varint('7'), new Varint('8'),
    ];
    $list = new Collection(Cassandra::TYPE_VARINT);
    foreach ($values as $value) {
        $list->add($value);
    }

    $index = 0;
    foreach ($list as $value) {
        expect($value)->toEqual($values[$index]);
        $index++;
    }

    $index = 0;
    foreach ($list as $key => $value) {
        expect($key)->toEqual($index)
            ->and($value)->toEqual($values[$key]);
        $index++;
    }
});

it('compares equal collections as equal', function ($value1, $value2) {
    expect($value2)->toEqual($value1)
        ->and($value1 == $value2)->toBeTrue();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [Type::collection(Type::int())->create(), Type::collection(Type::int())->create()],
        [Type::collection(Type::int())->create(1, 2, 3), Type::collection(Type::int())->create(1, 2, 3)],
        [Type::collection(Type::varchar())->create('a', 'b', 'c'), Type::collection(Type::varchar())->create('a', 'b', 'c')],
        [Type::collection($setType)->create($setType->create(1, 2, 3)), Type::collection($setType)->create($setType->create(1, 2, 3))],
    ];
});

it('compares non-equal collections as not equal', function ($value1, $value2) {
    expect($value2)->not->toEqual($value1)
        ->and($value1 == $value2)->toBeFalse();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [Type::collection(Type::int())->create(), Type::collection(Type::varchar())->create()],
        [Type::collection(Type::int())->create(1, 2, 3), Type::collection(Type::int())->create(4, 5, 6)],
        [Type::collection(Type::int())->create(1, 2, 3), Type::collection(Type::int())->create(1)],
        [Type::collection(Type::varchar())->create('a', 'b', 'c'), Type::collection(Type::varchar())->create('a', 'b', 'd')],
        [Type::collection($setType)->create($setType->create(1, 2, 3)), Type::collection($setType)->create($setType->create(4, 5, 6))],
    ];
});
