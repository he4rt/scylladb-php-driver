<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Collection implements Value, \Countable, \Iterator
    {
        public function __construct(mixed $type) {}

        public function type(): Type {}
        public function values(): array {}
        public function add(mixed ...$value): int {}
        public function get(int $index): mixed {}
        public function find(mixed $value): int|null {}
        public function remove(int $index): bool {}

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
