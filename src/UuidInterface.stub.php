<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    interface UuidInterface extends Value {
        public function uuid(): string;
        public function version(): int;
    }
}
