<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Support;

/**
 * Shared PhpBench parameter providers.
 *
 * A param provider returns an iterable of named parameter sets; PhpBench runs
 * the subject once per set and reports each under its "set" label. Keeping
 * them here means every collection/blob/batch benchmark scales over the same
 * axis, so their reports line up.
 */
trait Sizes
{
    /** @return iterable<string, array{count: int}> */
    public function provideElementCounts(): iterable
    {
        yield '1'    => ['count' => 1];
        yield '10'   => ['count' => 10];
        yield '100'  => ['count' => 100];
        yield '1000' => ['count' => 1000];
    }

    /** @return iterable<string, array{bytes: int}> */
    public function provideByteSizes(): iterable
    {
        yield '64B'  => ['bytes' => 64];
        yield '1KiB' => ['bytes' => 1024];
        yield '64KiB' => ['bytes' => 64 * 1024];
        yield '1MiB' => ['bytes' => 1024 * 1024];
    }
}
