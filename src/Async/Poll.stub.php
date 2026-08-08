<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Async {
    /**
     * A ready-made event loop over PHP 8.6's {@see \Io\Poll\Context} — epoll,
     * kqueue or event ports, chosen for you. No framework, no adapter, no
     * stream_select().
     *
     * The class exists only when the extension is built against a PHP that
     * provides the polling API. Guard with `Poll::isSupported()`.
     *
     * It removes the two things you would otherwise write around a raw Context:
     * attaching the future to its watcher, and removing that watcher once the
     * future resolves. A driver descriptor stays readable after it fires, so a
     * watcher left in place makes the loop spin.
     *
     * One watcher per future. For thousands of queries in flight, watch the
     * shared {@see Reactor} instead — it folds every completion onto one
     * descriptor:
     *
     *     $ctx = new \Io\Poll\Context();
     *     $ctx->add(PollHandle::reactor(), [\Io\Poll\Event::Read]);
     *
     * Example:
     *     $loop = new Poll();
     *     foreach ($queries as $cql) {
     *         $loop->watch($session->executeAsync($cql), function ($future) {
     *             foreach ($future->get() as $row) { }
     *         });
     *     }
     *     $loop->run();
     *
     * @strict-properties
     * @not-serializable
     * @scylladb-struct php_scylladb_async_poll
     */
    final class Poll
    {
        public function __construct(?\Io\Poll\Backend $backend = null) {}

        /** Whether this build can use the polling API at all. */
        public static function isSupported(): bool {}

        /**
         * Watch one future. $onComplete is called with the resolved future;
         * without it, tick() returns the future instead.
         */
        public function watch(\Cassandra\Future $future, ?callable $onComplete = null): \Io\Poll\Watcher {}

        /** Futures watched but not yet resolved. */
        public function pending(): int {}

        /**
         * Wait for the next completions, dispatch them, and drop their
         * watchers. Returns the resolved futures that had no callback.
         *
         * $timeout is in seconds, or null to wait indefinitely.
         *
         * Callbacks run before the return array is built, so a callback that
         * throws loses nothing: the exception leaves tick() with every
         * callback-less future of that round still watched for the next call.
         *
         * @return list<\Cassandra\Future>
         */
        public function tick(?int $timeout = 5): array {}

        /**
         * Tick until nothing is watched. Use it when every future carries a
         * callback — a future without one is dropped, since nothing would read
         * it.
         */
        public function run(?int $timeout = 5): void {}

        /**
         * The underlying context, to add your own sockets, timers or pipes to
         * the same loop.
         */
        public function context(): \Io\Poll\Context {}
    }
}
