<?php

/**
 * @generate-class-entries
 */
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Float implements Value, Numeric {
        public function __construct(string|int|float $value) {}

        public function type(): Type {}
        public function value(): float {}
        public function isInfinite(): bool {}
        public function isFinite(): bool {}
        public function isNaN(): bool {}

        public function add(mixed $num): Numeric {}
        public function sub(mixed $num): Numeric {}
        public function mul(mixed $num): Numeric {}
        public function div(mixed $num): Numeric {}
        public function mod(mixed $num): Numeric {}
        public function abs(): Numeric {}
        public function neg(): Numeric {}
        public function sqrt(): Numeric {}
        public function toInt(): int {}
        public function toDouble(): float {}

        public static function min(): Float {}
        public static function max(): Float {}

        public function __toString(): string {}
    }
}
