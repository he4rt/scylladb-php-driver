<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_future_value
 */    final class FutureValue implements Future {
        private function __construct() {}

        public function get(int|float|null $timeout = null): Value|string|int|float|bool|null {}

        /** @return resource */
        public function getResource(): mixed {}

        public function isReady(): bool {}
    }
}
