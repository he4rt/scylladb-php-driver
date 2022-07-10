<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Smallint implements Value, Numeric
    {
        /**
         * Creates a new 16-bit signed integer.
         *
         * @param int|double|string $value The value as an integer, double or string
         */
        public function __construct(int|float|string|Smallint $value)
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
        public static function min(): Smallint
        {
        }

        /**
         * @ref-count 1
         */
        public static function max(): Smallint
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
