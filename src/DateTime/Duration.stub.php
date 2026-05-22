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
    final class Duration implements Value {
        public function __construct(int|float|string|Bigint $months, int|float|string|Bigint $days, int|float|string|Bigint $nanos) {}

        public function type(): Type {}
        public function months(): string {}
        public function days(): string {}
        public function nanos(): string {}

        public function __toString(): string {}
    }
}
