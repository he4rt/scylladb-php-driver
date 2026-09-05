<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
     * @scylladb-struct php_scylladb_function
     * @deprecated
     */
    #[\Deprecated(message: 'The Rust based C/C++ driver does not implement user defined function metadata.', since: '1.6.0')]
    final class DefaultFunction implements \Cassandra\Function_ {
        public function name(): string {}

        public function simpleName(): string {}

        /** @return array<string, \Cassandra\Type> */
        public function arguments(): array {}

        /** @return \Cassandra\Type|null */
        public function returnType(): ?\Cassandra\Type {}

        public function signature(): string {}

        public function language(): string {}

        public function body(): string {}

        public function isCalledOnNullInput(): bool {}
    }
}
