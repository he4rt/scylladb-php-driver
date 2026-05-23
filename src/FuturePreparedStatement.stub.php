<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_future_prepared_statement
 */    final class FuturePreparedStatement implements Future {
        public function get(int|float|null $timeout = null): mixed {}
    }
}
