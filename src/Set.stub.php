<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Set implements Value, \Countable, \Iterator
    {
        public function __construct(mixed $type) {}

        public function type(): Type {}
        public function values(): array {}
        public function add(mixed $value): bool {}
        public function has(mixed $value): bool {}
        public function remove(mixed $value): bool {}

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
