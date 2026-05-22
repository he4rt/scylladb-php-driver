<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Cluster {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_cluster_builder
 */    final class Builder
    {
        public function withDefaultConsistency(int $consistency): static
        {
        }

        public function withDefaultPageSize(int $pageSize): static
        {
        }

        public function withDefaultTimeout(float $timeout): static
        {
        }

        public function withContactPoints(string ...$host): static
        {
        }

        public function withPort(int $port): static
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

        public function withBlackListHosts(string ...$hosts): static
        {
        }

        public function withWhiteListHosts(string ...$hosts): static
        {
        }

        public function withBlackListDCs(string ...$dcs): static
        {
        }

        public function withWhiteListDCs(string ...$dcs): static
        {
        }

        public function withTokenAwareRouting(bool $enabled = true): static
        {
        }

        public function withCredentials(string $username, #[\SensitiveParameter] string $password): static
        {
        }

        public function withConnectTimeout(float $timeout): static
        {
        }

        public function withRequestTimeout(float $timeout): static
        {
        }

        public function withSSL(\Cassandra\SSLOptions $options): static
        {
        }

        public function withPersistentSessions(bool $enabled = true): static
        {
        }

        public function withProtocolVersion(int $version): static
        {
        }

        public function withIOThreads(int $count): static
        {
        }

        public function withConnectionsPerHost(int $core, int $max = 2): static
        {
        }

        public function withReconnectInterval(float $interval): static
        {
        }

        public function withLatencyAwareRouting(bool $enabled = true): static
        {
        }

        public function withTCPNodelay(bool $enabled = true): static
        {
        }

        public function withTCPKeepalive(?float $delay): static
        {
        }

        public function withRetryPolicy(\Cassandra\RetryPolicy $policy): static
        {
        }

        public function withTimestampGenerator(\Cassandra\TimestampGenerator $generator): static
        {
        }

        public function withSchemaMetadata(bool $enabled = true): static
        {
        }

        public function withHostnameResolution(bool $enabled = true): static
        {
        }

        public function withRandomizedContactPoints(bool $enabled = true): static
        {
        }

        public function withConnectionHeartbeatInterval(float $interval): static
        {
        }

        public function build(): \Cassandra\Cluster
        {
        }
    }
}