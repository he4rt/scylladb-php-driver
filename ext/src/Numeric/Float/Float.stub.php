<?php

/** @generate-class-entries */

namespace Cassandra {
    final class Float implements Value, Numeric {

        /**
         * Creates a new float.
         *
         * @param double|int|string|\Cassandra\Float $value A float value as a string, number or Float
         */
        public function __construct(double|int|string|Float $value) { }

        /**
         * Returns string representation of the float value.
         *
         * @return string float value
         */
        public function __toString(): string { }

        public function value(): float { }

        public function isInfinite(): bool { }

        public function isFinite(): bool { }

        public function isNaN(): bool { }

        /**
         * Minimum possible Float value
         *
         * @return \Cassandra\Float minimum value
         */
        public static function min(): Float { }

        /**
         * Maximum possible Float value
         *
         * @return \Cassandra\Float maximum value
         */
        public static function max(): Float { }
    }

}