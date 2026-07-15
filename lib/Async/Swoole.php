<?php

declare(strict_types=1);

namespace Cassandra\Async;

use Cassandra\Future;

/**
 * Swoole / OpenSwoole adapter: await a ScyllaDB {@see Future} inside a coroutine
 * without blocking the reactor.
 *
 * The driver's notification fd (see {@see Future::getResource()}) is watched with
 * (Open)Swoole's Coroutine\System::waitEvent(), which suspends only the current
 * coroutine until the fd becomes readable; the scheduler keeps running every
 * other coroutine meanwhile. When the fd signals, get() returns the result
 * without blocking (or rethrows the driver exception).
 *
 * Works with both ext-swoole (`Swoole\` namespace) and ext-openswoole
 * (`OpenSwoole\` namespace) — the runtime is detected automatically. Must run
 * inside a coroutine (e.g. Swoole\Coroutine\run() / OpenSwoole\Coroutine\run()).
 *
 * If the extension was compiled with native support
 * (-DPHP_SCYLLADB_ENABLE_SWOOLE / _OPENSWOOLE), Future::get() is already
 * coroutine-aware and this adapter is equivalent to calling $future->get()
 * directly (see docs/async.md).
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
        [$coroutine, $system, $eventRead] = self::runtime();

        if ($coroutine === null) {
            throw new \RuntimeException(
                'Cassandra\Async\Swoole::await() requires the swoole or openswoole extension.',
            );
        }

        if ($coroutine::getCid() < 0) {
            throw new \RuntimeException(
                'Cassandra\Async\Swoole::await() must be called inside a coroutine.',
            );
        }

        if ($future->isReady()) {
            return $future->get($timeout);
        }

        $resource = $future->getResource();

        // Suspends only this coroutine until the fd is readable.
        $system::waitEvent($resource, $eventRead, $timeout ?? -1);

        return $future->get($timeout);
    }

    /**
     * Resolve the active coroutine runtime: [Coroutine class, System class,
     * EVENT_READ value], or [null, null, 0] if neither extension is loaded.
     *
     * @return array{0: class-string|null, 1: class-string|null, 2: int}
     */
    private static function runtime(): array
    {
        if (class_exists('\Swoole\Coroutine\System')) {
            return [
                '\Swoole\Coroutine',
                '\Swoole\Coroutine\System',
                \defined('SWOOLE_EVENT_READ') ? \SWOOLE_EVENT_READ : 512,
            ];
        }

        if (class_exists('\OpenSwoole\Coroutine\System')) {
            return [
                '\OpenSwoole\Coroutine',
                '\OpenSwoole\Coroutine\System',
                \defined('OPENSWOOLE_EVENT_READ') ? \OPENSWOOLE_EVENT_READ
                    : (\defined('SWOOLE_EVENT_READ') ? \SWOOLE_EVENT_READ : 512),
            ];
        }

        return [null, null, 0];
    }
}
