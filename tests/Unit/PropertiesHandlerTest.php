<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Collection;
use Cassandra\Date;
use Cassandra\Inet;
use Cassandra\Map;
use Cassandra\Set;
use Cassandra\Tuple;
use Cassandra\Type;
use Cassandra\UserTypeValue;
use Cassandra\Uuid;

function iterationLength(object $value, int $cap = 200): int
{
    $seen = 0;

    foreach ($value as $key => $unused) {
        $reread = (array) $value;
        unset($reread);

        if (++$seen > $cap) {
            return $cap + 1;
        }
    }

    return $seen;
}

dataset('propertyBearingValues', [
    'Type\Scalar'      => [fn () => Type::text()],
    'Type\Collection'  => [fn () => Type::collection(Type::text())],
    'Type\Map'         => [fn () => Type::map(Type::int(), Type::text())],
    'Type\Set'         => [fn () => Type::set(Type::int())],
    'Type\Tuple'       => [fn () => Type::tuple(Type::int(), Type::text())],
    'Uuid'             => [fn () => new Uuid()],
    'Timeuuid'         => [fn () => new Cassandra\Timeuuid()],
    'Inet'             => [fn () => new Inet('127.0.0.1')],
    'Blob'             => [fn () => new Blob('hi')],
    'Bigint'           => [fn () => new Bigint(1)],
    'Varint'           => [fn () => new Cassandra\Varint('1')],
    'Decimal'          => [fn () => new Cassandra\Decimal('1.5')],
    'Float'            => [fn () => new ("Cassandra\\Float")(1.5)],
    'Smallint'         => [fn () => new Cassandra\Smallint(1)],
    'Tinyint'          => [fn () => new Cassandra\Tinyint(1)],
    'Date'             => [fn () => new Date(0)],
    'Time'             => [fn () => new Cassandra\Time(0)],
    'Timestamp'        => [fn () => new Cassandra\Timestamp(0, 0)],
    'Duration'         => [fn () => new Cassandra\Duration(1, 2, 3)],
    'Collection'       => [fn () => (function () { $c = new Collection(Type::int()); $c->add(1, 2); return $c; })()],
    'Map'              => [fn () => (function () { $m = new Map(Type::int(), Type::int()); $m->set(1, 2); return $m; })()],
    'Set'              => [fn () => (function () { $s = new Set(Type::int()); $s->add(1); return $s; })()],
    'Tuple'            => [fn () => new Tuple([Type::int()])],
    'UserTypeValue'    => [fn () => (function () { $u = new UserTypeValue(['a' => Type::int()]); $u->set('a', 5); return $u; })()],
    'Cluster\Builder'  => [fn () => Cassandra::cluster()],
]);

it('finishes a foreach whose body reads the same object', function ($value) {
    expect(iterationLength($value))->toBeLessThanOrEqual(64);
})->with('propertyBearingValues');

it('keeps handing back the same property table', function ($value) {
    $first  = (array) $value;
    $second = (array) $value;

    expect($second)->toEqual($first);
})->with('propertyBearingValues');

it('still reports a container mutation made after the table was built', function () {
    $collection = new Collection(Type::int());
    expect(((array) $collection)['values'])->toBe([]);

    $collection->add(7);
    expect(((array) $collection)['values'])->toBe([7]);

    $collection->remove(0);
    expect(((array) $collection)['values'])->toBe([]);
});

it('still reports a builder setting changed after the table was built', function () {
    $builder = Cassandra::cluster();
    expect(((array) $builder)['defaultPageSize'])->not->toBe(4242);

    $builder->withDefaultPageSize(4242);
    expect(((array) $builder)['defaultPageSize'])->toBe(4242);
});
