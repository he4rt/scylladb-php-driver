<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Map implements Value, \Countable, \Iterator, \ArrayAccess
    {
        public function __construct(\Cassandra\Type|string $keyType, \Cassandra\Type|string $valueType) {}

        public function type(): Type {}
        public function keys(): array {}
        public function values(): array {}
        public function set(mixed $key, mixed $value): bool {}
        public function get(mixed $key): mixed {}
        public function remove(mixed $key): bool {}
        public function has(mixed $key): bool {}

        /* Countable */
        public function count(): int {}

        /* Iterator */
        public function current(): mixed {}
        public function key(): mixed {}
        public function next(): void {}
        public function valid(): bool {}
        public function rewind(): void {}

        /* ArrayAccess */
        public function offsetSet(mixed $offset, mixed $value): void {}
        public function offsetGet(mixed $offset): mixed {}
        public function offsetUnset(mixed $offset): void {}
        public function offsetExists(mixed $offset): bool {}
    }
}
