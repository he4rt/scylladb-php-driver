<?php

/** @generate-class-entries */

namespace Cassandra {
    /**
     * @strict-properties
     */
    final class DefaultMaterializedView extends \Cassandra\MaterializedView {
        public function name(): string {}

        /** @return mixed */
        public function option(string $name): mixed {}

        /** @return array<string, mixed> */
        public function options(): array {}

        /** @return string|false */
        public function comment(): string|false {}

        /** @return float|false */
        public function readRepairChance(): float|false {}

        /** @return float|false */
        public function localReadRepairChance(): float|false {}

        /** @return int|false */
        public function gcGraceSeconds(): int|false {}

        /** @return string|false */
        public function caching(): string|false {}

        /** @return float|false */
        public function bloomFilterFPChance(): float|false {}

        /** @return int|false */
        public function memtableFlushPeriodMs(): int|false {}

        /** @return int|false */
        public function defaultTTL(): int|false {}

        /** @return string|false */
        public function speculativeRetry(): string|false {}

        /** @return int|false */
        public function indexInterval(): int|false {}

        /** @return string|false */
        public function compactionStrategyClassName(): string|false {}

        /** @return array<string, mixed>|false */
        public function compactionStrategyOptions(): array|false {}

        /** @return array<string, mixed>|false */
        public function compressionParameters(): array|false {}

        /** @return bool|false */
        public function populateIOCacheOnFlush(): bool|false {}

        /** @return bool|false */
        public function replicateOnWrite(): bool|false {}

        /** @return int|false */
        public function maxIndexInterval(): int|false {}

        /** @return int|false */
        public function minIndexInterval(): int|false {}

        /** @return Column|false */
        public function column(string $name): Column|false {}

        /** @return array<string, Column> */
        public function columns(): array {}

        /** @return array<int, Column> */
        public function partitionKey(): array {}

        /** @return array<int, Column> */
        public function primaryKey(): array {}

        /** @return array<int, Column> */
        public function clusteringKey(): array {}

        /** @return array<int, string> */
        public function clusteringOrder(): array {}

        /** @return Table|null */
        public function baseTable(): ?Table {}
    }
}
