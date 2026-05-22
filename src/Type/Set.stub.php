<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Type {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_type
 */    final class Set extends \Cassandra\Type
    {
        private function __construct() {}

        public function name(): string {}
        public function valueType(): \Cassandra\Type {}
        public function __toString(): string {}
        public function create(mixed ...$value): \Cassandra\Set {}
    }
}
