<?php

/**
* @generate-class-entries
*/
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Timeuuid implements Value, UuidInterface {
        public function __construct(string|int $uuid = UNKNOWN) {}

        public function type(): Type {}
        public function time(): int {}
        public function version(): int {}
        public function uuid(): string {}

        public function __toString(): string {}
    }
}