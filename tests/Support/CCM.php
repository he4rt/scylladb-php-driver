<?php

declare(strict_types=1);

namespace Cassandra\Tests\Support;

use Cassandra;
use Cassandra\Exception;
use Cassandra\Session;
use Cassandra\SimpleStatement;
use RuntimeException;
use Symfony\Component\Process\Process;

class CCM
{
    private ?Session $session = null;

    public function __construct(
        private readonly string $clusterPrefix,
        private readonly string $schema,
        private readonly string $version,
        private readonly bool $ssl = false,
        private readonly bool $clientAuth = false,
        private readonly int $clusterNodes = 1,
        private readonly bool $dropExistingKeyspaces = true,
        private readonly int $clusterStartRetries = 10,
        private readonly bool $scylladb = false,

    ) {
    }

    /**
     * @throws Exception
     */
    private function dropSchema(): void {
        $keyspaces = $this->session->execute(new SimpleStatement("SELECT keyspace_name FROM system_schema.keyspaces"), []);

        foreach ($keyspaces as $row) {
            $keyspace = $row['keyspace_name'];

            if (str_starts_with($keyspace, 'system')) {
                continue;
            }

            $this->session?->execute(new SimpleStatement("DROP KEYSPACE $keyspace"), []);
        }
    }

    /**
     * @throws Exception
     */
    private function setupSchema(Session $conn): void {
        foreach (explode(";\n", $this->schema) as $cql) {
            $cql = trim($cql);

            if (empty($cql)) {
                continue;
            }

            $conn->execute(new SimpleStatement($cql), []);
        }
    }

    private function run(string ...$args): string
    {
        foreach ($args as $i => $arg) {
            $args[$i] = escapeshellarg($arg);
        }

        $process = new Process([
            'ccm',
            ...$args,
        ]);

        $process->mustRun();
        return $process->getOutput();
    }

    private function isActiveCluster(string $clusterName): bool
    {
        return str_starts_with($clusterName, ' *');
    }

    private function getClusters(): array
    {
        $active = '';
        $clusters = array();
        foreach (explode(PHP_EOL, $this->run('list')) as $cluster) {
            $clusterName = trim(substr($cluster, 2));

            if ($this->isActiveCluster($cluster)) {
                $active = $clusterName;
            }

            if (!empty($clusterName)) {
                $clusters[] = $clusterName;
            }
        }

        return ['current' => $active, 'clusters' => $clusters];
    }

    public function remove(): void
    {
        $this->run('remove', $this->getClusterName());
    }

    public function removeAll(): void {
        ['clusters' => $clusters] = $this->getClusters();

        foreach($clusters as $clusterName) {
            $this->run('remove', $clusterName);
        }
    }

    public function getClusterName(): string
    {
        $clusterName = $this->clusterPrefix . '_' . $this->clusterNodes;

        if ($this->ssl) {
            $clusterName .= "_ssl";
        }

        if ($this->clientAuth) {
            $clusterName .= "_client_auth";
        }

        return $clusterName;
    }

    private function setupCassandra(): void {
        $clusterName = $this->getClusterName();
        ['current' => $currentRunningCluster, 'clusters' => $clusters] = $this->getClusters();

        if($currentRunningCluster === $clusterName) {
            return;
        }

        if (!empty($currentRunningCluster)) {
            $this->stop($currentRunningCluster);
        }

        if (in_array($clusterName, $clusters, true)) {
            $this->run('switch', $clusterName);
            return;
        }

        $this->run('create', '-v', 'binary:' . $this->version, '-b', $clusterName);

        $this->run(
            'updateconf',
            '--rt', '1000',
            'read_request_timeout_in_ms: 1000',
            'write_request_timeout_in_ms: 1000',
            'request_timeout_in_ms: 1000',
            'phi_convict_threshold: 16',
            'hinted_handoff_enabled: false',
            'dynamic_snitch_update_interval_in_ms: 1000',
            'cas_contention_timeout_in_ms: 10000',
            'file_cache_size_in_mb: 0',
            'native_transport_max_threads: 1',
            'rpc_min_threads: 1',
            'rpc_max_threads: 1',
            'concurrent_reads: 2',
            'concurrent_writes: 2',
            'concurrent_compactors: 1',
            'compaction_throughput_mb_per_sec: 0',
            'enable_user_defined_functions: true',
            'enable_scripted_user_defined_functions: true',
            'key_cache_size_in_mb: 0',
            'key_cache_save_period: 0',
            'memtable_flush_writers: 1',
            'max_hints_delivery_threads: 1',
            ...($this->ssl ? [
                'client_encryption_options.enabled: true',
                'client_encryption_options.keystore: ' . realpath(__DIR__ . '/ssl/.keystore'),
            'client_encryption_options.keystore_password: php-driver'
        ] : []),
            ...($this->clientAuth ? [
            'client_encryption_options.enabled: true',
            'client_encryption_options.keystore: ' . realpath(__DIR__ . '/ssl/.keystore'),
            'client_encryption_options.keystore_password: php-driver',
            'client_encryption_options.require_client_auth: true',
            'client_encryption_options.truststore: ' . realpath(__DIR__ . '/ssl/.truststore'),
            'client_encryption_options.truststore_password: php-driver'
        ]: []),
        );

        $this->run('populate', '-n', $this->clusterNodes.':0', '-i', '127.0.0.');
    }

    /**
     * @throws Exception
     */
    public function start(): Session
    {
        if (!$this->scylladb) {
            $this->setupCassandra();
        }

        $this->run('start', '--wait-other-notice', '--wait-for-binary-proto');

        if($this->dropExistingKeyspaces) {
            $this->dropSchema();
        }

        $builder = Cassandra::cluster()
            ->withPersistentSessions(false)
            ->withContactPoints('127.0.0.1');


        if ($this->ssl) {
            $builder->withSSL(
                Cassandra::ssl()
                    ->withTrustedCerts(realpath(__DIR__ . '/ssl/cassandra.pem'))
                    ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT)
                    ->withClientCert(realpath(__DIR__ . '/ssl/driver.pem'))
                    ->withPrivateKey(realpath(__DIR__ . '/ssl/driver.key'), 'php-driver')
                    ->build()
            );
        }

        for ($retries = 0; $retries <= $this->clusterStartRetries; $retries++) {
            try {
                $cluster = $builder->build();
                $this->session = $cluster->connect();
                $this->setupSchema($this->session);
                return $this->session;
            } catch (Cassandra\Exception\RuntimeException) {
                sleep($retries * 2);
            }
        }

        throw new RuntimeException("Unable to initialize a Session, check cassandra logs");
    }

    public function stop(string $name = ''): void
    {
        $this->run('stop', $name === '' ? $this->getClusterName() : $name);
    }

    public function getSession(string $keyspace = ''): Session {
        $builder = Cassandra::cluster()
            ->withPersistentSessions(false)
            ->withContactPoints('127.0.0.1');

        if ($this->ssl) {
            $builder->withSSL(
                Cassandra::ssl()
                    ->withTrustedCerts(realpath(__DIR__ . '/ssl/cassandra.pem'))
                    ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT)
                    ->withClientCert(realpath(__DIR__ . '/ssl/driver.pem'))
                    ->withPrivateKey(realpath(__DIR__ . '/ssl/driver.key'), 'php-driver')
                    ->build()
            );
        }

       return $builder->build()->connect($keyspace);
    }
}