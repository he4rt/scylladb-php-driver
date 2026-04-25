<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Collections;

use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Type;
use Cassandra\UserTypeValue;
use Cassandra\Uuid;
use Cassandra\Varint;
use InvalidArgumentException;

const UTV_FLOAT_CLASS = '\\Cassandra\\Float';

it('supports only Cassandra types in UDT', function () {
    new UserTypeValue(['name1' => 'invalid type']);
})->throws(InvalidArgumentException::class, "Unsupported type 'invalid type'");

dataset('utv_cassandra_types', [
    [\Cassandra::TYPE_ASCII],
    [\Cassandra::TYPE_BIGINT],
    [\Cassandra::TYPE_BLOB],
    [\Cassandra::TYPE_BOOLEAN],
    [\Cassandra::TYPE_COUNTER],
    [\Cassandra::TYPE_DECIMAL],
    [\Cassandra::TYPE_DOUBLE],
    [\Cassandra::TYPE_FLOAT],
    [\Cassandra::TYPE_INT],
    [\Cassandra::TYPE_TEXT],
    [\Cassandra::TYPE_TIMESTAMP],
    [\Cassandra::TYPE_UUID],
    [\Cassandra::TYPE_VARCHAR],
    [\Cassandra::TYPE_VARINT],
    [\Cassandra::TYPE_TIMEUUID],
    [\Cassandra::TYPE_INET],
]);

it('supports all Cassandra types for UDT', function ($type) {
    $result = new UserTypeValue(['name1' => $type]);
    expect($result)->toBeInstanceOf(UserTypeValue::class);
})->with('utv_cassandra_types');

it('returns its UDT type', function ($type) {
    $udt   = new UserTypeValue(['name1' => $type]);
    $types = $udt->type()->types();
    expect($types['name1'])->toEqual($type);
})->with('utv_cassandra_types');

it('supports scalar values in UDT', function ($type, $value, $valueCopy) {
    $udt = new UserTypeValue(['name1' => $type]);
    $udt->set('name1', $value);
    expect(count($udt))->toEqual(1)
        ->and($udt->get('name1'))->toEqual($valueCopy);
})->with(function () {
    $cls = UTV_FLOAT_CLASS;
    return [
        [Type::ascii(), 'ascii', 'ascii'],
        [Type::bigint(), new Bigint('9223372036854775807'), new Bigint('9223372036854775807')],
        [Type::blob(), new Blob('blob'), new Blob('blob')],
        [Type::boolean(), true, true],
        [Type::counter(), new Bigint(123), new Bigint(123)],
        [Type::decimal(), new Decimal('3.14159265359'), new Decimal('3.14159265359')],
        [Type::double(), 3.14159, 3.14159],
        [Type::float(), new $cls(3.14159), new $cls(3.14159)],
        [Type::inet(), new Inet('127.0.0.1'), new Inet('127.0.0.1')],
        [Type::int(), 123, 123],
        [Type::text(), 'text', 'text'],
        [Type::timestamp(), new Timestamp(123), new Timestamp(123)],
        [Type::timeuuid(), new Timeuuid(0), new Timeuuid(0)],
        [Type::uuid(), new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c2'), new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c2')],
        [Type::varchar(), 'varchar', 'varchar'],
        [Type::varint(), new Varint('9223372036854775808'), new Varint('9223372036854775808')],
        [Type::duration(), new Duration(1, 2, 3), new Duration(1, 2, 3)],
    ];
});

it('sets and gets fields on UDT', function () {
    $fields = ['a' => Type::int(), 'b' => Type::text(), 'c' => Type::bigint()];
    $udt    = new UserTypeValue($fields);
    $udt->set('a', 1);
    $udt->set('b', 'xyz');
    $udt->set('c', new Bigint('123'));

    expect($udt->get('a'))->toEqual(1)
        ->and($udt->get('b'))->toEqual('xyz')
        ->and($udt->get('c'))->toEqual(new Bigint('123'));

    $other = new UserTypeValue($fields);
    $other->set('a', 1);
    $other->set('b', 'xyz');
    $other->set('c', new Bigint('123'));

    expect($other)->toEqual($udt);
});

it('UDT equals when values match', function () {
    $fields = ['a' => Type::int(), 'b' => Type::text(), 'c' => Type::bigint()];

    $udt = new UserTypeValue($fields);
    $udt->set('a', 1);
    $udt->set('b', 'xyz');
    $udt->set('c', new Bigint('123'));

    expect(count($udt))->toEqual(3);

    $other = new UserTypeValue($fields);
    $other->set('a', 1);
    $other->set('b', 'xyz');
    $other->set('c', new Bigint('123'));

    expect($other)->toEqual($udt);
});

it('throws on UDT set invalid name', function () {
    $udt = new UserTypeValue(['name1' => Type::int()]);
    $udt->set('invalid', 42);
})->throws(InvalidArgumentException::class, "Invalid name 'invalid'");

it('throws on UDT get invalid name', function () {
    $udt = new UserTypeValue(['name1' => Type::int()]);
    $udt->set('name1', 42);
    $udt->get('invalid');
})->throws(InvalidArgumentException::class, "Invalid name 'invalid'");

it('throws on UDT invalid value type', function () {
    $udt = new UserTypeValue(['name1' => Type::int()]);
    $udt->set('name1', 'text');
})->throws(InvalidArgumentException::class, "argument must be an int, 'text' given");

it('compares equal UDTs as equal', function ($value1, $value2) {
    expect($value2)->toEqual($value1)
        ->and($value1 == $value2)->toBeTrue();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create(),
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create(),
        ],
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create('a', 1, 'b', 'x', 'c', new Bigint(99)),
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create('a', 1, 'b', 'x', 'c', new Bigint(99)),
        ],
        [
            Type::userType('a', $setType, 'b', Type::varchar())->create('a', $setType->create(1, 2, 3), 'b', 'x'),
            Type::userType('a', $setType, 'b', Type::varchar())->create('a', $setType->create(1, 2, 3), 'b', 'x'),
        ],
    ];
});

it('compares non-equal UDTs as not equal', function ($value1, $value2) {
    expect($value2)->not->toEqual($value1)
        ->and($value1 == $value2)->toBeFalse();
})->with(function () {
    $setType = Type::set(Type::int());
    return [
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::varint())->create(),
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create(),
        ],
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create(),
            Type::userType('x', Type::int(), 'y', Type::varchar(), 'z', Type::bigint())->create(),
        ],
        [
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create('a', 1, 'b', 'x', 'c', new Bigint(99)),
            Type::userType('a', Type::int(), 'b', Type::varchar(), 'c', Type::bigint())->create('a', 2, 'b', 'y', 'c', new Bigint(999)),
        ],
        [
            Type::userType('a', $setType, 'b', Type::varchar())->create('a', $setType->create(1, 2, 3), 'b', 'x'),
            Type::userType('a', $setType, 'b', Type::varchar())->create('a', $setType->create(4, 5, 6), 'b', 'x'),
        ],
    ];
});
