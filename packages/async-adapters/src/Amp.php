<?php

declare(strict_types=1);

namespace Cassandra\Async;

use Amp\DeferredFuture;
use Amp\Future as AmpFuture;
use Cassandra\Future;
use Revolt\EventLoop;

/**
 * AMPHP v3 adapter: turn a ScyllaDB {@see Future} into an {@see AmpFuture} that
 * completes when the driver resolves the future, without blocking the loop.
 *
 * AMPHP v3 runs on Revolt, so the driver's notification fd is registered via
 * Revolt\EventLoop::onReadable(); on the readable event the watcher is
 * cancelled and the returned Amp future is completed (or errored with the
 * driver exception thrown by get()). The result can be awaited with
 * Amp\Future::await() or combined with Amp\Future\{await,awaitAll,...}.
 *
 * Requires amphp/amp (^3) and revolt/event-loop. Install with:
 *   composer require amphp/amp
 *
 * Example:
 *   $rows = Amp::toFuture($session->executeAsync($cql))->await();
 */
final class Amp
{
    /**
     * @return AmpFuture<mixed>
     */
    public static function toFuture(Future $future): AmpFuture
    {
        $deferred = new DeferredFuture();

        $settle = static function () use ($future, $deferred): void {
            try {
                $deferred->complete($future->get());
            } catch (\Throwable $e) {
                $deferred->error($e);
            }
        };

        // Already resolved: complete on the next microtask, never synchronously.
        if ($future->isReady()) {
            EventLoop::queue($settle);

            return $deferred->getFuture();
        }

        $resource = $future->getResource();
        EventLoop::onReadable(
            $resource,
            static function (string $watcher) use ($settle): void {
                EventLoop::cancel($watcher);
                $settle();
            },
        );

        return $deferred->getFuture();
    }
}
