<?php

declare(strict_types=1);

use Cassandra\BatchStatement;
use Cassandra\ExecutionOptions;
use Cassandra\SimpleStatement;

$keyspace = 'statement_idempotence';
$table    = 'counters';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    USE $keyspace;
    CREATE TABLE $table (
        id    int,
        value text,
        PRIMARY KEY (id)
    )
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

it('executes a statement marked idempotent', function () use ($keyspace, $table) {
    $session   = scyllaDbConnection($keyspace);
    $statement = (new SimpleStatement("SELECT * FROM $table"))->setIdempotent();

    expect($session->execute($statement)->count())->toBe(0);
})->group('feature');

it('executes a prepared statement marked idempotent', function () use ($keyspace, $table) {
    $session   = scyllaDbConnection($keyspace);
    $statement = $session->prepare("INSERT INTO $table (id, value) VALUES (?, ?)");

    expect($statement->setIdempotent()->isIdempotent())->toBeTrue();

    $session->execute($statement, ['arguments' => [1, 'one']]);

    expect($session->execute(new SimpleStatement("SELECT * FROM $table WHERE id = 1"))->count())->toBe(1);
})->group('feature');

it('accepts idempotence as a per-call execution option', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute("SELECT * FROM $table", ['idempotent' => true]);

    expect($rows)->not->toBeNull();
})->group('feature');

it('lets a per-call option override the statement default', function () use ($keyspace, $table) {
    $session   = scyllaDbConnection($keyspace);
    $statement = (new SimpleStatement("SELECT * FROM $table"))->setIdempotent(false);

    $rows = $session->execute($statement, ['idempotent' => true]);

    expect($rows->count())->toBe(1);
    expect($statement->isIdempotent())->toBeFalse();
})->group('feature');

it('accepts idempotence through an ExecutionOptions object', function () use ($keyspace, $table) {
    error_reporting(E_ALL ^ E_DEPRECATED);
    $session = scyllaDbConnection($keyspace);

    $rows = $session->execute("SELECT * FROM $table", new ExecutionOptions(['idempotent' => true]));

    expect($rows->count())->toBe(1);
    error_reporting(E_ALL);
})->group('feature');

it('executes an idempotent batch', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $batch   = new BatchStatement();

    $batch->add("INSERT INTO $table (id, value) VALUES (2, 'two')");
    $batch->add("INSERT INTO $table (id, value) VALUES (3, 'three')");
    $batch->setIdempotent();

    $session->execute($batch);

    expect($session->execute(new SimpleStatement("SELECT * FROM $table"))->count())->toBe(3);
})->group('feature');

it('executes an idempotent statement asynchronously', function () use ($keyspace, $table) {
    $session   = scyllaDbConnection($keyspace);
    $statement = (new SimpleStatement("SELECT * FROM $table"))->setIdempotent();

    expect($session->executeAsync($statement)->get()->count())->toBe(3);
})->group('feature');
