<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
     * @scylladb-struct php_scylladb_user_type_value
     * @deprecated
     */
    #[\Deprecated(message: 'Use a PHP array.', since: '1.6.0')]
    final class UserTypeValue implements Value, \Countable, \Iterator
    {
        public function __construct(array $types) {}

        public function type(): Type {}
        public function values(): array {}
        public function set(string $name, Value|string|int|float|bool|null $value): void {}
        public function get(string $name): Value|string|int|float|bool|null {}

        /* Countable */
        public function count(): int {}

        /* Iterator */
        public function current(): Value|string|int|float|bool|null {}
        public function key(): string {}
        public function next(): void {}
        public function valid(): bool {}
        public function rewind(): void {}
    }
}
