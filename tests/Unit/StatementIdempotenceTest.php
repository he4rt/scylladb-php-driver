<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra\BatchStatement;
use Cassandra\SimpleStatement;
use Cassandra\Statement;

it('reports no explicit setting on a fresh statement', function (Statement $statement) {
    expect($statement->isIdempotent())->toBeNull();
})->with([
    'simple' => fn () => new SimpleStatement('SELECT * FROM system.local'),
    'batch'  => fn () => new BatchStatement(),
]);

it('marks a statement idempotent and reads the flag back', function (Statement $statement) {
    expect($statement->setIdempotent()->isIdempotent())->toBeTrue();
    expect($statement->setIdempotent(false)->isIdempotent())->toBeFalse();
    expect($statement->setIdempotent(true)->isIdempotent())->toBeTrue();
})->with([
    'simple' => fn () => new SimpleStatement('SELECT * FROM system.local'),
    'batch'  => fn () => new BatchStatement(),
]);

it('returns the same instance so calls chain', function () {
    $statement = new SimpleStatement('SELECT * FROM system.local');

    expect($statement->setIdempotent())->toBe($statement);
});

it('keeps the flag per statement instance', function () {
    $first  = new SimpleStatement('SELECT * FROM system.local');
    $second = new SimpleStatement('SELECT * FROM system.local');

    $first->setIdempotent();

    expect($first->isIdempotent())->toBeTrue();
    expect($second->isIdempotent())->toBeNull();
});

it('declares the pair on the Statement interface', function () {
    expect((new SimpleStatement('SELECT 1')))->toBeInstanceOf(Statement::class);
    expect(method_exists(Statement::class, 'setIdempotent'))->toBeTrue();
    expect(method_exists(Statement::class, 'isIdempotent'))->toBeTrue();
});

