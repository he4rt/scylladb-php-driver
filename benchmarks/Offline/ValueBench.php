<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Offline;

use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Smallint;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Tinyint;
use Cassandra\Uuid;
use Cassandra\Varint;
use PhpBench\Attributes as Bench;

/**
 * Scalar-value marshalling: PHP -> extension object -> canonical string.
 *
 * These are the per-value costs paid on every bound parameter and every
 * decoded column. They touch no network, so they run in the `offline` group
 * with high revolution counts for tight confidence intervals. Watch the
 * `mem_peak` column as closely as time — value objects that leak or over-
 * allocate show up here before they ever reach a query.
 */
#[Bench\BeforeMethods('setUp')]
#[Bench\Groups(['offline', 'value'])]
#[Bench\Revs(10000)]
#[Bench\Iterations(5)]
#[Bench\Warmup(2)]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class ValueBench
{
    private string $uuidString = '756716f7-2e54-4715-9f00-91dcbea6cf50';
    private string $bigNumber = '9223372036854775807';
    private string $decimal = '3.14159265358979323846';
    private string $ipv6 = '2001:0db8:85a3:0000:0000:8a2e:0370:7334';

    public function setUp(): void
    {
        // Nothing to warm; kept so every subject shares one before-hook shape.
    }

    public function benchUuidGenerate(): void
    {
        new Uuid();
    }

    public function benchUuidFromString(): void
    {
        new Uuid($this->uuidString);
    }

    public function benchUuidRoundTrip(): void
    {
        (new Uuid($this->uuidString))->uuid();
    }

    public function benchTimeuuidGenerate(): void
    {
        new Timeuuid();
    }

    public function benchBigint(): void
    {
        (new Bigint($this->bigNumber))->value();
    }

    public function benchSmallint(): void
    {
        (new Smallint(32767))->value();
    }

    public function benchTinyint(): void
    {
        (new Tinyint(127))->value();
    }

    public function benchVarint(): void
    {
        (new Varint($this->bigNumber))->value();
    }

    public function benchDecimal(): void
    {
        (new Decimal($this->decimal))->value();
    }

    public function benchDecimalArithmetic(): void
    {
        $a = new Decimal($this->decimal);
        $a->add($a)->__toString();
    }

    public function benchTimestamp(): void
    {
        (new Timestamp(1_700_000_000, 500_000))->__toString();
    }

    public function benchDuration(): void
    {
        (new Duration(1, 2, 3_000_000_000))->__toString();
    }

    public function benchInet(): void
    {
        (new Inet($this->ipv6))->address();
    }

    public function benchBlobSmall(): void
    {
        (new Blob('scylladb-php-driver'))->bytes();
    }
}
