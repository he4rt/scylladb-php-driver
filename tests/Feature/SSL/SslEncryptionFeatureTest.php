<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\SSL;

use Cassandra;
use Cassandra\Exception\RuntimeException;

/**
 * Migrated from tests-old/features/ssl_encryption.feature.
 *
 * Every scenario assumes a Cassandra/ScyllaDB cluster brought up via CCM
 * with TLS encryption enabled (and, for the last scenario, client-cert
 * verification). The default CI fixture in this repo does NOT spin up such
 * a cluster, so all scenarios are skipped. The Pest skeleton is preserved
 * verbatim so they can be re-enabled when the SSL fixture is added.
 *
 * Original behat fixtures (paths relative to tests-old/support/):
 *   ssl/cassandra.pem  → server trust cert      (SERVER_CERT)
 *   ssl/driver.pem     → client cert            (CLIENT_CERT)
 *   ssl/driver.key     → client private key     (PRIVATE_KEY)
 *   passphrase         "php-driver"             (PASSPHRASE)
 *
 * To un-skip: bring up an SSL-encrypted ScyllaDB/Cassandra cluster, copy
 * the cert/key fixtures to a known location, and export the SCYLLADB_SSL_*
 * env vars (or wire equivalents into Pest.php).
 */

const SSL_FIXTURE_SERVER_CERT = __DIR__ . '/../../../tests-old/support/ssl/cassandra.pem';
const SSL_FIXTURE_CLIENT_CERT = __DIR__ . '/../../../tests-old/support/ssl/driver.pem';
const SSL_FIXTURE_PRIVATE_KEY = __DIR__ . '/../../../tests-old/support/ssl/driver.key';
const SSL_FIXTURE_PASSPHRASE  = 'php-driver';

it('fails to connect to an SSL-encrypted cluster without SSL options', function () {
    test()->markTestSkipped('requires SSL-configured cluster');

    $cluster = Cassandra::cluster()
        ->withContactPoints('127.0.0.1')
        ->build();

    $caught = false;
    try {
        $cluster->connect();
    } catch (RuntimeException $e) {
        $caught = true;
    }

    expect($caught)->toBeTrue();
});

it('connects with basic SSL encryption (VERIFY_NONE)', function () {
    test()->markTestSkipped('requires SSL-configured cluster');

    $ssl = Cassandra::ssl()
        ->withVerifyFlags(Cassandra::VERIFY_NONE)
        ->build();

    $cluster = Cassandra::cluster()
        ->withContactPoints('127.0.0.1')
        ->withSSL($ssl)
        ->build();

    $session = $cluster->connect();
    expect($session)->not->toBeNull();
});

it('connects with server certificate verification (VERIFY_PEER_CERT)', function () {
    test()->markTestSkipped('requires SSL-configured cluster + server cert fixture');

    $ssl = Cassandra::ssl()
        ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT)
        ->withTrustedCerts(SSL_FIXTURE_SERVER_CERT)
        ->build();

    $cluster = Cassandra::cluster()
        ->withContactPoints('127.0.0.1')
        ->withSSL($ssl)
        ->build();

    $session = $cluster->connect();
    expect($session)->not->toBeNull();
});

it('connects with client certificate verification', function () {
    test()->markTestSkipped('requires SSL+client-cert-verification cluster + cert/key fixtures');

    $ssl = Cassandra::ssl()
        ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT)
        ->withTrustedCerts(SSL_FIXTURE_SERVER_CERT)
        ->withClientCert(SSL_FIXTURE_CLIENT_CERT)
        ->withPrivateKey(SSL_FIXTURE_PRIVATE_KEY, SSL_FIXTURE_PASSPHRASE)
        ->build();

    $cluster = Cassandra::cluster()
        ->withContactPoints('127.0.0.1')
        ->withSSL($ssl)
        ->build();

    $session = $cluster->connect();
    expect($session)->not->toBeNull();
});
