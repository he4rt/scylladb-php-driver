<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_keyspace
 */    final class DefaultKeyspace implements \Cassandra\Keyspace {
        public function name(): string {}

        public function replicationClassName(): string {}

        /** @return array<string, mixed> */
        public function replicationOptions(): array {}

        public function hasDurableWrites(): bool {}

        /** @return Table|false */
        public function table(string $name): Table|false {}

        /** @return array<string, Table> */
        public function tables(): array {}

        /** @return \Cassandra\Type\UserType|null */
        public function userType(string $name): ?\Cassandra\Type\UserType {}

        /** @return array<string, \Cassandra\Type\UserType> */
        public function userTypes(): array {}

        /** @return MaterializedView|false */
        public function materializedView(string $name): MaterializedView|false {}

        /** @return array<string, MaterializedView> */
        public function materializedViews(): array {}

        /** @return \Cassandra\Function_|false */
        public function function(string $name, mixed ...$types): \Cassandra\Function_|false {}

        /** @return array<string, \Cassandra\Function_> */
        public function functions(): array {}

        /** @return Aggregate|false */
        public function aggregate(string $name, mixed ...$types): Aggregate|false {}

        /** @return array<string, Aggregate> */
        public function aggregates(): array {}
    }
}
