<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;
use Cassandra\ProtocolVersion;

describe('Cassandra\ProtocolVersion', function () {

    it('is an int-backed enum whose cases match the protocol version byte', function () {
        expect(ProtocolVersion::V1->value)->toBe(1);
        expect(ProtocolVersion::V2->value)->toBe(2);
        expect(ProtocolVersion::V3->value)->toBe(3);
        expect(ProtocolVersion::V4->value)->toBe(4);
        expect(ProtocolVersion::V5->value)->toBe(5);
    });

    it('exposes exactly the five native protocol versions', function () {
        expect(array_column(ProtocolVersion::cases(), 'name'))
            ->toBe(['V1', 'V2', 'V3', 'V4', 'V5']);
    });

    it('resolves a case from its backing value', function () {
        expect(ProtocolVersion::from(4))->toBe(ProtocolVersion::V4);
        expect(ProtocolVersion::tryFrom(9))->toBeNull();
    });
});

describe('Cassandra\Cluster\Builder protocol version', function () {

    it('defaults to v4', function () {
        $props = (array) Cassandra::cluster();

        expect($props['protocolVersion'])->toBe(ProtocolVersion::V4);
    });

    it('stores the version passed as an enum case', function () {
        $props = (array) Cassandra::cluster()->withProtocolVersion(ProtocolVersion::V5);

        expect($props['protocolVersion'])->toBe(ProtocolVersion::V5);
    });

    it('returns the builder so calls chain', function () {
        $builder = Cassandra::cluster();

        expect($builder->withProtocolVersion(ProtocolVersion::V3))->toBe($builder);
    });

    it('still accepts a plain integer', function () {
        $props = (array) Cassandra::cluster()->withProtocolVersion(3);

        expect($props['protocolVersion'])->toBe(ProtocolVersion::V3);
    });

    it('keeps an integer the enum does not name', function () {
        $props = (array) Cassandra::cluster()->withProtocolVersion(0x41);

        expect($props['protocolVersion'])->toBe(0x41);
    });

    it('rejects a non-positive integer', function () {
        Cassandra::cluster()->withProtocolVersion(0);
    })->throws(\Cassandra\Exception\InvalidArgumentException::class);

    it('rejects a value that is neither an int nor a ProtocolVersion', function () {
        Cassandra::cluster()->withProtocolVersion('v4');
    })->throws(\TypeError::class);
});
