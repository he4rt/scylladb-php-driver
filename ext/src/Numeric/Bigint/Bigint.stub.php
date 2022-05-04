<?php

/** @generate-class-entries */

namespace Cassandra {
    final class Bigint implements Value, Numeric {
        public function __construct(int|float|string|Bigint $value) { }

        public function __toString(): string { }

        public function value(): string { }

        public static function min(): Bigint { }

        public static function max(): Bigint { }
    }
}



