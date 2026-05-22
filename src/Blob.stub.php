<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_blob
 */    final class Blob implements Value {
        public function __construct(string $bytes) {}

        public function type(): Type {}
        public function bytes(): string {}
        public function toBinaryString(): string {}

        public function __toString(): string {}
    }
}
