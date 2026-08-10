<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_aggregate
 */    final class DefaultAggregate implements \Cassandra\Aggregate {
        public function name(): string {}

        public function simpleName(): string {}

        /** @return array<int, \Cassandra\Type> */
        public function argumentTypes(): array {}

        /** @return \Cassandra\Function_|null */
        public function finalFunction(): ?\Cassandra\Function_ {}

        /** @return \Cassandra\Function_|null */
        public function stateFunction(): ?\Cassandra\Function_ {}

        public function initialCondition(): Value|string|int|float|bool|null {}

        /** @return \Cassandra\Type|null */
        public function returnType(): ?\Cassandra\Type {}

        /** @return \Cassandra\Type|null */
        public function stateType(): ?\Cassandra\Type {}

        public function signature(): string {}
    }
}
