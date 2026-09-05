<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
     * @scylladb-struct php_scylladb_date
     * @deprecated
     */
    #[\Deprecated(message: 'Use a PHP DateTimeImmutable.', since: '1.6.0')]
    final class Date implements Value {
        public function __construct(int|string $value = UNKNOWN) {}

        public static function fromDateTime(\DateTimeInterface $datetime): static {}
        public function toDateTime(?Time $time = null): \DateTime {}
        public function seconds(): int {}
        public function type(): Type {}

        public function __toString(): string {}
    }
}