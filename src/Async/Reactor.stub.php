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
         * Register a future (currently a FutureRows from executeAsync) for
         * completion notification via this reactor. A future may use either the
         * reactor or getResource(), not both.
         */
        public static function add(\Cassandra\Future $future): void {}

        /**
         * Drain completions: returns the futures that have resolved since the
         * last call (each is ready — call get() on it without blocking).
         *
         * @return list<\Cassandra\Future>
         */
        public static function poll(): array {}

        /** Number of registered futures not yet returned by poll(). */
        public static function pending(): int {}
    }
}
