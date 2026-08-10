<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @not-serializable
 * @scylladb-struct php_scylladb_table
 */    final class DefaultTable implements \Cassandra\Table {
        public function name(): string {}

        public function option(string $name): Value|string|int|float|bool|null {}

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

        public function populateIOCacheOnFlush(): bool {}

        public function replicateOnWrite(): bool {}

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

        /** @return Index|false */
        public function index(string $name): Index|false {}

        /** @return array<string, Index> */
        public function indexes(): array {}

        /** @return MaterializedView|false */
        public function materializedView(string $name): MaterializedView|false {}

        /** @return array<string, MaterializedView> */
        public function materializedViews(): array {}
    }
}
