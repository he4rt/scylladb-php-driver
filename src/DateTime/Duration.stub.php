<?php

/**
 * @generate-class-entries
 */
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Duration implements Value {
        public function __construct(mixed $months, mixed $days, mixed $nanos) {}

        public function type(): Type {}
        public function months(): string {}
        public function days(): string {}
        public function nanos(): string {}

        public function __toString(): string {}
    }
}
