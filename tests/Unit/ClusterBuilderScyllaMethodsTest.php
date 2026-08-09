<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;

/**
 * Builder methods for the ScyllaDB-specific and newly exposed cluster settings.
 * These run in-process: none of them needs a php.ini override, which is the
 * point of having a method as well as a directive.
 */
describe('Cassandra\Cluster\Builder ScyllaDB methods', function () {

    it('selects rack-aware routing', function () {
        $props = (array) Cassandra::cluster()
            ->withRackAwareLoadBalancingPolicy('datacenter1', 'rack1');

        /* 3 is LOAD_BALANCING_RACK_AWARE. */
        expect($props['loadBalancingPolicy'])->toBe(3)
            ->and($props['localDatacenter'])->toBe('datacenter1')
            ->and($props['localRack'])->toBe('rack1');
    });

    it('lets the driver infer the datacenter and rack when both are empty', function () {
        $props = (array) Cassandra::cluster()->withRackAwareLoadBalancingPolicy();

        expect($props['loadBalancingPolicy'])->toBe(3)
            ->and($props['localDatacenter'])->toBeNull()
            ->and($props['localRack'])->toBeNull();
    });

    it('overrides the php.ini rack seed', function () {
        $props = (array) Cassandra::cluster()
            ->withRackAwareLoadBalancingPolicy('dc-a', 'rack-a')
            ->withRackAwareLoadBalancingPolicy('dc-b', 'rack-b');

        expect($props['localDatacenter'])->toBe('dc-b')
            ->and($props['localRack'])->toBe('rack-b');
    });

    it('carries the application name and version', function () {
        $props = (array) Cassandra::cluster()
            ->withApplicationName('checkout-api')
            ->withApplicationVersion('2.7.1');

        expect($props['applicationName'])->toBe('checkout-api')
            ->and($props['applicationVersion'])->toBe('2.7.1');
    });

    it('switches to exponential reconnect and keeps seconds', function () {
        $props = (array) Cassandra::cluster()->withExponentialReconnect(1.5, 45.0);

        expect($props['reconnectPolicy'])->toBe('exponential')
            ->and($props['reconnectInterval'])->toBe(1.5)
            ->and($props['reconnectMaxInterval'])->toBe(45.0);
    });

    it('enables and disables speculative execution', function () {
        $on = (array) Cassandra::cluster()->withConstantSpeculativeExecutionPolicy(0.2, 3);

        expect($on['speculativeExecutionDelay'])->toBe(0.2)
            ->and($on['speculativeExecutionMax'])->toBe(3);

        $off = (array) Cassandra::cluster()
            ->withConstantSpeculativeExecutionPolicy(0.2, 3)
            ->withNoSpeculativeExecutionPolicy();

        expect($off['speculativeExecutionDelay'])->toBeNull();
    });

    it('defaults speculative executions to 2', function () {
        $props = (array) Cassandra::cluster()->withConstantSpeculativeExecutionPolicy(0.5);

        expect($props['speculativeExecutionMax'])->toBe(2);
    });

    it('rejects a new request ratio outside 1 to 100', function () {
        expect(fn () => Cassandra::cluster()->withNewRequestRatio(500))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);

        expect(fn () => Cassandra::cluster()->withNewRequestRatio(0))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
    });

    it('rejects a max reconnect interval below the base', function () {
        expect(fn () => Cassandra::cluster()->withExponentialReconnect(5.0, 1.0))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
    });

    it('rejects a non-positive speculative delay and coalesce delay', function () {
        expect(fn () => Cassandra::cluster()->withConstantSpeculativeExecutionPolicy(-1.0))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);

        expect(fn () => Cassandra::cluster()->withCoalesceDelay(0))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
    });

    it('returns the same builder so calls chain', function () {
        $builder = Cassandra::cluster();

        expect($builder->withApplicationName('a'))->toBe($builder)
            ->and($builder->withApplicationVersion('1'))->toBe($builder)
            ->and($builder->withRackAwareLoadBalancingPolicy('d', 'r'))->toBe($builder)
            ->and($builder->withExponentialReconnect(1.0, 2.0))->toBe($builder)
            ->and($builder->withConstantSpeculativeExecutionPolicy(0.1))->toBe($builder)
            ->and($builder->withNoSpeculativeExecutionPolicy())->toBe($builder)
            ->and($builder->withCoalesceDelay(100))->toBe($builder)
            ->and($builder->withNewRequestRatio(50))->toBe($builder);
    });

    it('builds a cluster with every new setting applied', function () {
        $cluster = Cassandra::cluster()
            ->withRackAwareLoadBalancingPolicy('datacenter1', 'rack1')
            ->withApplicationName('checkout-api')
            ->withApplicationVersion('2.7.1')
            ->withExponentialReconnect(1.0, 30.0)
            ->withConstantSpeculativeExecutionPolicy(0.2, 2)
            ->withCoalesceDelay(400)
            ->withNewRequestRatio(75)
            ->withPersistentSessions(false)
            ->build();

        expect($cluster)->toBeInstanceOf(Cassandra\Cluster::class);
    });
});
