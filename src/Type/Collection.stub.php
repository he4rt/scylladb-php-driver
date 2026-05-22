<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Type {
    /**
     * @strict-properties
     */
    final class Collection extends \Cassandra\Type
    {
        private function __construct() {}

        public function name(): string {}
        public function valueType(): \Cassandra\Type {}
        public function __toString(): string {}
        public function create(mixed ...$value): \Cassandra\Collection {}
    }
}
