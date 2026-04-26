<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class DefaultSchema implements \Cassandra\Schema {
        /** @return Keyspace|false */
        public function keyspace(string $name): Keyspace|false {}

        /** @return array<string, Keyspace> */
        public function keyspaces(): array {}

        public function version(): int {}
    }
}
