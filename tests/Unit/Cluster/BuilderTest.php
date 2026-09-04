<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Cluster;

use Cassandra;

describe('Cassandra\Cluster\Builder', function () {

    it('defaults connectionHeartbeatInterval to 30 seconds', function () {
        $props = (array) Cassandra::cluster();

        expect($props['connectionHeartbeatInterval'])->toBe(30);
    });

    it('defaults tcpKeepalive to null', function () {
        $props = (array) Cassandra::cluster();

        expect($props['tcpKeepalive'])->toBeNull();
    });

    it('round-trips connectionHeartbeatInterval in seconds', function (float $interval, int $expected) {
        $props = (array) Cassandra::cluster()->withConnectionHeartbeatInterval($interval);

        expect($props['connectionHeartbeatInterval'])->toBe($expected);
    })->with([
        [0.0, 0],
        [5.0, 5],
        [30.0, 30],
        [3600.0, 3600],
    ]);

    it('round-trips tcpKeepalive in seconds', function (float $delay, float $expected) {
        $props = (array) Cassandra::cluster()->withTCPKeepalive($delay);

        expect($props['tcpKeepalive'])->toBe($expected);
    })->with([
        [0.0, 0.0],
        [60.0, 60.0],
        [1800.0, 1800.0],
    ]);

    it('disables tcpKeepalive when given null', function () {
        $props = (array) Cassandra::cluster()->withTCPKeepalive(null);

        expect($props['tcpKeepalive'])->toBeNull();
    });

    it('round-trips reconnectInterval in seconds', function () {
        $props = (array) Cassandra::cluster()->withReconnectInterval(2.5);

        expect($props['reconnectInterval'])->toBe(2.5);
    });

    it('rejects a negative connection heartbeat interval', function () {
        Cassandra::cluster()->withConnectionHeartbeatInterval(-1.0);
    })->throws(\Cassandra\Exception\InvalidArgumentException::class);

    it('rejects a negative tcp keepalive delay', function () {
        Cassandra::cluster()->withTCPKeepalive(-1.0);
    })->throws(\Cassandra\Exception\InvalidArgumentException::class);
});
