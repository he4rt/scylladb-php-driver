<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_duration
 */    final class Duration implements Value {
        public function __construct(int|float|string|Bigint|\DateInterval $months, int|float|string|Bigint $days = UNKNOWN, int|float|string|Bigint $nanos = UNKNOWN) {}

        public static function fromDateInterval(\DateInterval $interval): static {}

        public function type(): Type {}
        public function months(): string {}
        public function days(): string {}
        public function nanos(): string {}

        public function __toString(): string {}
    }
}
