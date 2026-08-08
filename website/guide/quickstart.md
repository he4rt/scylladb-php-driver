# Quick start

This page builds a small application end to end: connect, create a schema, write, read, and handle
errors. It assumes the extension is installed and a node is reachable. See
[installation](/guide/installation) if it is not.

## 1. Connect

```php
<?php

declare(strict_types=1);

$cluster = Cassandra::cluster()
    ->withContactPoints('127.0.0.1')
    ->withPort(9042)
    ->build();

$session = $cluster->connect();
```

`Cassandra::cluster()` returns a [`Cassandra\Cluster\Builder`](/reference/cluster-builder). Every
`with*()` method returns the same builder, so calls chain. `build()` produces an immutable
`Cassandra\Cluster`. `connect()` opens the connection pool and returns a `Cassandra\Session`.

::: tip One session per process
Building a cluster is cheap. Connecting is not: it opens sockets to every node and reads the schema.
Create the session once and share it. See [performance](/guide/performance).
:::

## 2. Create a keyspace and a table

```php
$session->execute(
    "CREATE KEYSPACE IF NOT EXISTS shop
     WITH replication = {'class': 'NetworkTopologyStrategy', 'replication_factor': 1}"
);

$session->execute(
    'CREATE TABLE IF NOT EXISTS shop.users (
        id uuid PRIMARY KEY,
        email text,
        created_at timestamp,
        login_count bigint
    )'
);
```

`execute()` accepts a plain CQL string. The driver wraps it in a
[`Cassandra\SimpleStatement`](/guide/queries#simple-statements) for you.

## 3. Select the keyspace

Connect with a keyspace to avoid writing it in every query:

```php
$session = $cluster->connect('shop');
```

An existing session can switch with `USE`, but a per-keyspace session is clearer.

## 4. Write with a prepared statement

```php
$insert = $session->prepare(
    'INSERT INTO users (id, email, created_at, login_count) VALUES (?, ?, ?, ?)'
);

$id = new Cassandra\Uuid();   // random version 4 UUID

$session->execute($insert, [
    'arguments' => [
        $id,
        'ada@example.com',
        new Cassandra\Timestamp(time()),
        new Cassandra\Bigint(0),
    ],
]);
```

Two rules to remember:

- Bind values with the `arguments` key. Never build CQL by string concatenation.
- A `bigint` column needs a `Cassandra\Bigint`, not a PHP `int`. A plain `int` marshals as a 4-byte
  value and the server rejects it. The full table is in [data types](/guide/data-types).

## 5. Read

```php
$rows = $session->execute('SELECT id, email, login_count FROM users');

echo count($rows), " rows\n";

foreach ($rows as $row) {
    printf(
        "%s  %-24s logins=%s\n",
        $row['id'],          // Cassandra\Uuid
        $row['email'],       // string
        $row['login_count'], // Cassandra\Bigint
    );
}
```

Each row is an associative array keyed by column name. Values arrive as PHP scalars or as driver
value objects. See [results and paging](/guide/results).

## 6. Bind by name

Named markers work as well, and they do not depend on argument order:

```php
$byId = $session->prepare('SELECT email, login_count FROM users WHERE id = :id');

$rows = $session->execute($byId, [
    'arguments' => ['id' => $id],
]);

$first = $rows->first();   // null when the result is empty
```

## 7. Handle errors

Every driver error implements `Cassandra\Exception`. Catch the specific class you can act on, and
the interface for everything else.

```php
use Cassandra\Exception\InvalidQueryException;
use Cassandra\Exception\ReadTimeoutException;

try {
    $rows = $session->execute($select, ['timeout' => 2.5]);
} catch (ReadTimeoutException $e) {
    // Not enough replicas answered in time. Safe to retry a read.
    error_log('read timeout: ' . $e->getMessage());
} catch (InvalidQueryException $e) {
    // The table or column does not exist. Retrying will not help.
    throw $e;
} catch (Cassandra\Exception $e) {
    error_log('driver error: ' . $e->getMessage());
}
```

The full hierarchy is in [error handling](/guide/error-handling).

## 8. Close

```php
$session->close();
```

The session closes on script shutdown as well. Call `close()` when you want the sockets released at
a known point, for example in a long running worker between jobs.

## The complete example

```php
<?php

declare(strict_types=1);

$session = Cassandra::cluster()
    ->withContactPoints('127.0.0.1')
    ->withPort(9042)
    ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
    ->withTokenAwareRouting(true)
    ->build()
    ->connect('shop');

$insert = $session->prepare(
    'INSERT INTO users (id, email, created_at, login_count) VALUES (?, ?, ?, ?)'
);

$id = new Cassandra\Uuid();

$session->execute($insert, [
    'arguments' => [$id, 'ada@example.com', new Cassandra\Timestamp(time()), new Cassandra\Bigint(0)],
]);

$select = $session->prepare('SELECT email, login_count FROM users WHERE id = ?');
$row = $session->execute($select, ['arguments' => [$id]])->first();

printf("%s has %s logins\n", $row['email'], $row['login_count']);

$session->close();
```

## Next steps

- [Clusters and sessions](/guide/connecting) — the full connection model and every builder option.
- [Queries and statements](/guide/queries) — simple, prepared, and batch statements.
- [Data types](/guide/data-types) — the CQL to PHP mapping.
- [Asynchronous queries](/guide/async) — run many queries at once.
