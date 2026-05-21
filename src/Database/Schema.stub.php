<?php

/** @generate-class-entries */

namespace Cassandra {
    interface Schema {
        /** @return Keyspace|false */
        public function keyspace(string $name): Keyspace|false;

        /** @return array<string, Keyspace> */
        public function keyspaces(): array;
    }
}
