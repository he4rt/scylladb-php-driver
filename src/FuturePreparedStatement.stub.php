<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_future_prepared_statement
 */    final class FuturePreparedStatement implements Future {
        private function __construct() {}

        public function get(int|float|null $timeout = null): ?PreparedStatement {}

        /** @return resource */
        public function getResource(): mixed {}

        public function isReady(): bool {}
    }
}
