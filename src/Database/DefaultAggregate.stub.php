<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class DefaultAggregate implements \Cassandra\Aggregate {
        public function name(): string {}

        public function simpleName(): string {}

        /** @return array<int, \Cassandra\Type> */
        public function argumentTypes(): array {}

        /** @return \Cassandra\Function_|null */
        public function finalFunction(): ?\Cassandra\Function_ {}

        /** @return \Cassandra\Function_|null */
        public function stateFunction(): ?\Cassandra\Function_ {}

        /** @return mixed */
        public function initialCondition(): mixed {}

        /** @return \Cassandra\Type|null */
        public function returnType(): ?\Cassandra\Type {}

        /** @return \Cassandra\Type|null */
        public function stateType(): ?\Cassandra\Type {}

        public function signature(): string {}
    }
}
