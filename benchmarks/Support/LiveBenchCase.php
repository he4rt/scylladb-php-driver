<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Support;

use Cassandra;
use Cassandra\Cluster;
use Cassandra\SimpleStatement;
use Cassandra\Session;

/**
 * Base class for benchmarks that talk to a real ScyllaDB/Cassandra node.
 *
 * PhpBench runs every *iteration* in its own subprocess, so a session opened
 * in a `@BeforeMethods` hook lives for all the revolutions of that iteration
 * and is torn down when the process exits. That is exactly what we want: the
 * connection cost is paid once per iteration and amortised across revs, while
 * the timed subject only measures the operation under test.
 *
 * Subclasses that need a schema call {@see ensureSchema()} from their own
 * before-hook. The benchmark keyspace is created IF NOT EXISTS and is *never*
 * dropped automatically — dropping it would race across the per-iteration
 * subprocesses. Use `composer bench` docs / `make bench-clean` to remove it.
 */
abstract class LiveBenchCase
{
    protected ?Session $session = null;

    protected static function builder(bool $persistent = false): Cluster\Builder
    {
        return Cassandra::cluster()
            ->withContactPoints(...Env::hosts())
            ->withPort(Env::port())
            ->withCredentials(Env::username(), Env::password())
            ->withPersistentSessions($persistent)
            ->withTokenAwareRouting(true);
    }

    protected function connect(?string $keyspace = null, bool $persistent = false): Session
    {
        $cluster = static::builder($persistent)->build();

        return $keyspace !== null && $keyspace !== ''
            ? $cluster->connect($keyspace)
            : $cluster->connect();
    }

    /**
     * Create the benchmark keyspace (IF NOT EXISTS) plus any table DDL the
     * subclass needs, then leave $this->session connected to that keyspace.
     *
     * @param list<string> $tableDdl fully-formed `CREATE TABLE IF NOT EXISTS`
     *                               statements, keyspace-qualified by $keyspace.
     */
    protected function ensureSchema(array $tableDdl = []): void
    {
        $keyspace = Env::keyspace();

        $admin = $this->connect(persistent: false);
        $admin->execute(new SimpleStatement(sprintf(
            "CREATE KEYSPACE IF NOT EXISTS %s WITH replication = "
                . "{'class': 'SimpleStrategy', 'replication_factor': 1}",
            $keyspace,
        )));

        foreach ($tableDdl as $ddl) {
            $admin->execute(new SimpleStatement($ddl));
        }

        $admin->close();

        $this->session = $this->connect($keyspace, persistent: false);
    }

    public function closeSession(): void
    {
        $this->session?->close();
        $this->session = null;
    }
}
