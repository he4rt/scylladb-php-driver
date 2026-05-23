<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_rows
 */    final class Rows implements \Iterator, \Countable, \ArrayAccess {
        public function __construct() {}
        public function count(): int {}
        public function rewind(): void {}
        public function current(): mixed {}
        public function key(): mixed {}
        public function next(): void {}
        public function valid(): bool {}
        public function offsetExists(mixed $offset): bool {}
        public function offsetGet(mixed $offset): mixed {}
        public function offsetSet(mixed $offset, mixed $value): void {}
        public function offsetUnset(mixed $offset): void {}
        public function isLastPage(): bool {}
        public function nextPage(int|float|null $timeout = null): Rows|false {}
        public function nextPageAsync(): Future {}
        public function pagingStateToken(): ?string {}
        public function first(): mixed {}
    }
}
