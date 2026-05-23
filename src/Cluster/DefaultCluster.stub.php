<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_cluster
 */    final class DefaultCluster implements Cluster {
        public function connect(?string $keyspace = null, ?int $timeout = null): Session {}
        public function connectAsync(?string $keyspace = null): Future {}
    }
}