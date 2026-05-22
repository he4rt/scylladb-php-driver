<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_tuple
 */    final class Tuple implements Value, \Countable, \Iterator
    {
        public function __construct(array $types) {}

        public function type(): Type {}
        public function values(): array {}
        public function set(int $index, mixed $value): void {}
        public function get(int $index): mixed {}

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
