<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra\Async {
    /**
     * A driver completion fd as a native {@see \Io\Poll\Handle} (PHP >= 8.6).
     *
     * The class exists only when the extension is built against a PHP that
     * provides the polling API. Guard with
     * `class_exists(\Cassandra\Async\PollHandle::class)`.
     *
     * Pass an instance straight to {@see \Io\Poll\Context::add()}, which takes
     * an {@see \Io\Poll\Handle} and nothing else. This handle registers the
     * driver's own eventfd/pipe: it does not duplicate the descriptor and does
     * not allocate a stream resource.
     *
     * One descriptor holds one watcher per context, so a context rejects a
     * second handle over the same future with
     * {@see \Io\Poll\HandleAlreadyWatchedException}.
     *
     * The handle is readable when work has completed. Read the result the same
     * way as with the stream API — {@see Reactor::poll()} for a reactor handle,
     * or `Future::get()` for a per-future handle.
     *
     * @strict-properties
     * @not-serializable
     * @scylladb-no-generate
     */
    final class PollHandle implements \Io\Poll\Handle
    {
        private function __construct() {}

        /**
         * The shared reactor's handle: one descriptor for every future
         * registered with {@see Reactor::add()}.
         *
         * Cached per request, so every call in one request returns the same
         * object — add it to a Context once.
         */
        public static function reactor(): PollHandle {}

        /**
         * A per-future handle, the poll-API counterpart of
         * {@see \Cassandra\Future::getResource()}.
         *
         * It becomes readable when this one future resolves. A future uses one
         * async model at a time: do not combine this with
         * {@see Reactor::add()}.
         */
        public static function forFuture(\Cassandra\Future $future): PollHandle {}

        /** The raw descriptor. Do not close it — the driver owns it. */
        public function getFileDescriptor(): int {}

        public function isValid(): bool {}
    }
}
