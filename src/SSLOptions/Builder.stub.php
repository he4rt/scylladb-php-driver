<?php

/**
* @generate-class-entries
*/
namespace Cassandra\SSLOptions {
    /**
     * @strict-properties
     */
     final class Builder {
        public function withTrustedCerts(string... $path): Builder {}
        public function withVerifyFlags(int $flags): Builder {}
        public function withClientCert(string $path): Builder {}
        public function withPrivateKey(string $path, string $passphrase): Builder {}

        public function build(): Cassandra\SSLOptions {}
     }
}