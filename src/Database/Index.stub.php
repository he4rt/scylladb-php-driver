<?php

/** @generate-class-entries */

namespace Cassandra {
    interface Index {
        public function name(): string;

        public function kind(): string;

        public function target(): string;

        /** @return mixed */
        public function option(string $name): mixed;

        /** @return array<string, string> */
        public function options(): array;

        /** @return string|false */
        public function className(): string|false;

        public function isCustom(): bool;
    }
}
