<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_schema
 */    final class DefaultSchema implements \Cassandra\Schema {
        /** @return Keyspace|false */
        public function keyspace(string $name): Keyspace|false {}

        /** @return array<string, Keyspace> */
        public function keyspaces(): array {}

        public function version(): int {}
    }
}
