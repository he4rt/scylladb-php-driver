<?php

/**
* @generate-class-entries
*/

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_time
 */    final class Time implements Value {
        public function __construct(int|string $nanoseconds = UNKNOWN) {}

        public function type(): Type {}
        public function seconds(): int {}
        public static function fromDateTime(\DateTimeInterface $datetime): static {}

        public function __toString(): string {}
    }
}