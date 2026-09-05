<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Cluster {
    /**
     * @strict-properties
     * @not-serializable
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

        /**
         * ScyllaDB only. Tries live nodes in $localRack first, then the rest of
         * $localDatacenter, then remote datacenters.
         *
         * Pass empty strings to let the driver infer both from the first contact
         * point it reaches; list contact points from the local rack only if you
         * rely on that.
         */
        public function withRackAwareLoadBalancingPolicy(
            string $localDatacenter = '',
            string $localRack = ''
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

        public function withProtocolVersion(\Cassandra\ProtocolVersion|int $version): static
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

        #[\Deprecated(message: 'The Rust based C/C++ driver does not implement hostname resolution.', since: '1.6.0')]
        public function withHostnameResolution(bool $enabled = true): static
        {
        }

        public function withRandomizedContactPoints(bool $enabled = true): static
        {
        }

        public function withConnectionHeartbeatInterval(float $interval): static
        {
        }

        /**
         * Reported to the server and visible in system.clients.client_options
         * as APPLICATION_NAME.
         */
        public function withApplicationName(string $name): static
        {
        }

        /** Visible in system.clients.client_options as APPLICATION_VERSION. */
        public function withApplicationVersion(string $version): static
        {
        }

        /**
         * Backs off from $baseInterval to $maxInterval, both in seconds, with
         * jitter. Replaces the constant delay set by withReconnectInterval().
         */
        public function withExponentialReconnect(float $baseInterval, float $maxInterval): static
        {
        }

        /**
         * Re-sends a request to another replica after $delay seconds when the
         * first replica has not answered. Cuts tail latency.
         *
         * Only safe for idempotent statements. A speculative attempt runs the
         * statement more than once, so never enable it for counter updates,
         * lightweight transactions, or appends to a list.
         */
        public function withConstantSpeculativeExecutionPolicy(
            float $delay,
            int $maxSpeculativeExecutions = 2
        ): static {
        }

        /** Turns speculative execution off. This is the default. */
        public function withNoSpeculativeExecutionPolicy(): static
        {
        }

        /** Microseconds the driver waits to batch writes into one system call. */
        public function withCoalesceDelay(int $microseconds): static
        {
        }

        /** Splits IO thread time between new and outstanding requests, 1 to 100. */
        #[\Deprecated(message: 'The Rust based C/C++ driver ignores the request ratio.', since: '1.6.0')]
        public function withNewRequestRatio(int $ratio): static
        {
        }

        /**
         * Registers a named execution profile.
         *
         * Select it per statement with the `execution_profile` execution option,
         * which takes the same string or enum case. Anything the profile does not
         * set falls back to the cluster setting. Calling this again with the same
         * name replaces the profile.
         *
         * $name may be a string or an enum case. A string-backed enum contributes
         * its value, any other enum its case name:
         *
         *     enum Profile: string { case Analytics = 'analytics'; }
         *     $builder->withExecutionProfile(Profile::Analytics, $profile);
         *     $session->execute($stmt, ['execution_profile' => Profile::Analytics]);
         */
        public function withExecutionProfile(string|\UnitEnum $name, \Cassandra\ExecutionProfile $profile): static
        {
        }

        public function build(): \Cassandra\Cluster
        {
        }
    }
}