<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
     * @scylladb-struct php_scylladb_collection
     * @deprecated
     */
    #[\Deprecated(message: 'Use a PHP array.', since: '1.6.0')]
    final class Collection implements Value, \Countable, \Iterator
    {
        public function __construct(\Cassandra\Type|string $type) {}

        public function type(): Type {}
        public function values(): array {}
        public function add(Value|string|int|float|bool|null ...$value): int {}
        public function get(int $index): Value|string|int|float|bool|null {}
        public function find(Value|string|int|float|bool|null $value): int|null {}
        public function remove(int $index): bool {}

        /* Countable */
        public function count(): int {}

        /* Iterator */
        public function current(): Value|string|int|float|bool|null {}
        public function key(): int {}
        public function next(): void {}
        public function valid(): bool {}
        public function rewind(): void {}
    }
}
