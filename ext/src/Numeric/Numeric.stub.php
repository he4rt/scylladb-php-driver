<?php

/** @generate-class-entries */

namespace Cassandra {
    interface Numeric {
        /**
         * @param \Cassandra\Numeric $num a number to add to this one
         * @return \Cassandra\Numeric sum
         */
        public function add(Numeric $num): Numeric;

        /**
         * @param \Cassandra\Numeric $num a number to subtract from this one
         * @return \Cassandra\Numeric difference
         */
        public function sub(Numeric $num): Numeric;

        /**
         * @param \Cassandra\Numeric $num a number to multiply this one by
         * @return \Cassandra\Numeric product
         */
        public function mul(Numeric $num): Numeric;

        /**
         * @param \Cassandra\Numeric $num a number to divide this one by
         * @return \Cassandra\Numeric quotient
         */
        public function div(Numeric $num): Numeric;

        /**
         * @param \Cassandra\Numeric $num a number to divide this one by
         * @return \Cassandra\Numeric remainder
         */
        public function mod(Numeric $num): Numeric;

        /**
         * @return \Cassandra\Numeric absolute value
         */
        public function abs(): Numeric;

        /**
         * @return \Cassandra\Numeric negative value
         */
        public function neg(): Numeric;

        /**
         * @return \Cassandra\Numeric square root
         */
        public function sqrt(): Numeric;

        /**
         * @return int this number as int
         */
        public function toInt(): int;

        /**
         * @return float this number as float
         */
        public function toDouble(): float;
    }
}

