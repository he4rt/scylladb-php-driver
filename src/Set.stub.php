<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_set
 */    final class Set implements Value, \Countable, \Iterator
    {
        public function __construct(\Cassandra\Type|string $type) {}

        public function type(): Type {}
        public function values(): array {}
        public function add(Value|string|int|float|bool|null $value): bool {}
        public function has(Value|string|int|float|bool|null $value): bool {}
        public function remove(Value|string|int|float|bool|null $value): bool {}

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
