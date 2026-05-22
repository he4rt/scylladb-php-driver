<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
     */
    final class Uuid implements Value, UuidInterface {
        public function __construct(string $uuid = UNKNOWN) {}

        public function uuid(): string {}
        public function version(): int {}
        public function type(): Type {}

        public function __toString(): string {}
    }
}
