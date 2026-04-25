<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;

it('allows creating scalar values from types', function () {
    expect(Type::varchar()->create('some string'))->toEqual('some string');
});

it('compares equal scalar types', function ($type) {
    expect($type() == $type())->toBeTrue();
})->with([
    [fn () => Type::ascii()],
    [fn () => Type::bigint()],
    [fn () => Type::blob()],
    [fn () => Type::boolean()],
    [fn () => Type::counter()],
    [fn () => Type::decimal()],
    [fn () => Type::double()],
    [fn () => Type::float()],
    [fn () => Type::inet()],
    [fn () => Type::int()],
    [fn () => Type::text()],
    [fn () => Type::timestamp()],
    [fn () => Type::timeuuid()],
    [fn () => Type::uuid()],
    [fn () => Type::varchar()],
    [fn () => Type::varint()],
]);

it('compares not-equal scalar types', function () {
    expect(Type::ascii() != Type::bigint())->toBeTrue();
});
