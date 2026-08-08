<?php

declare(strict_types=1);

namespace Cassandra\Async;

use Cassandra\Future;
use Revolt\EventLoop;
use Revolt\EventLoop\Suspension;

/**
 * Revolt adapter for the shared {@see Reactor} — O(1) fds for high fan-out.
 *
 * Unlike {@see Revolt} (one fd per future), this watches the reactor's single
 * shared stream once and dispatches every completion from it, so the event loop
 * holds one watcher no matter how many queries are in flight. Best for awaiting
 * large batches concurrently.
 *
 * Requires revolt/event-loop and a running EventLoop. Register futures only
 * through this adapter (it owns the reactor's dispatch); don't mix with
 * Reactor::poll() calls of your own.
 *
 * Example:
 *   EventLoop::queue(function () use ($session) {
 *       $rows = ReactorRevolt::awaitAll(array_map(
 *           fn ($cql) => $session->executeAsync($cql),
 *           $queries,
 *       ));
 *   });
 *   EventLoop::run();
 */
final class ReactorRevolt
{
    private static ?string $watcher = null;

    /** @var \SplObjectStorage<Future, Suspension>|null */
    private static ?\SplObjectStorage $waiters = null;

    /**
     * Suspend the current fiber until $future resolves (dispatched via the
     * shared reactor) and return its result.
     */
    public static function await(Future $future): mixed
    {
        if (self::$waiters === null) {
            self::$waiters = new \SplObjectStorage();
        }

        Reactor::add($future);

        $suspension = EventLoop::getSuspension();
        self::$waiters[$future] = $suspension;
        self::ensureWatcher();

        $suspension->suspend();

        return $future->get();
    }

    /**
     * Await many futures concurrently through the one shared watcher.
     *
     * @param  iterable<array-key, Future> $futures
     * @return array<array-key, mixed>
     */
    public static function awaitAll(iterable $futures): array
    {
        $results = [];
        $errors  = [];
        $pending = 0;
        $parent  = EventLoop::getSuspension();

        foreach ($futures as $key => $future) {
            $pending++;
            EventLoop::queue(static function () use ($future, $key, &$results, &$errors, &$pending, $parent): void {
                try {
                    $results[$key] = self::await($future);
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

    /** Register the single shared reactor stream with the loop (once). */
    private static function ensureWatcher(): void
    {
        if (self::$watcher !== null) {
            return;
        }

        self::$watcher = EventLoop::onReadable(
            Reactor::resource(),
            static function (): void {
                foreach (Reactor::poll() as $future) {
                    if (self::$waiters !== null && isset(self::$waiters[$future])) {
                        $suspension = self::$waiters[$future];
                        unset(self::$waiters[$future]);
                        $suspension->resume();
                    }
                }

                // No one waiting → stop watching; a later await() re-arms it.
                if ((self::$waiters === null || self::$waiters->count() === 0) && self::$watcher !== null) {
                    EventLoop::cancel(self::$watcher);
                    self::$watcher = null;
                }
            },
        );
    }
}
