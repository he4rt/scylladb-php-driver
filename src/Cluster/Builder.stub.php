<?php

/** @generate-class-entries */

namespace Cassandra\Cluster {
    /**
     * @strict-properties
     */
    final class Builder
    {
        public function withDefaultConsistency(int $consistency): Builder
        {
        }

        public function withDefaultPageSize(int $pageSize): Builder
        {
        }

        public function withDefaultTimeout(?float $timeout): Builder
        {
        }

        public function withContactPoints(string ...$host): Builder
        {
        }

        public function withPort(int $port): Builder
        {
        }

        public function withRoundRobinLoadBalancingPolicy(): Builder
        {
        }

        public function withDatacenterAwareRoundRobinLoadBalancingPolicy(
            string $localDatacenter,
            int $hostPerRemoteDatacenter,
            bool $useRemoteDatacenterForLocalConsistencies
        ): Builder {
        }

        public function withBlackListHosts(string $hosts): Builder
        {
        }

        public function withWhiteListHosts(string $hosts): Builder
        {
        }

        public function withBlackListDCs(string $dcs): Builder
        {
        }

        public function withWhiteListDCs(string $dcs): Builder
        {
        }

        public function withTokenAwareRouting(bool $enabled = true): Builder
        {
        }

        public function withCredentials(string $username, string $password): Builder
        {
        }

        public function withConnectTimeout(float $timeout): Builder
        {
        }

        public function withRequestTimeout(float $timeout): Builder
        {
        }

        public function withSSL(\Cassandra\SSLOptions $options): Builder
        {
        }

        public function withPersistentSessions(bool $enabled = true): Builder
        {
        }

        public function withProtocolVersion(int $version): Builder
        {
        }

        public function withIOThreads(int $count): Builder
        {
        }

        public function withConnectionsPerHost(int $core, int $max = 2): Builder
        {
        }

        public function withReconnectInterval(float $interval): Builder
        {
        }

        public function withLatencyAwareRouting(bool $enabled = true): Builder
        {
        }

        public function withTCPNodelay(bool $enabled = true): Builder
        {
        }

        public function withTCPKeepalive(?float $delay): Builder
        {
        }

        public function withRetryPolicy(\Cassandra\RetryPolicy $policy): Builder
        {
        }

        public function withTimestampGenerator(\Cassandra\TimestampGenerator $generator): Builder
        {
        }

        public function withSchemaMetadata(bool $enabled = true): Builder
        {
        }

        public function withHostnameResolution(bool $enabled = true): Builder
        {
        }

        public function withRandomizedContactPoints(bool $enabled = true): Builder
        {
        }

        public function withConnectionHeartbeatInterval(float $interval): Builder
        {
        }

        public function build(): \Cassandra\Cluster
        {
        }
    }
}