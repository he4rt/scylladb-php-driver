<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    interface Cluster {
        public function connect(?string $keyspace = null, ?int $timeout = null): Session;
        public function connectAsync(?string $keyspace = null): Future;
    }
}