# Performance

Five things account for almost every performance problem with this driver. They are listed in the
order of how much they usually matter.

## 1. Reuse the session

A `connect()` opens sockets to every node, performs the TLS handshake, authenticates, and reads the
schema. That is expensive. Doing it per request dominates everything else.

```php
// Wrong: a new pool on every request.
function handle(Request $r): Response
{
    $session = Cassandra::cluster()->withContactPoints('10.0.0.1')->build()->connect('shop');
    // ...
}
```

```php
// Right: one session, created once.
final class SessionFactory
{
    private static ?Cassandra\Session $session = null;

    public static function get(): Cassandra\Session
    {
        return self::$session ??= Cassandra::cluster()
            ->withContactPoints(...HOSTS)
            ->withPersistentSessions(true)
            ->build()
            ->connect('shop');
    }
}
```

Under PHP-FPM, `withPersistentSessions(true)` keeps the session in the worker process across
requests. It is on by default. Do not turn it off in production.

Under a long-running runtime such as RoadRunner, FrankenPHP, or Swoole, hold the session in a
container or a static and build it at boot.

## 2. Prepare every statement on the request path

```php
// Wrong: a network round trip to prepare, on every call.
function find(Cassandra\Session $s, $id) {
    $stmt = $s->prepare('SELECT * FROM users WHERE id = ?');
    return $s->execute($stmt, ['arguments' => [$id]]);
}
```

Preparing costs a round trip and, worse, an unprepared statement cannot be routed to a replica.
Every simple statement takes an extra network hop through a coordinator.

Prepare in the constructor and keep the object:

```php
final class UserRepository
{
    private Cassandra\PreparedStatement $findById;

    public function __construct(private Cassandra\Session $session)
    {
        $this->findById = $session->prepare('SELECT id, email FROM users WHERE id = ?');
    }
}
```

Combined with token-aware routing, which is on by default, this removes one hop from every query.

## 3. Use concurrency instead of loops

A sequential loop pays the full round trip for every row.

```php
// 1000 sequential round trips.
foreach ($ids as $id) {
    $users[] = $session->execute($findById, ['arguments' => [$id]])->first();
}
```

```php
// About one round trip, in windows of 128.
$window = 128;
$futures = [];

foreach ($ids as $id) {
    $futures[] = $session->executeAsync($findById, ['arguments' => [$id]]);

    if (count($futures) >= $window) {
        foreach ($futures as $f) { $users[] = $f->get()->first(); }
        $futures = [];
    }
}
foreach ($futures as $f) { $users[] = $f->get()->first(); }
```

See [asynchronous queries](/guide/async). Do not reach for a
[batch](/guide/batches) here. A multi-partition batch is slower than concurrent single writes.

## 4. Select the columns you need

```php
// Transfers and decodes every column.
$session->execute('SELECT * FROM users WHERE id = ?', ['arguments' => [$id]]);

// Transfers and decodes two.
$session->execute('SELECT id, email FROM users WHERE id = ?', ['arguments' => [$id]]);
```

Decoding happens in C, but a wide row with collections still costs allocation per value. Naming the
columns also keeps the code working when someone adds a column.

## 5. Match the page size to the work

```php
$session->execute($listQuery,   ['page_size' => 50]);     // a user-facing list
$session->execute($exportQuery, ['page_size' => 5000]);   // a background export
```

A page too small adds round trips. A page too large risks the request timeout and raises peak memory.
The default is 5000. See [results and paging](/guide/results#choosing-a-page-size).

## Consistency costs latency

| Level | Replicas that must answer | Relative cost |
| --- | --- | --- |
| `CONSISTENCY_LOCAL_ONE` | One, in the local datacenter | Lowest |
| `CONSISTENCY_LOCAL_QUORUM` | A local quorum | The usual choice |
| `CONSISTENCY_QUORUM` | A quorum across datacenters | A cross-region round trip |
| `CONSISTENCY_ALL` | Every replica | Highest, and fragile |

Set the cluster default to `LOCAL_QUORUM` and lower it per query where staleness is acceptable:

```php
$rows = $session->execute($popularItems, [
    'consistency' => Cassandra::CONSISTENCY_LOCAL_ONE,
]);
```

## Pool and thread settings

Defaults suit PHP-FPM. Change them only against a measurement. See
[connection pool and timeouts](/guide/connection-tuning).

| Setting | Default | Raise it when |
| --- | --- | --- |
| `withConnectionsPerHost(core, max)` | 1, 2 | Latency rises while node CPU stays low |
| `withIOThreads(n)` | 1 | One process drives many concurrent futures |
| `withConnectionHeartbeatInterval(s)` | 30 | Never raise. Lower it behind an idle-flow-killing firewall. [Defect in 1.4.x](/guide/connection-tuning#heartbeats-and-reconnection) |

Remember the multiplier: connections per host times nodes times PHP worker processes is the total
socket count from one machine.

## Value objects and memory

Every non-native column allocates a PHP object per row per column. A page of 5000 rows with ten
`bigint` columns allocates 50000 objects.

Two ways to reduce that:

- Select fewer columns.
- Use a smaller page size when memory matters more than round trips.

Convert to plain PHP values as soon as you can, so the driver objects become collectable:

```php
$emails = [];
foreach ($rows as $row) {
    $emails[(string) $row['id']] = $row['email'];
}
unset($rows);
```

## Measuring

`Session::metrics()` gives the driver's own view. Read it before you change a setting, and again
after.

```php
$m = $session->metrics();

$m['requests']['median'];       // microseconds
$m['requests']['p99'];
$m['requests']['mean_rate'];    // requests per second
$m['stats']['total_connections'];
$m['errors']['request_timeouts'];
```

A rising p99 with a flat median points at queueing or at one slow node. See
[metrics and logging](/guide/observability).

## A checklist

1. One session per process, or persistent sessions under PHP-FPM.
2. Every request-path statement prepared once and kept.
3. Token-aware routing on. It is the default.
4. Batches only for atomicity. Concurrency for throughput.
5. Explicit column lists.
6. `LOCAL_QUORUM` by default, lower where staleness is fine.
7. A page size chosen per query.
8. Metrics recorded before and after every tuning change.
