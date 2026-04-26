<?php

/**
 * @generate-class-entries
 */
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Bigint implements Value, Numeric {
        public function __construct(string|int|float $value) {}

        public function type(): Type {}
        public function value(): string {}

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

        public static function min(): Bigint {}
        public static function max(): Bigint {}

        public function __toString(): string {}
    }
}
