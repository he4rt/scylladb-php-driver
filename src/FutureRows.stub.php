<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_future_rows
 */    final class FutureRows implements Future {
        public function get(int|float|null $timeout = null): mixed {}

        /** @return resource */
        public function getResource(): mixed {}

        public function isReady(): bool {}
    }
}
