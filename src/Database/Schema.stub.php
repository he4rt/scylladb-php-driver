<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    interface Schema {
        /** @return Keyspace|false */
        public function keyspace(string $name): Keyspace|false;

        /** @return array<string, Keyspace> */
        public function keyspaces(): array;
    }
}
