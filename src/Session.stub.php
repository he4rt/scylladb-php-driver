<?php

/** @generate-class-entries */

namespace Cassandra {
    interface Session {
        public function execute(string|Statement $statement, array|ExecutionOptions|null $options = null): Rows;
        public function executeAsync(string|Statement $statement, array|ExecutionOptions|null $options = null): FutureRows;
        public function prepare(string $cql, array|ExecutionOptions|null $options = null): PreparedStatement;
        public function prepareAsync(string $cql, array|ExecutionOptions|null $options = null): FuturePreparedStatement;
        public function close(int|float|null $timeout = null): void;
        public function closeAsync(): FutureClose;
        public function metrics(): array;
        public function schema(): \Cassandra\Schema;
    }
}
