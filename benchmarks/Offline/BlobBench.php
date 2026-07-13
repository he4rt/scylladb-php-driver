<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Offline;

use Cassandra\Blob;
use Cassandra\Benchmarks\Support\Sizes;
use PhpBench\Attributes as Bench;

/**
 * Blob construction and readback across payload sizes.
 *
 * A Blob copies its bytes on the way in and again on `bytes()` on the way out.
 * This is the clearest place to catch an accidental extra copy or an O(n^2)
 * append: the per-byte cost (mode / bytes) should be flat across the size
 * axis, and mem_peak should track the payload, not multiply it.
 */
#[Bench\Groups(['offline', 'blob'])]
#[Bench\Iterations(5)]
#[Bench\Warmup(2)]
#[Bench\ParamProviders('provideByteSizes')]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class BlobBench
{
    use Sizes;

    private array $payloads = [];

    public function setUp(array $params): void
    {
        $this->payloads[$params['bytes']] ??= random_bytes($params['bytes']);
    }

    #[Bench\Revs(300)]
    #[Bench\BeforeMethods('setUp')]
    public function benchConstruct(array $params): void
    {
        new Blob($this->payloads[$params['bytes']]);
    }

    #[Bench\Revs(300)]
    #[Bench\BeforeMethods('setUp')]
    public function benchRoundTrip(array $params): void
    {
        (new Blob($this->payloads[$params['bytes']]))->bytes();
    }
}
