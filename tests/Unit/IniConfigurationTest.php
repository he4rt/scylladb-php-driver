<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

/**
 * Every driver directive is PHP_INI_SYSTEM, so each case runs in a child
 * process. A null result means the child could not load the extension —
 * skip rather than fail, matching ClusterBuilderCredentialsTest.
 *
 * @param array<string, scalar> $ini
 */
function builderProps(array $ini): ?array
{
    $result = phpWithIni('echo serialize((array) (new Cassandra\Cluster\Builder));', $ini);

    if ($result === null || $result['exit'] !== 0) {
        return null;
    }

    $props = @unserialize((string) $result['payload']);

    return is_array($props) ? $props : null;
}

describe('php.ini configuration', function () {

    it('seeds Cluster\Builder defaults from php.ini', function () {
        $props = builderProps([
            'cassandra.contact_points'              => '10.0.0.1,10.0.0.2',
            'cassandra.port'                        => 19042,
            'cassandra.default_consistency'         => 'LOCAL_QUORUM',
            'cassandra.default_page_size'           => 250,
            'cassandra.protocol_version'            => 3,
            'cassandra.io_threads'                  => 4,
            'cassandra.core_connections_per_host'   => 2,
            'cassandra.max_connections_per_host'    => 8,
            'cassandra.connect_timeout'             => 3000,
            'cassandra.request_timeout'             => 7000,
            'cassandra.reconnect_interval'          => 4000,
            'cassandra.connection_heartbeat_interval' => 15,
            'cassandra.token_aware_routing'         => 0,
            'cassandra.latency_aware_routing'       => 0,
            'cassandra.tcp_nodelay'                 => 0,
            'cassandra.schema_metadata'             => 0,
        ]);

        if ($props === null) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect($props['contactPoints'])->toBe('10.0.0.1,10.0.0.2')
            ->and($props['defaultConsistency'])->toBe(\Cassandra::CONSISTENCY_LOCAL_QUORUM)
            ->and($props['defaultPageSize'])->toBe(250)
            ->and($props['protocolVersion'])->toBe(\Cassandra\ProtocolVersion::V3)
            ->and($props['ioThreads'])->toBe(4)
            ->and($props['coreConnectionPerHost'])->toBe(2)
            ->and($props['maxConnectionsPerHost'])->toBe(8)
            ->and($props['connectTimeout'])->toBe(3.0)
            ->and($props['requestTimeout'])->toBe(7.0)
            ->and($props['reconnectInterval'])->toBe(4.0)
            ->and($props['connectionHeartbeatInterval'])->toBe(15)
            ->and($props['useTokenAwareRouting'])->toBeFalse()
            ->and($props['latencyAwareRouting'])->toBeFalse()
            ->and($props['tcpNodelay'])->toBeFalse()
            ->and($props['schemaMetadata'])->toBeFalse();
    });

    it('uses the compiled defaults when php.ini says nothing', function () {
        $props = builderProps([]);

        if ($props === null) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect($props['contactPoints'])->toBe('127.0.0.1')
            ->and($props['defaultConsistency'])->toBe(\Cassandra::CONSISTENCY_LOCAL_QUORUM)
            ->and($props['defaultPageSize'])->toBe(5000)
            ->and($props['protocolVersion'])->toBe(\Cassandra\ProtocolVersion::V4)
            ->and($props['usePersistentSessions'])->toBeTrue();
    });

    it('lets a with*() call override its php.ini seed', function () {
        $result = phpWithIni(
            'echo ((array) (new Cassandra\Cluster\Builder)->withDefaultPageSize(42))["defaultPageSize"];',
            ['cassandra.default_page_size' => 250],
        );

        if ($result === null || $result['exit'] !== 0) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect(trim($result['payload']))->toBe('42');
    });

    it('rejects an out-of-range number, warns, and keeps the default', function () {
        $result = phpWithIni(
            'echo json_encode(["ini" => ini_get("cassandra.protocol_version"),'
            . ' "driver" => ((array) new Cassandra\Cluster\Builder)["protocolVersion"]]);',
            ['cassandra.port' => 99999, 'cassandra.protocol_version' => 0],
        );

        if ($result === null || $result['exit'] !== 0) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect($result['stderr'])
            ->toContain("cassandra.port must be between 1 and 65535, ignoring '99999' and using the default")
            ->and($result['stderr'])
            ->toContain("cassandra.protocol_version must be between 1 and 5, ignoring '0' and using the default");

        /* The whole point of rejecting instead of clamping: what ini_get()
           reports is what the driver uses. */
        $seen = json_decode((string) $result['payload'], true);

        expect($seen['ini'])->toBe('4')->and($seen['driver'])->toBe(4);
    });

    it('reports the default through ini_get for every rejected directive', function () {
        $result = phpWithIni(
            'echo json_encode(array_map("ini_get", ["cassandra.port", "cassandra.io_threads",'
            . ' "cassandra.default_page_size", "cassandra.default_consistency"]));',
            [
                'cassandra.port'                => 99999,
                'cassandra.io_threads'          => -3,
                'cassandra.default_page_size'   => 0,
                'cassandra.default_consistency' => 'BOGUS',
            ],
        );

        if ($result === null || $result['exit'] !== 0) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect(json_decode((string) $result['payload'], true))
            ->toBe(['9042', '1', '5000', 'LOCAL_QUORUM']);
    });

    it('falls back to the default on an unknown consistency name', function () {
        $result = phpWithIni(
            'echo ((array) new Cassandra\Cluster\Builder)["defaultConsistency"];',
            ['cassandra.default_consistency' => 'NOT_A_CONSISTENCY'],
        );

        if ($result === null || $result['exit'] !== 0) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect((int) trim($result['payload']))->toBe(\Cassandra::CONSISTENCY_LOCAL_QUORUM)
            ->and($result['stderr'])->toContain("Unknown consistency 'NOT_A_CONSISTENCY'");
    });

    it('accepts a consistency name in any case', function () {
        $result = phpWithIni(
            'echo ((array) new Cassandra\Cluster\Builder)["defaultConsistency"];',
            ['cassandra.default_consistency' => 'each_quorum'],
        );

        if ($result === null || $result['exit'] !== 0) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect((int) trim($result['payload']))->toBe(\Cassandra::CONSISTENCY_EACH_QUORUM)
            ->and($result['stderr'])->not->toContain('Unknown consistency');
    });

    it('refuses ini_set on a driver directive', function () {
        $result = phpWithIni(
            'var_export([@ini_set("cassandra.port", "1234"), ini_get("cassandra.port")]);',
        );

        if ($result === null || $result['exit'] !== 0) {
            $this->markTestSkipped('child process could not load the extension');
        }

        expect($result['payload'])->toBe("array (\n  0 => false,\n  1 => '9042',\n)");
    });

    /**
     * cass_cluster_set_tcp_keepalive and cass_cluster_set_connection_heartbeat_interval
     * take seconds, every other timing setter takes milliseconds. Both groups are
     * asserted here so a shared helper cannot silently convert one of them again.
     */
    describe('timing units', function () {

        it('keeps TCP keepalive and heartbeat in seconds', function () {
            $props = builderProps([
                'cassandra.tcp_keepalive_delay'           => 15,
                'cassandra.connection_heartbeat_interval' => 17,
            ]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['tcpKeepalive'])->toBe(15.0)
                ->and($props['connectionHeartbeatInterval'])->toBe(17);
        });

        it('round-trips the seconds a with*() call was given', function () {
            $result = phpWithIni(
                'echo serialize((array) (new Cassandra\Cluster\Builder)'
                . '->withTCPKeepalive(60.0)->withConnectionHeartbeatInterval(20.0));',
            );

            if ($result === null || $result['exit'] !== 0) {
                $this->markTestSkipped('child process could not load the extension');
            }

            $props = @unserialize((string) $result['payload']);

            expect($props['tcpKeepalive'])->toBe(60.0)
                ->and($props['connectionHeartbeatInterval'])->toBe(20);
        });

        it('keeps the millisecond directives in milliseconds', function () {
            $props = builderProps([
                'cassandra.connect_timeout'    => 3000,
                'cassandra.request_timeout'    => 7000,
                'cassandra.reconnect_interval' => 4000,
            ]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            /* The properties view reports these in seconds. */
            expect($props['connectTimeout'])->toBe(3.0)
                ->and($props['requestTimeout'])->toBe(7.0)
                ->and($props['reconnectInterval'])->toBe(4.0);
        });
    });

    describe('ScyllaDB settings', function () {

        it('selects rack-aware routing when a local rack is set', function () {
            $props = builderProps([
                'cassandra.local_dc'   => 'datacenter1',
                'cassandra.local_rack' => 'rack1',
            ]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            /* 3 is LOAD_BALANCING_RACK_AWARE. */
            expect($props['loadBalancingPolicy'])->toBe(3)
                ->and($props['localDatacenter'])->toBe('datacenter1')
                ->and($props['localRack'])->toBe('rack1');
        });

        it('leaves the load balancing policy alone without a local rack', function () {
            $props = builderProps(['cassandra.local_dc' => 'datacenter1']);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['loadBalancingPolicy'])->toBe(0)
                ->and($props['localRack'])->toBeNull();
        });

        it('carries the application name and version', function () {
            $props = builderProps([
                'cassandra.application_name'    => 'checkout-api',
                'cassandra.application_version' => '2.7.1',
            ]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['applicationName'])->toBe('checkout-api')
                ->and($props['applicationVersion'])->toBe('2.7.1');
        });

        it('defaults to constant reconnect and no speculative execution', function () {
            $props = builderProps([]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['reconnectPolicy'])->toBe('constant')
                ->and($props['speculativeExecutionDelay'])->toBeNull()
                ->and($props['applicationName'])->toBeNull()
                ->and($props['localRack'])->toBeNull();
        });

        it('switches to exponential reconnect on request', function () {
            $props = builderProps([
                'cassandra.reconnect_policy'       => 'exponential',
                'cassandra.reconnect_max_interval' => 45000,
            ]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['reconnectPolicy'])->toBe('exponential')
                ->and($props['reconnectMaxInterval'])->toBe(45.0);
        });

        it('enables speculative execution only with a non-zero delay', function () {
            $props = builderProps([
                'cassandra.speculative_execution_delay' => 200,
                'cassandra.speculative_execution_max'   => 3,
            ]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['speculativeExecutionDelay'])->toBe(0.2)
                ->and($props['speculativeExecutionMax'])->toBe(3);
        });

        it('holds new_request_ratio to the driver range of 1 to 100', function () {
            $result = phpWithIni(
                'echo ini_get("cassandra.new_request_ratio");',
                ['cassandra.new_request_ratio' => 500],
            );

            if ($result === null || $result['exit'] !== 0) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect(trim((string) $result['payload']))->toBe('50')
                ->and($result['stderr'])->toContain('must be between 1 and 100');
        });
    });

    describe('cassandra.allow_persistent', function () {

        it('turns persistence off for a fresh builder', function () {
            $props = builderProps(['cassandra.allow_persistent' => 0]);

            if ($props === null) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect($props['usePersistentSessions'])->toBeFalse();
        });

        it('cannot be overridden by withPersistentSessions(true)', function () {
            $result = phpWithIni(
                'var_export(((array) (new Cassandra\Cluster\Builder)'
                . '->withPersistentSessions(true))["usePersistentSessions"]);',
                ['cassandra.allow_persistent' => 0],
            );

            if ($result === null || $result['exit'] !== 0) {
                $this->markTestSkipped('child process could not load the extension');
            }

            expect(trim($result['payload']))->toBe('false');
        });
    });
});
