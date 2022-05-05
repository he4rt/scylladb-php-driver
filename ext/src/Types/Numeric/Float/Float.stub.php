<?php

/** @generate-class-entries */

namespace Cassandra {

    /**
     * @strict-properties
     */
    final class Float implements Value, Numeric {

        /**
         * Creates a new float.
         *
         */
        public function __construct(float|int|string|Float $value)
        {
        }

        public function __toString(): string
        {
        }

        /**
         * @ref-count 1
         */

        public function value(): string
        {
        }

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


        /**
         * @ref-count 1
         */
        public function type(): Type
        {
        }

        /**
         * @ref-count 1

         */
        public function add(Numeric $num): Numeric
        {
        }

        /**
         * @ref-count 1

         */
        public function sub(Numeric $num): Numeric
        {
        }

        /**
         * @ref-count 1

         */
        public function mul(Numeric $num): Numeric
        {
        }

        /**
         * @ref-count 1

         */
        public function div(Numeric $num): Numeric
        {
        }

        /**
         * @ref-count 1

         */
        public function mod(Numeric $num): Numeric
        {
        }

        /**
         * @ref-count 1
         */
        public function abs(): Numeric
        {
        }

        /**
         * @ref-count 1
         */
        public function neg(): Numeric
        {
        }

        /**
         * @ref-count 1
         */
        public function sqrt(): Numeric
        {
        }

        public function toInt(): int
        {
        }

        public function toDouble(): float
        {
        }
    }
}
