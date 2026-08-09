<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    /**
     * A named bundle of execution settings.
     *
     * Attach one to a cluster with
     * {@see \Cassandra\Cluster\Builder::withExecutionProfile()}, then select it per
     * statement with the `execution_profile` execution option. Anything the profile
     * does not set falls back to the cluster setting.
     *
     * The profile is copied into the cluster when the cluster is built. Later changes
     * to this object do not reach a cluster that was already built.
     *
     * @strict-properties
     * @not-serializable
     * @scylladb-struct php_scylladb_execution_profile
     */
    final class ExecutionProfile
    {
        public function __construct()
        {
        }

        public function withConsistency(int $consistency): static
        {
        }

        public function withSerialConsistency(int $consistency): static
        {
        }

        /** Seconds. */
        public function withRequestTimeout(float $timeout): static
        {
        }

        public function withRetryPolicy(\Cassandra\RetryPolicy $policy): static
        {
        }

        public function withRoundRobinLoadBalancingPolicy(): static
        {
        }

        public function withDatacenterAwareRoundRobinLoadBalancingPolicy(
            string $localDatacenter,
            int $hostPerRemoteDatacenter,
            bool $useRemoteDatacenterForLocalConsistencies
        ): static {
        }

        public function withTokenAwareRouting(bool $enabled = true): static
        {
        }

        public function withTokenAwareRoutingShuffleReplicas(bool $enabled = true): static
        {
        }

        public function withLatencyAwareRouting(bool $enabled = true): static
        {
        }

        /**
         * @param float $exclusionThreshold How much worse than the best average a node
         *                                  must be before it is skipped
         * @param float $scale              Seconds
         * @param float $retryPeriod        Seconds
         * @param float $updateRate         Seconds
         * @param int   $minMeasured        Measurements needed before a node is judged
         */
        public function withLatencyAwareRoutingSettings(
            float $exclusionThreshold,
            float $scale,
            float $retryPeriod,
            float $updateRate,
            int $minMeasured
        ): static {
        }

        /**
         * Only safe for idempotent statements: a speculative attempt runs the
         * statement more than once.
         *
         * @param float $delay Seconds
         */
        public function withConstantSpeculativeExecutionPolicy(
            float $delay,
            int $maxSpeculativeExecutions = 2
        ): static {
        }

        public function withNoSpeculativeExecutionPolicy(): static
        {
        }

        public function withWhiteListHosts(string ...$hosts): static
        {
        }

        public function withBlackListHosts(string ...$hosts): static
        {
        }

        public function withWhiteListDCs(string ...$dcs): static
        {
        }

        public function withBlackListDCs(string ...$dcs): static
        {
        }
    }
}
