<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_statement
 */    final class BatchStatement implements Statement {
        public function __construct(int $type = \Cassandra::BATCH_LOGGED) {}
        public function add(string|Statement $statement, ?array $arguments = null): static {}
    }
}
