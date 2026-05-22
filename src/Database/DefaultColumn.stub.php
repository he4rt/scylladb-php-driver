<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class DefaultColumn implements \Cassandra\Column {
        public function name(): string {}

        /** @return \Cassandra\Type|null */
        public function type(): ?\Cassandra\Type {}

        /** @deprecated */
        public function isReversed(): bool {}

        public function isStatic(): bool {}

        public function isFrozen(): bool {}

        /** @deprecated */
        /** @return string|null */
        public function indexName(): ?string {}

        /** @deprecated */
        /** @return string|null */
        public function indexOptions(): ?string {}
    }
}
