<?php

/** @generate-class-entries */

namespace Cassandra\Type {
    /**
     * @strict-properties
     */
    final class Scalar extends \Cassandra\Type
    {
        private function __construct() {}

        public function name(): string {}
        public function __toString(): string {}
        public function create(mixed $value = null): \Cassandra\Value {}
    }
}
