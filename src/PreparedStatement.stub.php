<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_statement
 */    final class PreparedStatement implements Statement {
        private function __construct() {}
        public function setIdempotent(bool $idempotent = true): static {}
        public function isIdempotent(): ?bool {}
    }
}
