# Batches

A batch sends several statements in one request. Its purpose is atomicity inside one partition, not
throughput.

```php
$batch = new Cassandra\BatchStatement(Cassandra::BATCH_LOGGED);

$batch->add($insertUser, [$id, 'ada@example.com']);
$batch->add($insertIndex, ['ada@example.com', $id]);

$session->execute($batch);
```

`add()` returns the batch, so calls chain:

```php
$batch
    ->add($insertUser, [$id, $email])
    ->add($insertIndex, [$email, $id]);
```

::: danger A batch is not a performance tool
In a relational database a batch reduces round trips. In ScyllaDB it does the opposite when the
statements touch different partitions: one coordinator receives everything and forwards each write,
so it becomes a bottleneck and latency rises.

Use [`executeAsync()`](/guide/async) for throughput. Use a batch for atomicity.
:::

## Batch types

### `BATCH_LOGGED`

```php
new Cassandra\BatchStatement(Cassandra::BATCH_LOGGED);   // the default
```

The coordinator writes a batch log before it applies anything. If it dies partway, another node
replays the log, so every statement eventually applies.

The guarantee is atomicity, not isolation. Another reader can observe some statements applied and
others not, while the batch is in flight.

The batch log costs an extra write and an extra delete per batch. Use logged batches only where a
partial apply would corrupt the data model, such as keeping a table and its inverted index in step.

### `BATCH_UNLOGGED`

```php
new Cassandra\BatchStatement(Cassandra::BATCH_UNLOGGED);
```

No batch log, so no atomicity across partitions. When every statement targets the same partition,
the write is atomic anyway and this is the right type.

```php
// Same partition key: one atomic write, no batch log.
$batch = new Cassandra\BatchStatement(Cassandra::BATCH_UNLOGGED);
$batch->add($insertEvent, [$userId, $event1Id, $payload1]);
$batch->add($insertEvent, [$userId, $event2Id, $payload2]);
```

### `BATCH_COUNTER`

```php
new Cassandra\BatchStatement(Cassandra::BATCH_COUNTER);
```

Required for counter updates. A counter batch cannot contain a normal write, and a normal batch
cannot contain a counter update.

```php
$batch = new Cassandra\BatchStatement(Cassandra::BATCH_COUNTER);
$batch->add($incrementViews, [$postId]);
$batch->add($incrementDaily, [$day, $postId]);
```

Counter updates are not idempotent. A retry double-counts. See
[retry policies](/guide/retry-policies#retries-and-idempotency).

## What you can add

`add()` accepts a CQL string, a `Cassandra\SimpleStatement`, or a `Cassandra\PreparedStatement`. The
second argument is the array of bound values.

```php
$batch->add('UPDATE users SET email = ? WHERE id = ?', [$email, $id]);

$batch->add(new Cassandra\SimpleStatement('DELETE FROM sessions WHERE id = ?'), [$sid]);

$prepared = $session->prepare('INSERT INTO audit (id, action) VALUES (?, ?)');
$batch->add($prepared, [new Cassandra\Timeuuid(), 'login']);
```

Bind by name with a string-keyed array:

```php
$batch->add($prepared, ['id' => new Cassandra\Timeuuid(), 'action' => 'login']);
```

Prepared statements are as valuable in a batch as anywhere else.

## Execution options

A batch takes the same options as any statement:

```php
$session->execute($batch, [
    'consistency' => Cassandra::CONSISTENCY_LOCAL_QUORUM,
    'timeout'     => 5.0,
]);
```

The `arguments` key does not apply. Values belong to each `add()` call.

## Size limits

The server rejects an oversized batch and warns about a large one. Both thresholds are server
settings (`batch_size_fail_threshold_in_kb` and `batch_size_warn_threshold_in_kb`).

Keep a batch small, in the tens of statements. Split a large workload:

```php
foreach (array_chunk($writes, 25) as $chunk) {
    $batch = new Cassandra\BatchStatement(Cassandra::BATCH_UNLOGGED);

    foreach ($chunk as [$id, $email]) {
        $batch->add($insert, [$id, $email]);
    }

    $session->execute($batch);
}
```

## When to use what

| Goal | Tool |
| --- | --- |
| Several rows in one partition, applied together | `BATCH_UNLOGGED` |
| A table and its index kept in step across partitions | `BATCH_LOGGED` |
| Counter updates | `BATCH_COUNTER` |
| Many independent writes, as fast as possible | [`executeAsync()`](/guide/async) |
| A conditional write | [A lightweight transaction](/guide/queries#lightweight-transactions) |

## A bulk load, done right

```php
$insert  = $session->prepare('INSERT INTO users (id, email) VALUES (?, ?)');
$inFlight = [];

foreach ($records as $record) {
    $inFlight[] = $session->executeAsync($insert, [
        'arguments' => [$record['id'], $record['email']],
    ]);

    if (count($inFlight) >= 100) {
        foreach ($inFlight as $future) {
            $future->get();
        }
        $inFlight = [];
    }
}

foreach ($inFlight as $future) {
    $future->get();
}
```

Each write goes straight to a replica, because the statement is prepared and routing is token aware.
No single coordinator carries the whole load.
