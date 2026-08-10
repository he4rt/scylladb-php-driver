<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
 * @scylladb-struct php_scylladb_map
 */    final class Map implements Value, \Countable, \Iterator, \ArrayAccess
    {
        public function __construct(\Cassandra\Type|string $keyType, \Cassandra\Type|string $valueType) {}

        public function type(): Type {}
        public function keys(): array {}
        public function values(): array {}
        public function set(Value|string|int|float|bool|null $key, Value|string|int|float|bool|null $value): bool {}
        public function get(Value|string|int|float|bool|null $key): Value|string|int|float|bool|null {}
        public function remove(Value|string|int|float|bool|null $key): bool {}
        public function has(Value|string|int|float|bool|null $key): bool {}

        /* Countable */
        public function count(): int {}

        /* Iterator */
        public function current(): Value|string|int|float|bool|null {}
        public function key(): Value|string|int|float|bool|null {}
        public function next(): void {}
        public function valid(): bool {}
        public function rewind(): void {}

        /* ArrayAccess */
        public function offsetSet(mixed $offset, mixed $value): void {}
        public function offsetGet(mixed $offset): Value|string|int|float|bool|null {}
        public function offsetUnset(mixed $offset): void {}
        public function offsetExists(mixed $offset): bool {}
    }
}
