<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_future_rows
 */    final class FutureRows implements Future {
        private function __construct() {}

        public function get(int|float|null $timeout = null): ?Rows {}

        /** @return resource */
        public function getResource(): mixed {}

        public function isReady(): bool {}
    }
}
