<?php

/**
* @generate-class-entries
*/

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_timestamp
 */    final class Timestamp implements Value {
        public function __construct(int|\DateTimeInterface $seconds = UNKNOWN, int $microseconds = UNKNOWN) {}

        public function type(): Type {}
        public function time(): int {}
        public function microtime(bool $get_as_float = false): float|string {}
        public function toDateTime(): \DateTime {}
        public static function fromDateTime(\DateTimeInterface $datetime): static {}
        public static function now(): static {}
        /** Alias of now(). A timestamp holds a UTC epoch value, so it carries no zone. */
        public static function nowUtc(): static {}

        public function __toString(): string {}
    }
}