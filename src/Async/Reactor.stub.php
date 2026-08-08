<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Async {
    /**
     * Shared completion reactor: watch ONE stream for many in-flight futures.
     *
     * Opt-in high-concurrency alternative to per-future {@see \Cassandra\Future::getResource()}.
     * Register futures with add(), watch resource() in your event loop, and on
     * the readable event call poll() to get the completed futures to dispatch.
     * fd usage stays O(1) regardless of how many futures are outstanding.
     *
     * @strict-properties
     * @not-serializable
     */
    final class Reactor {
        /**
         * The single readable stream resource to register with your event loop.
         * It becomes readable whenever one or more registered futures complete.
         *
         * @return resource
         */
        public static function resource(): mixed {}

        /**
         * Register a future for completion notification via this reactor. Every
         * concrete future works: FutureRows, FutureSession, FuturePreparedStatement,
         * FutureClose and FutureValue. A future may use either the reactor or a
         * per-future descriptor (getResource / PollHandle::forFuture), not both.
         *
         * A FutureValue resolves on construction, so poll() reports it on the
         * next call.
         *
         * With $onComplete, poll() calls it with the resolved future instead of
         * returning that future. Without it, poll() returns the future. Mix both
         * styles freely — the choice is per future.
         *
         * The callback runs on the PHP thread inside poll(), never on a driver
         * IO thread, so it may touch anything a normal PHP callable may. Its
         * one argument is the resolved future.
         */
        public static function add(\Cassandra\Future $future, ?callable $onComplete = null): void {}

        /**
         * Drain completions. Never waits: it returns at once with whatever has
         * resolved since the last call.
         *
         * A future registered with a callback has that callback called here. A
         * future registered without one is returned, ready — get() does no
         * network wait.
         *
         * get() still decodes the result set on the calling thread, so an
         * unbounded batch makes one event-loop tick as long as the sum of every
         * decode. Pass $max to cap the batch: the rest stays queued and the
         * stream stays readable, so the loop takes the remainder on the next
         * tick and other watchers run in between.
         *
         * A callback that throws stops the batch the same way and the exception
         * propagates out of poll(). Nothing is lost: the futures this call had
         * already collected go back to the front of the queue, so the next poll()
         * returns them before the rest.
         *
         * $max is the number of completions to take, or null for every one.
         *
         * @return list<\Cassandra\Future>
         */
        public static function poll(?int $max = null): array {}

        /** Number of registered futures not yet returned by poll(). */
        public static function pending(): int {}
    }
}
