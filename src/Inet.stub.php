<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_inet
 */    final class Inet implements Value {
        public function __construct(string $address) {}

        public function type(): Type {}
        public function address(): string {}

        public function __toString(): string {}
    }
}
