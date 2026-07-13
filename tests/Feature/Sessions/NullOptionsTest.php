<?php

declare(strict_types=1);

use Cassandra\FuturePreparedStatement;
use Cassandra\FutureRows;
use Cassandra\PreparedStatement;
use Cassandra\Rows;
use Cassandra\SimpleStatement;

/*
 * Regression coverage for he4rt/scylladb-php-driver#108.
 *
 * The stub declares `$options` as `array|ExecutionOptions|null = null`, so
 * passing an explicit `null` is valid userland. Internally the methods guard
 * on `if (options)` (pointer non-null), but `Z_PARAM_ZVAL` yields a non-null
 * pointer of type IS_NULL for an explicit `null`, which then failed the
 * IS_ARRAY / IS_OBJECT type check and threw an InvalidArgumentException.
 *
 * The fix is `Z_PARAM_ZVAL_OR_NULL`, which collapses an explicit `null` to a
 * real nullptr so the guard behaves the same as omitting the argument.
 *
 * These tests assert that explicit `null` behaves identically to omitting the
 * argument, across every session entry point that accepts $options.
 */

$keyspace = 'null_options_test';

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE items (id int PRIMARY KEY, payload text)
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('execute() accepts an explicit null $options', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute(new SimpleStatement('SELECT id FROM items'), null);

    expect($rows)->toBeInstanceOf(Rows::class);
    expect($rows->count())->toBe(0);
})->group('feature', 'sessions');

it('executeAsync() accepts an explicit null $options', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $future = $session->executeAsync(new SimpleStatement('SELECT id FROM items'), null);

    expect($future)->toBeInstanceOf(FutureRows::class);
    expect($future->get()->count())->toBe(0);
})->group('feature', 'sessions');

it('prepare() accepts an explicit null $options', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $prepared = $session->prepare('SELECT id FROM items WHERE id = ?', null);

    expect($prepared)->toBeInstanceOf(PreparedStatement::class);
})->group('feature', 'sessions');

it('prepareAsync() accepts an explicit null $options', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $future = $session->prepareAsync('SELECT id FROM items WHERE id = ?', null);

    expect($future)->toBeInstanceOf(FuturePreparedStatement::class);
    expect($future->get())->toBeInstanceOf(PreparedStatement::class);
})->group('feature', 'sessions');

it('explicit null $options behaves the same as omitting it', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $omitted  = $session->execute(new SimpleStatement('SELECT id FROM items'));
    $explicit = $session->execute(new SimpleStatement('SELECT id FROM items'), null);

    expect($omitted->count())->toBe($explicit->count());
})->group('feature', 'sessions');

it('still accepts an array $options after the null fix', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute(
        new SimpleStatement('SELECT id FROM items'),
        ['page_size' => 10],
    );

    expect($rows)->toBeInstanceOf(Rows::class);
})->group('feature', 'sessions');
