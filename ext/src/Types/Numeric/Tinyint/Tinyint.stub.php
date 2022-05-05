<?php

/** @generate-class-entries */

namespace Cassandra {

    /**
     * @strict-properties
     */
    final class Tinyint implements Value, Numeric
    {
        /**
         * Creates a new 8-bit signed integer.
         *
         * @param int|double|string $value The value as an integer, double or string
         */
        public function __construct(int|float|string|Tinyint $value)
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

        /**
         * @ref-count 1
         */
        public static function min(): Tinyint
        {
        }

        /**
         * @ref-count 1
         */
        public static function max(): Tinyint
        {
        }

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
