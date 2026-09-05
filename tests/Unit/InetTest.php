<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra\Exception\InvalidArgumentException;
use Cassandra\Inet;
use Cassandra\Type;

describe('Cassandra\Inet', function () {

    it('round-trips an IPv4 address', function () {
        expect((string) new Inet('127.0.0.1'))->toBe('127.0.0.1')
            ->and((string) new Inet('0.0.0.0'))->toBe('0.0.0.0')
            ->and((string) new Inet('255.255.255.255'))->toBe('255.255.255.255');
    });

    it('expands an IPv6 address to its uncompressed form', function ($address, $expected) {
        expect((string) new Inet($address))->toBe($expected);
    })->with([
        ['::1', '0:0:0:0:0:0:0:1'],
        ['::', '0:0:0:0:0:0:0:0'],
        ['2001:db8::1', '2001:db8:0:0:0:0:0:1'],
        ['2001:0db8:0000:0000:0000:0000:0000:0001', '2001:db8:0:0:0:0:0:1'],
    ]);

    it('accepts an IPv4-mapped IPv6 address', function () {
        expect((string) new Inet('::ffff:127.0.0.1'))->toBe('0:0:0:0:0:ffff:7f00:1');
    });

    it('exposes the CQL inet type', function () {
        expect((new Inet('127.0.0.1'))->type())->toEqual(Type::inet());
    });

    it('compares two addresses by value', function () {
        expect(new Inet('127.0.0.1') == new Inet('127.0.0.1'))->toBeTrue()
            ->and(new Inet('127.0.0.1') == new Inet('127.0.0.2'))->toBeFalse();
    });

    it('rejects a malformed address', function (string $address) {
        new Inet($address);
    })->with([
        '127.0.0.1x',
        '999.1.1.1',
        '::1::2',
        '1:2:3:4:5:6:7:8:9',
        '1:2:3',
        'not-an-address',
        '',
        'fe80::1%0',
    ])->throws(InvalidArgumentException::class);

    it('rejects an address that stops at an embedded NUL byte', function () {
        expect(fn () => new Inet("127.0.0.1\0garbage"))
            ->toThrow(InvalidArgumentException::class, 'The IP address of 17 bytes contains a NUL byte at offset 9');

        expect(fn () => new Inet("::1\0x"))
            ->toThrow(InvalidArgumentException::class, 'The IP address of 5 bytes contains a NUL byte at offset 3');
    });

    it('names the address, not the parsed bytes, when too many bytes are seen', function () {
        expect(fn () => new Inet('1:2:3:4:5:6:7:8:9'))
            ->toThrow(InvalidArgumentException::class, 'Address "1:2:3:4:5:6:7:8:9" exceeds the maximum IPv6 byte length');
    });

    it('keeps every rejection message free of a trailing newline', function (string $address) {
        try {
            new Inet($address);
            $this->fail('expected a rejection of ' . $address);
        } catch (InvalidArgumentException $e) {
            expect($e->getMessage())->not->toEndWith("\n");
        }
    })->with(['1:2:3:4:5:6:7:8:9', '1:2:3', '::1::2', '999.1.1.1', '127.0.0.1x']);

    it('rejects an address longer than the parser buffer', function () {
        new Inet(str_repeat('1', 60));
    })->throws(InvalidArgumentException::class, 'is too long');
});
