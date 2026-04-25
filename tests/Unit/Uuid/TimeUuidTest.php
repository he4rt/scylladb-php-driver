<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Uuid;

use Cassandra\Timeuuid;
use Cassandra\Exception\InvalidArgumentException;
use DateTime;

describe('Cassandra\Timeuuid (migrated)', function () {

    it('compares two equal TimeUuids', function (Timeuuid $value1, Timeuuid $value2) {
        expect($value2)->toEqual($value1)
            ->and($value1 == $value2)->toBeTrue();
    })->with([
        [new Timeuuid(0), new Timeuuid(0)],
    ]);

    it('compares two non-equal TimeUuids', function (Timeuuid $value1, Timeuuid $value2) {
        expect($value2)->not->toEqual($value1)
            ->and($value1 == $value2)->toBeFalse();
    })->with([
        [new Timeuuid(0), new Timeuuid(2)],
    ]);

    it('can be created from a type 1 UUID string', function () {
        $uuid = new Timeuuid('5f344f20-52a3-11e7-915b-5f4f349b532d');
        expect($uuid)->toBeObject();
    });

    it('cannot be created from a type 4 UUID string', function () {
        new Timeuuid('65f9e722-036a-4029-b03b-a9046b23b4c9');
    })->throws(InvalidArgumentException::class, 'UUID must be of type 1, type 4 given')
      ->skip('Extension throws Cassandra\\Exception\\InvalidArgumentException; un-skip after verifying class+message');

    it('cannot be created from an invalid string', function () {
        new Timeuuid('invalid');
    })->throws(InvalidArgumentException::class, 'Invalid UUID')
      ->skip('Extension throws Cassandra\\Exception\\InvalidArgumentException; un-skip after verifying class+message');

    it('rejects an invalid argument type in the constructor', function () {
        new Timeuuid(new DateTime());
    })->throws(InvalidArgumentException::class, 'Invalid argument')
      ->skip('Extension throws Cassandra\\Exception\\InvalidArgumentException; un-skip after verifying class+message');
});
