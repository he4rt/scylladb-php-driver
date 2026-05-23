<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_index
 */    final class DefaultIndex implements \Cassandra\Index {
        public function name(): string {}

        public function kind(): string {}

        public function target(): string {}

        /** @return mixed */
        public function option(string $name): mixed {}

        /** @return array<string, string> */
        public function options(): array {}

        /** @return string|false */
        public function className(): string|false {}

        public function isCustom(): bool {}
    }
}
