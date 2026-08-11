<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_statement
 */    final class SimpleStatement implements Statement {
        public function __construct(string $cql) {}
        public function setIdempotent(bool $idempotent = true): static {}
        public function isIdempotent(): ?bool {}
    }
}
