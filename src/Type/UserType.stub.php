<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Type {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_type
 */    final class UserType extends \Cassandra\Type
    {
        private function __construct() {}

        public function withName(string $name): UserType {}
        public function name(): ?string {}
        public function withKeyspace(string $keyspace): UserType {}
        public function keyspace(): ?string {}
        public function __toString(): string {}
        public function types(): array {}
        public function create(mixed ...$value): \Cassandra\UserTypeValue {}
    }
}
