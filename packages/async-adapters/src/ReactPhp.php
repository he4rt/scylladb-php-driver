<?php

declare(strict_types=1);

namespace Cassandra\Async;

use Cassandra\Future;
use React\EventLoop\LoopInterface;
use React\Promise\Deferred;
use React\Promise\PromiseInterface;

/**
 * ReactPHP adapter: turn a ScyllaDB {@see Future} into a React promise that
 * settles when the driver resolves the future, without blocking the reactor.
 *
 * The driver's notification fd (see {@see Future::getResource()}) is registered
 * with the loop via addReadStream(); on the readable event the watcher is
 * removed and the promise is resolved with the result (or rejected with the
 * driver exception thrown by get()).
 *
 * Requires react/event-loop and react/promise. Install with:
 *   composer require react/event-loop react/promise
 *
 * Example:
 *   ReactPhp::toPromise($session->executeAsync($cql), $loop)
 *       ->then(fn($rows) => ...);
 */
final class ReactPhp
{
    /**
     * @return PromiseInterface<mixed>
     */
    public static function toPromise(Future $future, LoopInterface $loop): PromiseInterface
    {
        $deferred = new Deferred();

        $settle = static function () use ($future, $deferred): void {
            try {
                $deferred->resolve($future->get());
            } catch (\Throwable $e) {
                $deferred->reject($e);
            }
        };

        // Already resolved: settle on the next tick to preserve async semantics
        // (never resolve synchronously from within toPromise()).
        if ($future->isReady()) {
            $loop->futureTick($settle);

            return $deferred->promise();
        }

        $resource = $future->getResource();
        $loop->addReadStream(
            $resource,
            static function ($stream) use ($loop, $settle): void {
                $loop->removeReadStream($stream);
                $settle();
            },
        );

        return $deferred->promise();
    }
}
