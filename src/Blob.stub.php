<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * @strict-properties
     * @scylladb-value-handlers
     * @scylladb-struct php_scylladb_blob
     * @deprecated
     */
    #[\Deprecated(message: 'Use a PHP string.', since: '1.6.0')]
    final class Blob implements Value {
        public function __construct(string $bytes) {}

        public function type(): Type {}
        public function bytes(): string {}
        public function toBinaryString(): string {}

        public function __toString(): string {}
    }
}
