<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra\Future;
use Cassandra\FutureClose;
use Cassandra\FuturePreparedStatement;
use Cassandra\FutureRows;
use Cassandra\FutureSession;
use Cassandra\FutureValue;
use ReflectionClass;

/*
 * https://github.com/he4rt/scylladb-php-driver/issues/155 — a directly
 * constructed Future had no CassFuture, and get() dereferenced it.
 */

dataset('future classes', [
    'rows'               => FutureRows::class,
    'prepared statement' => FuturePreparedStatement::class,
    'close'              => FutureClose::class,
    'session'            => FutureSession::class,
    'value'              => FutureValue::class,
]);

it('refuses direct construction', function (string $class) {
    expect(fn () => new $class())->toThrow(\Error::class);
})->with('future classes');

it('keeps the constructor private and the class final', function (string $class) {
    $reflection = new ReflectionClass($class);

    expect($reflection->isFinal())->toBeTrue();
    expect($reflection->implementsInterface(Future::class))->toBeTrue();
    expect($reflection->getConstructor()?->isPrivate())->toBeTrue();
})->with('future classes');
