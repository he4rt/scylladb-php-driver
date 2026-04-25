<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Schema;

use Cassandra;

/**
 * Migrated from tests-old/features/risky/function_and_aggregate_metadata.feature.
 *
 * The original feature targeted Cassandra >= 2.2 and exercised User-Defined
 * Functions (UDFs) and User-Defined Aggregates (UDAs) in Java. ScyllaDB does
 * NOT support UDFs/UDAs (it ships with experimental Lua-only support that is
 * neither API- nor metadata-compatible with Cassandra Java UDFs), so every
 * scenario is skipped here. The bodies are preserved verbatim so this file
 * can be opted-in for Cassandra-only test runs in the future.
 *
 * To un-skip: target a Cassandra (not ScyllaDB) cluster with `enable_user_
 * defined_functions: true` in cassandra.yaml, and gate on the connected
 * server flavour.
 */

$keyspace = 'simplex_udf_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    // The fixture DDL is preserved for future use — currently a no-op
    // because every test below is skipped.
    // migrateKeyspace(<<<CQL
    // CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1} AND DURABLE_WRITES = false;
    // CREATE OR REPLACE FUNCTION $keyspace.fLog (input double) CALLED ON NULL INPUT RETURNS double LANGUAGE java AS 'return Double.valueOf(Math.log(input.doubleValue()));';
    // CREATE OR REPLACE FUNCTION $keyspace.avgState (state tuple<int,bigint>, val int) CALLED ON NULL INPUT RETURNS tuple<int,bigint> LANGUAGE java AS 'if (val !=null) { state.setInt(0, state.getInt(0)+1); state.setLong(1, state.getLong(1)+val.intValue()); } return state;';
    // CREATE OR REPLACE FUNCTION $keyspace.avgFinal (state tuple<int,bigint>) CALLED ON NULL INPUT RETURNS double LANGUAGE java AS 'double r = 0; if (state.getInt(0) == 0) return null; r = state.getLong(1); r/= state.getInt(0); return Double.valueOf(r);';
    // CREATE AGGREGATE IF NOT EXISTS $keyspace.average (int) SFUNC avgState STYPE tuple<int,bigint> FINALFUNC avgFinal INITCOND (0,0);
    // CQL);
});

afterAll(function () use ($keyspace) {
    try {
        dropKeyspace($keyspace);
    } catch (\Throwable) {
        // ignore — keyspace was never created when skipped.
    }
});

it("exposes a User-Defined Function's metadata", function () use ($keyspace) {
    test()->markTestSkipped('ScyllaDB does not support UDFs/UDAs');

    $session  = scyllaDbConnection($keyspace);
    $schema   = $session->schema();
    $function = $schema->keyspace($keyspace)->function('flog', Cassandra\Type::double());

    expect($function->simpleName())->toBe('flog')
        ->and($function->signature())->toBe('flog(double)')
        ->and($function->language())->toBe('java')
        ->and($function->body())->toBe('return Double.valueOf(Math.log(input.doubleValue()));')
        ->and($function->isCalledOnNullInput())->toBeTrue();

    $args = $function->arguments();
    expect($args)->toHaveKey('input')
        ->and((string) $args['input'])->toBe('double')
        ->and((string) $function->returnType())->toBe('double');
});

it("exposes a User-Defined Aggregate's metadata", function () use ($keyspace) {
    test()->markTestSkipped('ScyllaDB does not support UDFs/UDAs');

    $session   = scyllaDbConnection($keyspace);
    $schema    = $session->schema();
    $aggregate = $schema->keyspace($keyspace)->aggregate('average', Cassandra\Type::int());

    expect($aggregate->simpleName())->toBe('average')
        ->and($aggregate->signature())->toBe('average(int)');

    $argTypes = $aggregate->argumentTypes();
    expect($argTypes)->toHaveCount(1)
        ->and((string) $argTypes[0])->toBe('int')
        ->and((string) $aggregate->stateType())->toBe('tuple<int, bigint>')
        ->and((string) $aggregate->returnType())->toBe('double')
        ->and($aggregate->stateFunction()->signature())->toBe('avgstate(frozen<tuple<int,bigint>>,int)')
        ->and($aggregate->finalFunction()->signature())->toBe('avgfinal(frozen<tuple<int,bigint>>)');

    expect($aggregate->initialCondition())->not->toBeNull();
});
