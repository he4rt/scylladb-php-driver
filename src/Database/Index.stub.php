<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    #[\Deprecated(message: 'The Rust based C/C++ driver does not implement secondary index metadata.', since: '1.6.0')]
    interface Index {
        public function name(): string;

        public function kind(): string;

        public function target(): string;

        public function option(string $name): Value|string|int|float|bool|null;

        /** @return array<string, string> */
        public function options(): array;

        /** @return string|false */
        public function className(): string|false;

        public function isCustom(): bool;
    }
}
