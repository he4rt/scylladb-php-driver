<?php

/** @generate-class-entries */

namespace Cassandra\Type {
    /**
     * @strict-properties
     */
    final class Map extends \Cassandra\Type
    {
        private function __construct() {}

        public function name(): string {}
        public function keyType(): \Cassandra\Type {}
        public function valueType(): \Cassandra\Type {}
        public function __toString(): string {}
        public function create(mixed ...$value): \Cassandra\Map {}
    }
}
