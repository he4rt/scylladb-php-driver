<?php

/**
 * @generate-class-entries
 */
namespace Cassandra {
    interface UuidInterface extends Value {
        public function uuid(): string;
        public function version(): int;
    }
}
