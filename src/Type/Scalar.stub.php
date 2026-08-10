<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Type {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_type
 */    final class Scalar extends \Cassandra\Type
    {
        private function __construct() {}

        public function name(): string {}
        public function __toString(): string {}
        public function create(\Cassandra\Value|string|int|float|bool|null ...$args): \Cassandra\Value|string|int|float|bool|null {}
    }
}
