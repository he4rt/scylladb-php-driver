# TLS and SSL

TLS is configured with a second builder. `Cassandra::ssl()` returns a
`Cassandra\SSLOptions\Builder`. Its `build()` produces a `Cassandra\SSLOptions` object that you hand
to the cluster builder.

```php
$ssl = Cassandra::ssl()
    ->withTrustedCerts('/etc/ssl/certs/scylla-ca.pem')
    ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY)
    ->build();

$cluster = Cassandra::cluster()
    ->withContactPoints('db-1.example.com', 'db-2.example.com')
    ->withSSL($ssl)
    ->build();
```

::: danger Set the verify flags
The builder starts with no verify flags, which equals `Cassandra::VERIFY_NONE`. A connection built
without `withVerifyFlags()` encrypts the traffic but accepts any certificate, so it does not stop a
machine-in-the-middle attack. Always set the flags.
:::

## Verification levels

| Constant | Meaning |
| --- | --- |
| `Cassandra::VERIFY_NONE` | Encrypt only. The server certificate is not checked. |
| `Cassandra::VERIFY_PEER_CERT` | Check the server certificate against the trusted certificates. |
| `Cassandra::VERIFY_PEER_IDENTITY` | Also check that the certificate identity matches the peer. |

Combine the flags with the bitwise `or` operator:

```php
->withVerifyFlags(Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY)
```

`VERIFY_PEER_IDENTITY` compares the certificate against the address the driver dialed. When the
certificate carries host names and the driver connects by IP address, the check fails. Turn on
host name resolution so the driver learns the peer host names:

```php
$cluster = Cassandra::cluster()
    ->withContactPoints('db-1.example.com')
    ->withHostnameResolution(true)
    ->withSSL($ssl)
    ->build();
```

::: warning Host name resolution needs driver support
Some backends do not implement host name resolution. The driver emits a warning and continues with
the feature off. Issue certificates that carry the node IP addresses in the subject alternative name
when that happens.
:::

## Trusted certificates

`withTrustedCerts()` takes one or more paths to PEM files. Each file is read at `build()` time and
loaded into the TLS context.

```php
->withTrustedCerts('/etc/ssl/certs/scylla-ca.pem')

// Several certificates, for example during a CA rotation:
->withTrustedCerts('/etc/ssl/certs/scylla-ca.pem', '/etc/ssl/certs/scylla-ca-next.pem')
```

The paths must be readable by the PHP process. An unreadable path raises
`Cassandra\Exception\InvalidArgumentException`.

::: tip System trust store
The driver does not read the operating system trust store. Pass the CA certificate that signed the
node certificates, even when it is a public CA.
:::

## Client certificates (mutual TLS)

When the cluster requires client certificates, supply the certificate and the private key:

```php
$ssl = Cassandra::ssl()
    ->withTrustedCerts('/etc/ssl/certs/scylla-ca.pem')
    ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY)
    ->withClientCert('/etc/ssl/certs/app-client.pem')
    ->withPrivateKey('/etc/ssl/private/app-client-key.pem', getenv('KEY_PASSPHRASE'))
    ->build();
```

`withPrivateKey()` takes an optional passphrase. Omit it for an unencrypted key:

```php
->withPrivateKey('/etc/ssl/private/app-client-key.pem')
```

Both methods check that the path exists and is readable when you call them, not at `build()` time.

## Full example

```php
<?php

declare(strict_types=1);

function makeSslOptions(): Cassandra\SSLOptions
{
    $builder = Cassandra::ssl()
        ->withTrustedCerts(getenv('SCYLLA_CA') ?: '/etc/ssl/certs/scylla-ca.pem')
        ->withVerifyFlags(
            Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY
        );

    $clientCert = getenv('SCYLLA_CLIENT_CERT');
    $clientKey  = getenv('SCYLLA_CLIENT_KEY');

    if ($clientCert && $clientKey) {
        $builder = $builder
            ->withClientCert($clientCert)
            ->withPrivateKey($clientKey, getenv('SCYLLA_KEY_PASSPHRASE') ?: null);
    }

    return $builder->build();
}

$session = Cassandra::cluster()
    ->withContactPoints(...explode(',', getenv('SCYLLA_HOSTS')))
    ->withPort(9142)
    ->withCredentials(getenv('SCYLLA_USER'), getenv('SCYLLA_PASSWORD'))
    ->withSSL(makeSslOptions())
    ->withHostnameResolution(true)
    ->build()
    ->connect('shop');
```

Note the port. A cluster that terminates TLS on a separate port commonly uses 9142. Check the
`native_transport_port_ssl` setting of your cluster.

## Cost

TLS adds a handshake to every new connection and encrypts every frame. Two settings reduce the
impact:

- Keep the session alive so the handshake happens once. See [performance](/guide/performance).
- Raise the core connection count so the pool does not open connections during traffic peaks. See
  [connection pool and timeouts](/guide/connection-tuning).

## Debugging a failed handshake

A TLS failure surfaces as a connection error from `connect()`. Raise the driver log level to see the
reason from the C driver:

```ini
cassandra.log_level = DEBUG
cassandra.log = /var/log/php-cassandra.log
```

Then check the certificate chain from the command line:

```bash
openssl s_client -connect db-1.example.com:9142 -CAfile /etc/ssl/certs/scylla-ca.pem
```

Common causes:

| Symptom | Cause |
| --- | --- |
| Handshake fails at once, no server log | Wrong port. The plain CQL port does not speak TLS. |
| "certificate verify failed" | The CA file does not match the chain the node presents. |
| Verification passes by IP, fails by name | The certificate lacks the subject alternative name. |
| Works with `VERIFY_NONE` only | The trusted certificate is missing or is the wrong one. |
