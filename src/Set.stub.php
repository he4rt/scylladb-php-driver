<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
     */
    final class Set implements Value, \Countable, \Iterator
    {
        public function __construct(\Cassandra\Type|string $type) {}

        public function type(): Type {}
        public function values(): array {}
        public function add(mixed $value): bool {}
        public function has(mixed $value): bool {}
        public function remove(mixed $value): bool {}

        /* Countable */
        public function count(): int {}

        /* Iterator */
        public function current(): mixed {}
        public function key(): int {}
        public function next(): void {}
        public function valid(): bool {}
        public function rewind(): void {}
    }
}
