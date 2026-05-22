<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    interface Column {
        public function name(): string;

        /** @return \Cassandra\Type|null */
        public function type(): ?\Cassandra\Type;

        public function isReversed(): bool;

        public function isStatic(): bool;

        public function isFrozen(): bool;

        /** @return string|null */
        public function indexName(): ?string;

        /** @return string|null */
        public function indexOptions(): ?string;
    }
}
