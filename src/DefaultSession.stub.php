<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_session
 */    final class DefaultSession implements Session {
        public function execute(string|Statement $statement, array|ExecutionOptions|null $options = null, string|\UnitEnum|null $executionProfile = null): Rows {}
        public function executeAsync(string|Statement $statement, array|ExecutionOptions|null $options = null, string|\UnitEnum|null $executionProfile = null): FutureRows {}
        public function prepare(string $cql, array|ExecutionOptions|null $options = null): PreparedStatement {}
        public function prepareAsync(string $cql, array|ExecutionOptions|null $options = null): FuturePreparedStatement {}
        public function close(int|float|null $timeout = null): void {}
        public function closeAsync(): FutureClose {}
        public function metrics(): array {}
        public function schema(): \Cassandra\Schema {}
    }
}
