<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Collections;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Set;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Type;
use Cassandra\Type\UnsupportedType;
use Cassandra\Uuid;
use Cassandra\Varint;
use InvalidArgumentException;
use stdClass;

require_once __DIR__ . '/../Type/UnsupportedType.php';

const SET_FLOAT_CLASS = '\\Cassandra\\Float';

it('throws on invalid type for set', function () {
    new Set(new stdClass());
})->throws(
    InvalidArgumentException::class,
    'type must be a string or an instance of Cassandra\\Type, an instance of stdClass given',
);

it('throws on unsupported string type for set', function () {
    new Set('custom type');
})->throws(InvalidArgumentException::class, "Unsupported type 'custom type'");

it('throws on unsupported type for set', function () {
    new Set(new UnsupportedType());
})->throws(
    InvalidArgumentException::class,
    'type must be a valid Cassandra\\Type, an instance of Cassandra\\Type\\UnsupportedType given',
);

it('contains unique values', function () {
    $set = new Set(Cassandra::TYPE_VARINT);
    expect(count($set))->toEqual(0);
    $set->add(new Varint('123'));
    expect(count($set))->toEqual(1);
    $set->add(new Varint('123'));
    expect(count($set))->toEqual(1);
});

it('supports scalar keys for set', function ($type, $values) {
    $set = Type::set($type)->create();
    foreach ($values as $value) {
        $set->add($value);
    }
    expect(count($set))->toEqual(count($values));

    foreach ($values as $value) {
        expect($set->has($value))->toBeTrue();
    }

    foreach ($values as $value) {
        $set->remove($value);
    }
    expect(count($set))->toEqual(0);
})->with(function () {
    $cls = SET_FLOAT_CLASS;
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

it('supports composite keys for set', function ($type) {
    $map = Type::set($type)->create();

    $map->add($type->create('a', '1', 'b', '2'));
    expect($map->has($type->create('a', '1', 'b', '2')))->toBeTrue()
        ->and(count($map))->toEqual(1);

    $map->add($type->create('c', '3', 'd', '4', 'e', '5'));
    expect($map->has($type->create('c', '3', 'd', '4', 'e', '5')))->toBeTrue()
        ->and(count($map))->toEqual(2);

    $map->remove($type->create('a', '1', 'b', '2'));
    expect($map->has($type->create('a', '1', 'b', '2')))->toBeFalse()
        ->and(count($map))->toEqual(1);

    $map->remove($type->create('c', '3', 'd', '4', 'e', '5'));
    expect($map->has($type->create('c', '3', 'd', '4', 'e', '5')))->toBeFalse()
        ->and(count($map))->toEqual(0);
})->with([
    [Type::map(Type::varchar(), Type::varchar())],
    [Type::set(Type::varchar())],
    [Type::collection(Type::varchar())],
]);

it('supports only Cassandra types', function () {
    new Set('some custom type');
})->throws(InvalidArgumentException::class, "Unsupported type 'some custom type'");

dataset('set_cassandra_types', [
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

it('supports all Cassandra types for set', function ($type) {
    $var = new Set($type);
    expect((string) $var->type())->toEqual("set<$type>");
})->with('set_cassandra_types');

it('returns its type', function ($type) {
    $set = new Set($type);
    expect($set->type()->valueType())->toEqual($type);
})->with('set_cassandra_types');

it('validates types of elements in set', function () {
    $set = new Set(Cassandra::TYPE_VARINT);
    $set->add(new Decimal('123'));
})->throws(
    InvalidArgumentException::class,
    'argument must be an instance of Cassandra\\Varint, an instance of Cassandra\\Decimal given',
);

it('rejects null values in set', function () {
    $set = new Set(Cassandra::TYPE_VARINT);
    $set->add(null);
})->throws(InvalidArgumentException::class, 'Invalid value: null is not supported inside sets');

dataset('set_sample_numbers', [
    [[1, 2, 3, 4, 5, 6, 7, 8, 9]],
]);

it('supports iteration on set', function ($numbers) {
    $set = new Set(Cassandra::TYPE_INT);
    foreach ($numbers as $number) {
        $set->add($number);
    }
    expect(count($set))->toEqual(count($numbers));

    foreach ($set as $i => $number) {
        expect($number)->toEqual($numbers[$i]);
    }
})->with('set_sample_numbers');

it('supports conversion to array', function ($numbers) {
    $set = new Set(Cassandra::TYPE_INT);
    foreach ($numbers as $number) {
        $set->add($number);
    }
    expect($set->values())->toEqual($numbers);
})->with('set_sample_numbers');

it('resumes iteration after converting to array', function ($numbers) {
    $set = new Set(Cassandra::TYPE_INT);
    foreach ($numbers as $number) {
        $set->add($number);
    }

    $set->rewind();

    expect($set->current())->toEqual(1);
    $set->next();
    expect($set->current())->toEqual(2);
    $set->next();
    expect($set->current())->toEqual(3);

    $set->values();

    $set->next();
    expect($set->current())->toEqual(4);
    $set->next();
    expect($set->current())->toEqual(5);
    $set->next();
    expect($set->current())->toEqual(6);
})->with('set_sample_numbers');

it('supports retrieving values', function () {
    $values = [
        new Varint('1'), new Varint('2'), new Varint('3'),
        new Varint('4'), new Varint('5'), new Varint('6'),
        new Varint('7'), new Varint('8'),
    ];
    $set = new Set(Cassandra::TYPE_VARINT);
    foreach ($values as $value) {
        $set->add($value);
    }
    expect($set->values())->toEqual($values);
});

it('compares equal sets as equal', function ($value1, $value2) {
    expect($value2)->toEqual($value1)
        ->and($value1 == $value2)->toBeTrue();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [Type::set(Type::int())->create(), Type::set(Type::int())->create()],
        [Type::set(Type::int())->create(1, 2, 3), Type::set(Type::int())->create(1, 2, 3)],
        [Type::set(Type::varchar())->create('a', 'b', 'c'), Type::set(Type::varchar())->create('a', 'b', 'c')],
        [Type::set($setType)->create($setType->create(1, 2, 3)), Type::set($setType)->create($setType->create(1, 2, 3))],
    ];
});

it('compares non-equal sets as not equal', function ($value1, $value2) {
    expect($value2)->not->toEqual($value1)
        ->and($value1 == $value2)->toBeFalse();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [Type::set(Type::int())->create(), Type::set(Type::varchar())->create()],
        [Type::set(Type::int())->create(1, 2, 3), Type::set(Type::int())->create(4, 5, 6)],
        [Type::set(Type::int())->create(1, 2, 3), Type::set(Type::int())->create(1)],
        [Type::set(Type::varchar())->create('a', 'b', 'c'), Type::set(Type::varchar())->create('a', 'b', 'd')],
        [Type::set($setType)->create($setType->create(1, 2, 3)), Type::set($setType)->create($setType->create(4, 5, 6))],
    ];
});

it('compares sets of different counts via lt/gt', function () {
    expect(
        Type::set(Type::int())->create(1) <
        Type::set(Type::int())->create(1, 2),
    )->toBeTrue()
        ->and(
            Type::set(Type::int())->create(1, 2) >
            Type::set(Type::int())->create(1),
        )->toBeTrue();
});
