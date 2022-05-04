<?php

/** @generate-class-entries */

namespace Cassandra {
    final class Decimal implements Value, Numeric {
        public function __construct(int|float|string|Decimal $value) { }

        public function __toString(): string { }

        public function value(): string { }

        public function scale(): int { }
    }
}

