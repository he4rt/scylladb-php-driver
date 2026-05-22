<?php

/**
* @generate-class-entries
*/

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_uuid
 */    final class Timeuuid implements Value, UuidInterface {
        public function __construct(string|int $uuid = UNKNOWN) {}

        public function type(): Type {}
        public function time(): int {}
        public function version(): int {}
        public function uuid(): string {}

        public function __toString(): string {}
    }
}