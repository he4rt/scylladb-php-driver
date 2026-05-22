<?php

/**
* @generate-class-entries
*/

declare(strict_types=1);

namespace Cassandra\SSLOptions {
    /**
     * @strict-properties
 * @scylladb-struct php_scylladb_ssl_builder
 */     final class Builder {
        public function withTrustedCerts(string ...$path): static {}
        public function withVerifyFlags(int $flags): static {}
        public function withClientCert(string $path): static {}
        public function withPrivateKey(string $path, ?string $passphrase = null): static {}

        public function build(): \Cassandra\SSLOptions {}
     }
}