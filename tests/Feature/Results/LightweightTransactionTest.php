<?php

declare(strict_types=1);

use Cassandra\SimpleStatement;

$keyspace = 'lwt_was_applied';
$table    = 'users';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE $table (
        id    int PRIMARY KEY,
        email text
    )
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('reports true when a conditional insert succeeds', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute(
        new SimpleStatement("INSERT INTO $table (id, email) VALUES (1, 'ada@example.com') IF NOT EXISTS")
    );

    expect($rows->wasApplied())->toBeTrue()
        ->and($rows->first()['[applied]'])->toBeTrue();
})->group('feature');

it('reports false and exposes the current values when a conditional insert loses', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $session->execute(
        new SimpleStatement("INSERT INTO $table (id, email) VALUES (2, 'grace@example.com') IF NOT EXISTS")
    );

    $rows = $session->execute(
        new SimpleStatement("INSERT INTO $table (id, email) VALUES (2, 'ada@example.com') IF NOT EXISTS")
    );

    expect($rows->wasApplied())->toBeFalse()
        ->and($rows->first()['email'])->toBe('grace@example.com');
})->group('feature');

it('reports false when a conditional update finds no row', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute(
        new SimpleStatement("UPDATE $table SET email = 'nobody@example.com' WHERE id = 999 IF EXISTS")
    );

    expect($rows->wasApplied())->toBeFalse();
})->group('feature');

it('reports false when the IF condition does not match', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $session->execute(new SimpleStatement("INSERT INTO $table (id, email) VALUES (3, 'lovelace@example.com')"));

    $rows = $session->execute(
        new SimpleStatement("UPDATE $table SET email = 'x@example.com' WHERE id = 3 IF email = 'wrong@example.com'")
    );

    expect($rows->wasApplied())->toBeFalse()
        ->and($rows->first()['email'])->toBe('lovelace@example.com');
})->group('feature');

it('reports true for a non-conditional write', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute(
        new SimpleStatement("INSERT INTO $table (id, email) VALUES (4, 'plain@example.com')")
    );

    expect($rows->count())->toBe(0)
        ->and($rows->wasApplied())->toBeTrue();
})->group('feature');

it('reports true for a SELECT that has no [applied] column', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute(new SimpleStatement("SELECT * FROM $table WHERE id = 1"));

    expect($rows->count())->toBe(1)
        ->and($rows->wasApplied())->toBeTrue();
})->group('feature');

it('is available on a result fetched through executeAsync', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->executeAsync(
        new SimpleStatement("INSERT INTO $table (id, email) VALUES (5, 'async@example.com') IF NOT EXISTS")
    )->get();

    expect($rows->wasApplied())->toBeTrue();
})->group('feature', 'async');
