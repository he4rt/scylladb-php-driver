<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Results;

use Cassandra\Exception\ProtocolException;
use Cassandra\Type;
use Cassandra\Uuid;
use InvalidArgumentException;

$keyspace = 'paging_int_' . substr(bin2hex(random_bytes(4)), 0, 8);
$table = 'paging_entries';

beforeAll(function () use ($keyspace, $table) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    CREATE TABLE $keyspace.$table (key int PRIMARY KEY, value int);
    CQL);

    $session = scyllaDbConnection($keyspace);
    for ($i = 0; $i < 10; $i++) {
        $session->execute(
            "INSERT INTO $keyspace.$table (key, value) VALUES (?, ?)",
            ['arguments' => [$i, $i]]
        );
    }
});

afterAll(fn () => dropKeyspace($keyspace));

function rowsToColumn(iterable $rows, string $column): array
{
    $values = [];
    foreach ($rows as $row) {
        $values[] = $row[$column];
    }
    return $values;
}

it('Returns each row exactly once when iterating with paging tokens', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $results = [];
    $result = null;

    for ($i = 0; $i < 10; $i++) {
        $options = ['page_size' => 1];
        if ($result !== null) {
            $options['paging_state_token'] = $result->pagingStateToken();
        }
        $result = $session->execute("SELECT * FROM $keyspace.$table", $options);
        expect($result->count())->toBe(1);
        $results[] = $result->first()['value'];
    }

    sort($results);
    expect($results)->toEqual(range(0, 9));
});

it('Throws ProtocolException for an invalid paging state token', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    expect(fn () => $session->execute(
        "SELECT * FROM $keyspace.$table",
        ['paging_state_token' => 'invalid']
    ))->toThrow(ProtocolException::class);
});

it('Throws InvalidArgumentException for a null paging state token', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    expect(fn () => $session->execute(
        "SELECT * FROM $keyspace.$table",
        ['paging_state_token' => null]
    ))->toThrow(InvalidArgumentException::class);
});

it('Caches the result of nextPage() and matches nextPageAsync()', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $pageSize = 2;

    $rows = $session->execute("SELECT * FROM $keyspace.$table", ['page_size' => $pageSize]);
    expect($rows->count())->toBe($pageSize);
    $values = rowsToColumn($rows, 'value');

    $next = $rows->nextPage();
    $nextValues = rowsToColumn($next, 'value');
    expect($next->count())->toBe($pageSize)
        ->and($nextValues)->not->toEqual($values);

    // Re-calling nextPage should return the same cached page
    $nextAgain = $rows->nextPage();
    expect(rowsToColumn($nextAgain, 'value'))->toEqual($nextValues);

    // Async should match sync
    $nextAsync = $rows->nextPageAsync()->get();
    expect(rowsToColumn($nextAsync, 'value'))->toEqual($nextValues);

    // Advancing further yields a new page
    $last = $next->nextPage();
    $lastValues = rowsToColumn($last, 'value');
    expect($lastValues)->not->toEqual($nextValues);
});

it('Caches nextPageAsync() results and matches synchronous nextPage()', function () use ($keyspace, $table) {
    $session = scyllaDbConnection($keyspace);
    $pageSize = 2;

    $rows = $session->execute("SELECT * FROM $keyspace.$table", ['page_size' => $pageSize]);
    $values = rowsToColumn($rows, 'value');

    $nextAsync = $rows->nextPageAsync()->get();
    $nextAsyncValues = rowsToColumn($nextAsync, 'value');
    expect($nextAsyncValues)->not->toEqual($values);

    $nextAsyncAgain = $rows->nextPageAsync()->get();
    expect(rowsToColumn($nextAsyncAgain, 'value'))->toEqual($nextAsyncValues);

    $nextSync = $rows->nextPage();
    expect(rowsToColumn($nextSync, 'value'))->toEqual($nextAsyncValues);
});

it('Pages through a non-trivial dataset without losing rows', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);
    $bigTable = 'paging_big';
    $session->execute(
        "CREATE TABLE $keyspace.$bigTable (id uuid PRIMARY KEY, value int)"
    );

    $totalInserts = 200;
    $stmt = $session->prepare("INSERT INTO $keyspace.$bigTable (id, value) VALUES (?, ?)");
    for ($i = 0; $i < $totalInserts; $i++) {
        $session->execute($stmt, ['arguments' => [new Uuid(), $i]]);
    }

    $rows = $session->execute("SELECT * FROM $keyspace.$bigTable", ['page_size' => 10]);
    $count = $rows->count();
    while ($rows = $rows->nextPage()) {
        $count += $rows->count();
    }

    expect($count)->toBe($totalInserts);
});
