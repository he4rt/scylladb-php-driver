<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class UserTypeValue implements Value, \Countable, \Iterator
    {
        public function __construct(array $types) {}

        public function type(): Type {}
        public function values(): array {}
        public function set(string $name, mixed $value): void {}
        public function get(string $name): mixed {}

        /* Countable */
        public function count(): int {}

        /* Iterator */
        public function current(): mixed {}
        public function key(): mixed {}
        public function next(): void {}
        public function valid(): bool {}
        public function rewind(): void {}
    }
}
