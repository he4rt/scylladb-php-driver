<?php

declare(strict_types=1);

namespace Cassandra\Async;

use Cassandra\Future;
use Revolt\EventLoop;

/**
 * Framework-agnostic fiber-based await built on Revolt's event loop.
 *
 * Revolt is the standard event-loop abstraction shared by AMPHP v3 and
 * interoperable with other fiber schedulers, which makes {@see Revolt::await()}
 * the ergonomic default for awaiting a ScyllaDB {@see Future} without blocking
 * the loop: it suspends the current fiber until the driver resolves the future
 * on its IO thread, then returns the result (or rethrows the driver exception).
 *
 * Requires revolt/event-loop and a running EventLoop (e.g. inside
 * EventLoop::run() or an AMPHP context). Install with:
 *   composer require revolt/event-loop
 *
 * Example:
 *   Revolt::await($session->executeAsync('SELECT * FROM t'));
 */
final class Revolt
{
    /**
     * Suspend the current fiber until $future resolves and return its result.
     *
     * @param  Future           $future  any Cassandra future (FutureRows, FutureSession, ...)
     * @param  int|float|null   $timeout seconds to wait for the driver result once
     *                                   the fd signals; null waits indefinitely.
     * @return mixed the resolved value (Rows, Session, PreparedStatement, ...)
     */
    public static function await(Future $future, int|float|null $timeout = null): mixed
    {
        // Already resolved — no need to touch the loop or allocate an fd.
        if ($future->isReady()) {
            return $future->get($timeout);
        }

        $resource   = $future->getResource();
        $suspension = EventLoop::getSuspension();

        // One-shot readable watcher: cancel on the first wakeup, then resume.
        $watcher = EventLoop::onReadable(
            $resource,
            static function (string $watcher) use ($suspension): void {
                EventLoop::cancel($watcher);
                $suspension->resume();
            },
        );

        try {
            $suspension->suspend();
        } finally {
            // Defensive: ensure the watcher is gone even on cancellation.
            EventLoop::cancel($watcher);
        }

        return $future->get($timeout);
    }

    /**
     * Await several futures concurrently, returning results keyed the same as
     * the input. Each future is awaited in its own fiber so they overlap.
     *
     * @param  iterable<array-key, Future> $futures
     * @return array<array-key, mixed>
     */
    public static function awaitAll(iterable $futures, int|float|null $timeout = null): array
    {
        $suspensions = [];
        $results     = [];
        $errors      = [];
        $pending     = 0;
        $parent      = EventLoop::getSuspension();

        foreach ($futures as $key => $future) {
            $pending++;
            EventLoop::queue(static function () use ($future, $key, $timeout, &$results, &$errors, &$pending, $parent): void {
                try {
                    $results[$key] = self::await($future, $timeout);
                } catch (\Throwable $e) {
                    $errors[$key] = $e;
                } finally {
                    if (--$pending === 0) {
                        $parent->resume();
                    }
                }
            });
        }

        if ($pending === 0) {
            return [];
        }

        $parent->suspend();

        if ($errors !== []) {
            throw \array_values($errors)[0];
        }

        return $results;
    }
}
