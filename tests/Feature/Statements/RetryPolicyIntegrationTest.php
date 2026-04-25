<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

use Cassandra;
use Cassandra\BatchStatement;
use Cassandra\Exception\TimeoutException;
use Cassandra\Exception\UnavailableException;
use Cassandra\RetryPolicy;
use Cassandra\Session;

const RP_NUMBER_OF_INSERTS = 25;
const RP_NUMBER_OF_TIMEOUT_RETRIES = 5;
const RP_BATCH_STATEMENT = 1;
const RP_PREPARED_STATEMENT = 2;
const RP_SIMPLE_STATEMENT = 3;

$keyspace = 'retrypolicy_int_' . substr(bin2hex(random_bytes(4)), 0, 8);
$table = 'retry_policy';
$insertQuery = '';

beforeAll(function () use ($keyspace, $table, &$insertQuery) {
    // RF = 3 against a single node so consistency requirements deliberately fail.
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 3}
    CQL);

    migrateKeyspace(
        "CREATE TABLE $keyspace.$table (key int, value_int int, PRIMARY KEY(key, value_int))"
    );

    $insertQuery = "INSERT INTO $keyspace.$table (key, value_int) VALUES (?, ?)";
});

afterAll(fn () => dropKeyspace($keyspace));

/**
 * @param int $statementType
 * @param int $key
 * @param int $numberOfInserts
 * @param int $consistency
 */
function rpInsert(
    Session $session,
    string $insertQuery,
    int $statementType,
    RetryPolicy $policy,
    int $key,
    int $numberOfInserts,
    int $consistency,
    int $retries = RP_NUMBER_OF_TIMEOUT_RETRIES
): void {
    try {
        $batch = new BatchStatement(Cassandra::BATCH_UNLOGGED);
        $prepare = $session->prepare($insertQuery);

        $options = [
            'consistency'  => $consistency,
            'retry_policy' => $policy,
        ];

        foreach (range(1, $numberOfInserts) as $i) {
            $values = [$key, $i];
            if ($statementType === RP_BATCH_STATEMENT) {
                if ($i % 2 === 0) {
                    $batch->add($prepare, $values);
                } else {
                    $batch->add($insertQuery, $values);
                }
            } else {
                $statement = $prepare;
                if ($statementType === RP_SIMPLE_STATEMENT) {
                    $statement = $insertQuery;
                }
                $options['arguments'] = $values;
                $session->execute($statement, $options);
            }
        }

        if ($statementType === RP_BATCH_STATEMENT) {
            $session->execute($batch, $options);
        }
    } catch (TimeoutException $te) {
        if ($retries > 0) {
            rpInsert($session, $insertQuery, $statementType, $policy, $key, $numberOfInserts, $consistency, $retries - 1);
        } else {
            throw $te;
        }
    }
}

function rpAssert(
    Session $session,
    string $keyspace,
    string $table,
    RetryPolicy $policy,
    int $key,
    int $numberOfAsserts,
    int $consistency,
    int $retries = RP_NUMBER_OF_TIMEOUT_RETRIES
): void {
    try {
        $options = [
            'consistency'  => $consistency,
            'retry_policy' => $policy,
        ];
        $rows = $session->execute(
            "SELECT value_int FROM $keyspace.$table WHERE key = $key",
            $options
        );

        expect($rows)->toHaveCount($numberOfAsserts);
        foreach ($rows as $i => $row) {
            expect($row['value_int'])->toEqual($i + 1);
        }
    } catch (TimeoutException $te) {
        if ($retries > 0) {
            rpAssert($session, $keyspace, $table, $policy, $key, $numberOfAsserts, $consistency, $retries - 1);
        } else {
            throw $te;
        }
    }
}

it('supports the downgrading consistency retry policy', function () use ($keyspace, $table, &$insertQuery) {
    $session = scyllaDbConnection($keyspace);
    $policy = new RetryPolicy\DowngradingConsistency();

    foreach (range(1, 3) as $statementType) {
        if ($statementType === RP_BATCH_STATEMENT
            && version_compare(Cassandra::CPP_DRIVER_VERSION, '2.2.3') < 0) {
            continue;
        }

        rpInsert($session, $insertQuery, $statementType, $policy, 0, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_ALL);
        rpAssert($session, $keyspace, $table, $policy, 0, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_ALL);

        rpInsert($session, $insertQuery, $statementType, $policy, 1, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_QUORUM);
        rpAssert($session, $keyspace, $table, $policy, 1, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_QUORUM);

        rpInsert($session, $insertQuery, $statementType, $policy, 2, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_TWO);
        rpAssert($session, $keyspace, $table, $policy, 2, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_TWO);
    }
});

it('supports the fallthrough retry policy on writes', function () use ($keyspace, &$insertQuery) {
    $session = scyllaDbConnection($keyspace);
    $policy = new RetryPolicy\Fallthrough();

    try {
        foreach (range(1, 3) as $statementType) {
            if ($statementType === RP_BATCH_STATEMENT
                && version_compare(Cassandra::CPP_DRIVER_VERSION, '2.2.3') < 0) {
                continue;
            }
            rpInsert($session, $insertQuery, $statementType, $policy, 0, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_ALL);
        }
        $this->fail('Expected UnavailableException to be thrown');
    } catch (UnavailableException $e) {
        expect($e->getMessage())->toMatch('/Cannot achieve consistency level .*/');
    }
});

it('supports the fallthrough retry policy on reads', function () use ($keyspace, $table, &$insertQuery) {
    $session = scyllaDbConnection($keyspace);
    $policy = new RetryPolicy\Fallthrough();

    try {
        foreach (range(1, 3) as $statementType) {
            if ($statementType === RP_BATCH_STATEMENT
                && version_compare(Cassandra::CPP_DRIVER_VERSION, '2.2.3') < 0) {
                continue;
            }
            rpInsert($session, $insertQuery, $statementType, $policy, 0, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_ONE);
            rpAssert($session, $keyspace, 'retry_policy', $policy, 0, RP_NUMBER_OF_INSERTS, Cassandra::CONSISTENCY_ALL);
        }
        $this->fail('Expected UnavailableException to be thrown');
    } catch (UnavailableException $e) {
        expect($e->getMessage())->toMatch('/Cannot achieve consistency level .*/');
    }
});
