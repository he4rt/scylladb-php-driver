<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra\Blob;

describe('Cassandra\Blob', function () {

    it('hex-encodes a string', function () {
        $blob = new Blob('Hi');
        expect($blob->__toString())->toEqual('0x4869')
            ->and($blob->bytes())->toEqual('0x4869');
    });

    it('returns the original bytes as a binary string', function () {
        $blob = new Blob('Hi');
        expect((string) $blob->toBinaryString())->toEqual('Hi');
    });

    it('compares two equal Blobs', function (Blob $value1, Blob $value2) {
        expect($value2)->toEqual($value1)
            ->and($value1 == $value2)->toBeTrue();
    })->with([
        [new Blob('0x1234'), new Blob('0x1234')],
    ]);

    it('compares two non-equal Blobs', function (Blob $value1, Blob $value2) {
        expect($value2)->not->toEqual($value1)
            ->and($value1 == $value2)->toBeFalse();
    })->with([
        [new Blob('0x1234'), new Blob('0x4567')],
    ]);
});
