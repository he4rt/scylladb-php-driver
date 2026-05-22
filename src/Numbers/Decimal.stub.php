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
    final class Decimal implements Value, Numeric {
        public function __construct(int|float|string|Decimal $value) {}

        public function type(): Type {}
        public function value(): string {}
        public function scale(): int {}

        public function add(Numeric $num): static {}
        public function sub(Numeric $num): static {}
        public function mul(Numeric $num): static {}
        public function div(Numeric $num): static {}
        public function mod(Numeric $num): static {}
        public function abs(): static {}
        public function neg(): static {}
        public function sqrt(): static {}
        public function toInt(): int {}
        public function toDouble(): float {}

        public function __toString(): string {}
    }
}
