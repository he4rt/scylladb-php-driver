<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Type {
    /**
     * @strict-properties
     */
    final class Tuple extends \Cassandra\Type
    {
        private function __construct() {}

        public function name(): string {}
        public function __toString(): string {}
        public function types(): array {}
        public function create(mixed ...$values): \Cassandra\Tuple {}
    }
}
