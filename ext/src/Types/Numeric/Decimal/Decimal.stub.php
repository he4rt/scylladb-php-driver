<?php

/** @generate-class-entries */

namespace Cassandra {

    /**
     * @strict-properties
     */
    final class Decimal implements Value, Numeric {
        public function __construct(int|float|string|Decimal $value) { }

        public function __toString(): string
        {
        }

        /**
         * @ref-count 1
         */
        public function value(): string { }

        public function scale(): int { }

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

