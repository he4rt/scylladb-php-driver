<?php

/** @generate-class-entries */

namespace Cassandra {

    /**
     * @strict-properties
     */
    final class Varint implements Value, Numeric
    {
        /**
         * Creates a new variable length integer.
         *
         * @param string $value integer value as a string
         */
        public function __construct(string|Varint $value)
        {
        }

        public function __toString(): string
        {
        }

        /**
         * Returns the integer value.
         *
         * @return string integer value
         */
        public function value(): string
        {
        }

        public function type(): Type
        {
        }

        public function add(Numeric $num): Numeric
        {
        }

        public function sub(Numeric $num): Numeric
        {
        }

        public function mul(Numeric $num): Numeric
        {
        }

        public function div(Numeric $num): Numeric
        {
        }

        public function mod(Numeric $num): Numeric
        {
        }

        public function abs(): Numeric
        {
        }

        public function neg(): Numeric
        {
        }

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
