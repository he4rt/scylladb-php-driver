<?php

/**
 * @generate-class-entries
 */
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Uuid implements Value, UuidInterface {
        public function __construct(string $uuid = UNKNOWN) {}

        public function uuid(): string {}
        public function version(): int {}
        public function type(): Type {}

        public function __toString(): string {}
    }
}
