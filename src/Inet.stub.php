<?php

/**
 * @generate-class-entries
 */
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Inet implements Value {
        public function __construct(string $address) {}

        public function type(): Type {}
        public function address(): string {}

        public function __toString(): string {}
    }
}
