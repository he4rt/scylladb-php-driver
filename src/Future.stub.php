<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    interface Future {
        public function get(int|float|null $timeout = null): mixed;

        /**
         * A readable stream resource that becomes readable exactly once, when
         * this future resolves. Register it with any event loop
         * (stream_select, ReactPHP addReadStream, AMPHP/Revolt onReadable,
         * Swoole Event::add); on the readable event call {@see Future::get()},
         * which then returns without blocking. The stream is one-shot — remove
         * the watcher after the first readable event.
         *
         * @return resource
         */
        public function getResource(): mixed;

        /** Whether the future has resolved (get() will not block). */
        public function isReady(): bool;
    }
}
