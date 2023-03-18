<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Cluster;

use Cassandra;
use Cassandra\Exception\InvalidArgumentException;

use function PHPUnit\Framework\assertFalse;
use function PHPUnit\Framework\assertTrue;

//test('test', function () {
//   assertTrue(true);
//});
//
test('Cassandra Cluster Builder -> withDefaultConsistency | valid', function (int $consistency) {
    $builder = Cassandra::cluster()->withDefaultConsistency($consistency);

    expect($builder)
        ->toBeInstanceOf(Cassandra\Cluster\Builder::class)
        ->and($builder->build())
        ->toBeInstanceOf(Cassandra\Cluster::class);
})->with([
    Cassandra::CONSISTENCY_ANY,
    Cassandra::CONSISTENCY_ONE,
    Cassandra::CONSISTENCY_TWO,
    Cassandra::CONSISTENCY_THREE,
    Cassandra::CONSISTENCY_QUORUM,
    Cassandra::CONSISTENCY_ALL,
    Cassandra::CONSISTENCY_LOCAL_QUORUM,
    Cassandra::CONSISTENCY_EACH_QUORUM,

    Cassandra::CONSISTENCY_SERIAL,
    Cassandra::CONSISTENCY_LOCAL_SERIAL,
]);
//
test('Cassandra Cluster Builder -> withDefaultConsistency | invalid consistency', function () {
    Cassandra::cluster()->withDefaultConsistency(100);
})->throws(
    Cassandra\Exception\InvalidArgumentException::class,
    'consistency must be one of Cassandra::CONSISTENCY_*, 100 given'
);

test('Cassandra Cluster Builder -> withDefaultPageSize | invalid', function () {
    Cassandra::cluster()->withDefaultPageSize(-1);
})->throws(
    InvalidArgumentException::class,
    'pageSize must be a positive integer, -1 given',
);
//
test('Cassandra Cluster Builder -> withDefaultPageSize | valid', function () {
    $builder = Cassandra::cluster()->withDefaultPageSize(100);

    expect($builder)
        ->toBeInstanceOf(Cassandra\Cluster\Builder::class)
        ->and($builder->build())
        ->toBeInstanceOf(Cassandra\Cluster::class);
});

test('Cassandra Cluster Builder -> withDefaultTimeout | invalid', function () {
    Cassandra::cluster()->withDefaultTimeout(-1.0);
})->throws(
    InvalidArgumentException::class,
    'timeout must be a positive number or null, -1 given'
);
//
//
test('Cassandra Cluster Builder -> withDefaultTimeout | valid', function (?float $timeout) {
    expect(Cassandra::cluster()->withDefaultTimeout($timeout))
        ->toBeInstanceOf(Cassandra\Cluster\Builder::class)
        ->not
        ->toBeNull();
})->with([1.0, null]);

//
test('Cassandra Cluster Builder -> withDefaultTimeout | set timeout twice', function () {
    $builder = Cassandra::cluster()
        ->withDefaultTimeout(1.0);

    $builder->withDefaultTimeout(2.0);

    assertTrue(true);
});

//
test('Cassandra Cluster Builder -> withDefaultTimeout | set to undef', function () {
    $builder = Cassandra::cluster()
        ->withDefaultTimeout(1.0);

    $builder->withDefaultTimeout(null);
    assertTrue(true);
});