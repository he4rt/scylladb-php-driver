<?php

/** @generate-class-entries */

namespace Cassandra {
    interface Numeric
    {
        public function add(Numeric $num): Numeric;

        public function sub(Numeric $num): Numeric;

        public function mul(Numeric $num): Numeric;

        public function div(Numeric $num): Numeric;

        public function mod(Numeric $num): Numeric;

        public function abs(): Numeric;

        public function neg(): Numeric;

        public function sqrt(): Numeric;

        public function toInt(): int;

        public function toDouble(): float;
    }
}
