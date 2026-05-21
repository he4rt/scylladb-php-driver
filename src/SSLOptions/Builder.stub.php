<?php

/**
* @generate-class-entries
*/
namespace Cassandra\SSLOptions {
    /**
     * @strict-properties
     */
     final class Builder {
        public function withTrustedCerts(string ...$path): static {}
        public function withVerifyFlags(int $flags): static {}
        public function withClientCert(string $path): static {}
        public function withPrivateKey(string $path, ?string $passphrase = null): static {}

        public function build(): \Cassandra\SSLOptions {}
     }
}