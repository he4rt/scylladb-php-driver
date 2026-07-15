<?php

declare(strict_types=1);

namespace Cassandra\Async;

use Cassandra\Future;
use Swoole\Coroutine;
use Swoole\Coroutine\System;

/**
 * Swoole adapter: await a ScyllaDB {@see Future} inside a Swoole coroutine
 * without blocking the reactor.
 *
 * The driver's notification fd (see {@see Future::getResource()}) is watched
 * with Swoole\Coroutine\System::waitEvent(), which suspends only the current
 * coroutine until the fd becomes readable; the scheduler keeps running every
 * other coroutine meanwhile. When the fd signals, get() returns the result
 * without blocking (or rethrows the driver exception).
 *
 * Requires ext-swoole (or ext-openswoole) and must run inside a coroutine
 * (e.g. Swoole\Coroutine\run() / go()).
 *
 * Example:
 *   Coroutine\run(function () use ($session) {
 *       $rows = Swoole::await($session->executeAsync($cql));
 *   });
 *
 * This userland adapter works with any build of the extension. If the extension
 * was compiled with native Swoole support (-DPHP_SCYLLADB_ENABLE_SWOOLE), then
 * Future::get() is already coroutine-aware and this adapter is equivalent to
 * calling $future->get() directly (see docs/async.md).
 */
final class Swoole
{
    /**
     * Suspend the current coroutine until $future resolves and return its result.
     *
     * @param  int|float|null $timeout seconds to wait for the fd to signal
     *                                  (-1 / null waits indefinitely).
     * @return mixed the resolved value
     */
    public static function await(Future $future, int|float|null $timeout = null): mixed
    {
        if (!\extension_loaded('swoole') && !\extension_loaded('openswoole')) {
            throw new \RuntimeException(
                'Cassandra\Async\Swoole::await() requires the swoole (or openswoole) extension.',
            );
        }

        if (Coroutine::getCid() < 0) {
            throw new \RuntimeException(
                'Cassandra\Async\Swoole::await() must be called inside a Swoole coroutine.',
            );
        }

        if ($future->isReady()) {
            return $future->get($timeout);
        }

        $resource = $future->getResource();

        // SWOOLE_EVENT_READ = 1. Suspends only this coroutine until readable.
        System::waitEvent($resource, 1, $timeout ?? -1);

        return $future->get($timeout);
    }
}
