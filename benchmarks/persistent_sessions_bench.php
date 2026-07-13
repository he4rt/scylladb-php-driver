<?php

declare(strict_types=1);

/**
 * ScyllaDB PHP Driver 1.4.0 — persistent-session + statement benchmark
 *
 * What this measures, and why it exists for 1.4.0:
 *
 *   1. Persistent-session cache cost.
 *      In 1.4.0 the persistent-session cache key stopped being a ~200-char
 *      string built with spprintf/emalloc on every build() and became an inline
 *      uint64_t FNV-1a fingerprint looked up with zend_hash_index_find. The win
 *      is "no allocation on the cache hot path", so it shows up more in the
 *      peak-memory column than in raw wall-clock. This bench loops
 *      build()+connect() many times with persistent sessions ON vs OFF and
 *      reports both ops/sec and peak-memory growth.
 *
 *   2. Prepared vs SimpleStatement throughput.
 *      Prepared statements skip server-side re-parsing on every execute. This is
 *      unchanged behaviour, included as a baseline you can sanity-check.
 *
 *   3. Batch insert throughput (optional, only if the table is writable).
 *
 * This is a real, standalone script. It needs a running ScyllaDB/Cassandra and
 * the 1.4.x extension loaded. It creates its own throwaway keyspace and table
 * and drops the keyspace at the end.
 *
 * Run:
 *
 *   # bring up a local ScyllaDB first, e.g.
 *   docker run --rm -p 9042:9042 --name scylla-bench scylladb/scylla \
 *       --smp 1 --skip-wait-for-gossip-to-settle 0
 *
 *   SCYLLA_HOST=127.0.0.1 SCYLLA_PORT=9042 \
 *   php -d extension=cassandra benchmarks/persistent_sessions_bench.php
 *
 * Env vars (all optional):
 *   SCYLLA_HOST      default 127.0.0.1
 *   SCYLLA_PORT      default 9042
 *   SCYLLA_USER      default (none — no auth)
 *   SCYLLA_PASS      default (none)
 *   BENCH_SESSIONS   iterations for the persistent-session bench (default 200)
 *   BENCH_QUERIES    iterations for the statement bench (default 2000)
 */

if (!extension_loaded('cassandra')) {
    fwrite(STDERR, "The 'cassandra' extension (scylladb-php-driver >= 1.4) is not loaded.\n");
    fwrite(STDERR, "Build it, then run with: php -d extension=cassandra " . basename(__FILE__) . "\n");
    exit(1);
}

$HOST     = getenv('SCYLLA_HOST') ?: '127.0.0.1';
$PORT     = (int) (getenv('SCYLLA_PORT') ?: 9042);
$USER     = getenv('SCYLLA_USER') ?: null;
$PASS     = getenv('SCYLLA_PASS') ?: null;
$N_SESS   = (int) (getenv('BENCH_SESSIONS') ?: 200);
$N_QUERY  = (int) (getenv('BENCH_QUERIES') ?: 2000);
$KEYSPACE = 'bench_1_4_0';

echo "scylladb-php-driver benchmark\n";
echo "  driver version : " . (defined('Cassandra::VERSION') ? Cassandra::VERSION : 'unknown') . "\n";
echo "  php            : " . PHP_VERSION . " (" . (ZEND_THREAD_SAFE ? 'ZTS' : 'NTS') . ")\n";
echo "  target         : {$HOST}:{$PORT}\n";
echo "  keyspace       : {$KEYSPACE}\n\n";

/**
 * Build a cluster with a fixed config, toggling only persistent sessions.
 * The point of the bench is that the cache-key work inside build() differs;
 * everything else about the config is held constant.
 */
function makeCluster(bool $persistent): \Cassandra\Cluster
{
    global $HOST, $PORT, $USER, $PASS;

    $builder = Cassandra::cluster()
        ->withContactPoints($HOST)
        ->withPort($PORT)
        ->withPersistentSessions($persistent)
        ->withTokenAwareRouting(true)
        ->withConnectTimeout(5.0)
        ->withRequestTimeout(12.0)
        ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_ONE);

    if ($USER !== null) {
        $builder = $builder->withCredentials($USER, $PASS ?? '');
    }

    return $builder->build();
}

function fmt(float $seconds): string
{
    if ($seconds < 1e-3) {
        return sprintf('%6.1f us', $seconds * 1e6);
    }
    if ($seconds < 1.0) {
        return sprintf('%6.2f ms', $seconds * 1e3);
    }
    return sprintf('%6.3f s ', $seconds);
}

function mib(int $bytes): string
{
    return sprintf('%6.2f MiB', $bytes / (1024 * 1024));
}

// --- connectivity check + schema setup ---------------------------------------

try {
    $boot = makeCluster(false)->connect();
    $boot->execute(new \Cassandra\SimpleStatement(
        "CREATE KEYSPACE IF NOT EXISTS {$KEYSPACE} " .
        "WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}"
    ));
    $boot->execute(new \Cassandra\SimpleStatement(
        "CREATE TABLE IF NOT EXISTS {$KEYSPACE}.events (" .
        "  id uuid PRIMARY KEY, kind text, payload text, created bigint)"
    ));
    $boot->close();
} catch (\Throwable $e) {
    fwrite(STDERR, "Could not reach ScyllaDB at {$HOST}:{$PORT}: {$e->getMessage()}\n");
    fwrite(STDERR, "Start a node (see the header comment) and try again.\n");
    exit(1);
}

// --- bench 1: persistent-session cache ---------------------------------------
//
// Loop build()+connect()+close() N times. With persistent sessions ON, the
// second build() onward hits the persistent_list cache (the uint64 fingerprint
// path). With OFF, every build() constructs a fresh CassCluster. We report
// wall-clock ops/sec and, more tellingly for 1.4.0, peak-memory growth.

function benchSessions(bool $persistent, int $n): array
{
    // Prime once so the very first connect (TCP + control connection) isn't
    // counted against the loop we care about.
    makeCluster($persistent)->connect()->close();

    $memBefore = memory_get_peak_usage(true);
    $t0 = hrtime(true);

    for ($i = 0; $i < $n; $i++) {
        $session = makeCluster($persistent)->connect();
        // one trivial query so the session actually does work
        $session->execute(new \Cassandra\SimpleStatement('SELECT now() FROM system.local'));
        $session->close();
    }

    $elapsed = (hrtime(true) - $t0) / 1e9;
    $memAfter = memory_get_peak_usage(true);

    return [
        'elapsed'    => $elapsed,
        'per_op'     => $elapsed / $n,
        'ops_sec'    => $n / $elapsed,
        'mem_growth' => $memAfter - $memBefore,
    ];
}

echo "── bench 1: build()+connect() x {$N_SESS} ───────────────────────────\n";

$off = benchSessions(false, $N_SESS);
printf("  persistent OFF : %s total  %s/op  %8.0f ops/s  peak+%s\n",
    fmt($off['elapsed']), fmt($off['per_op']), $off['ops_sec'], mib($off['mem_growth']));

$on = benchSessions(true, $N_SESS);
printf("  persistent ON  : %s total  %s/op  %8.0f ops/s  peak+%s\n",
    fmt($on['elapsed']), fmt($on['per_op']), $on['ops_sec'], mib($on['mem_growth']));

$speedup = $off['per_op'] / max($on['per_op'], 1e-12);
printf("  → persistent ON is %.2fx faster per build+connect, peak-mem growth %s vs %s\n\n",
    $speedup, mib($on['mem_growth']), mib($off['mem_growth']));

// --- bench 2: prepared vs simple statement -----------------------------------

echo "── bench 2: SELECT x {$N_QUERY} (prepared vs simple) ────────────────\n";

$session = makeCluster(true)->connect($KEYSPACE);

// seed one row to select
$seedId = new \Cassandra\Uuid();
$session->execute(new \Cassandra\SimpleStatement(
    "INSERT INTO events (id, kind, payload, created) VALUES (" .
    $seedId->uuid() . ", 'seed', 'hello', 1)"
));

$simpleCql = "SELECT id, kind, payload FROM events LIMIT 1";

$t0 = hrtime(true);
for ($i = 0; $i < $N_QUERY; $i++) {
    $rows = $session->execute(new \Cassandra\SimpleStatement($simpleCql));
    foreach ($rows as $_) { /* drain */ }
}
$simpleElapsed = (hrtime(true) - $t0) / 1e9;

$prepared = $session->prepare($simpleCql);
$t0 = hrtime(true);
for ($i = 0; $i < $N_QUERY; $i++) {
    $rows = $session->execute($prepared);
    foreach ($rows as $_) { /* drain */ }
}
$preparedElapsed = (hrtime(true) - $t0) / 1e9;

printf("  simple   : %s total  %8.0f q/s\n", fmt($simpleElapsed), $N_QUERY / $simpleElapsed);
printf("  prepared : %s total  %8.0f q/s\n", fmt($preparedElapsed), $N_QUERY / $preparedElapsed);
printf("  → prepared is %.2fx the throughput of re-parsed simple statements\n\n",
    $simpleElapsed / max($preparedElapsed, 1e-12));

// --- bench 3: batch insert throughput ----------------------------------------

echo "── bench 3: batched INSERT (100 rows/batch x 20) ────────────────────\n";

$insert = $session->prepare("INSERT INTO events (id, kind, payload, created) VALUES (?, ?, ?, ?)");
$batches = 20;
$perBatch = 100;

$t0 = hrtime(true);
for ($b = 0; $b < $batches; $b++) {
    $batch = new \Cassandra\BatchStatement(Cassandra::BATCH_UNLOGGED);
    for ($r = 0; $r < $perBatch; $r++) {
        $batch->add($insert, [new \Cassandra\Uuid(), 'evt', str_repeat('x', 64), (int) (hrtime(true) / 1000)]);
    }
    $session->execute($batch);
}
$batchElapsed = (hrtime(true) - $t0) / 1e9;
$totalRows = $batches * $perBatch;

printf("  %d rows in %s  →  %8.0f rows/s\n\n", $totalRows, fmt($batchElapsed), $totalRows / $batchElapsed);

// --- teardown ----------------------------------------------------------------

$session->execute(new \Cassandra\SimpleStatement("DROP KEYSPACE IF EXISTS {$KEYSPACE}"));
$session->close();

echo "done. peak memory this process: " . mib(memory_get_peak_usage(true)) . "\n";
