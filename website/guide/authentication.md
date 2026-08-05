# Authentication

The driver supports the password authenticator that ScyllaDB and Cassandra ship with
(`PasswordAuthenticator`). Supply the credentials on the builder.

```php
$cluster = Cassandra::cluster()
    ->withContactPoints('10.0.0.1')
    ->withCredentials('app_user', getenv('SCYLLA_PASSWORD'))
    ->build();
```

The password parameter carries the `#[\SensitiveParameter]` attribute. PHP redacts it in stack
traces, so a leaked exception log does not expose the password.

## Where credentials go

Never write the password into source code. Read it from the environment or from a secret store:

```php
$password = getenv('SCYLLA_PASSWORD');

if ($password === false || $password === '') {
    throw new RuntimeException('SCYLLA_PASSWORD is not set');
}

$builder->withCredentials(getenv('SCYLLA_USER') ?: 'app_user', $password);
```

## Credentials travel in clear text

The CQL password authenticator sends the user name and the password in a plain protocol frame.
Anyone on the network path can read them.

::: danger Always pair credentials with TLS
Turn on TLS whenever the connection leaves a trusted network segment. See
[TLS and SSL](/guide/tls).
:::

```php
$cluster = Cassandra::cluster()
    ->withContactPoints('db.internal.example.com')
    ->withCredentials('app_user', getenv('SCYLLA_PASSWORD'))
    ->withSSL(
        Cassandra::ssl()
            ->withTrustedCerts('/etc/ssl/certs/scylla-ca.pem')
            ->withVerifyFlags(Cassandra::VERIFY_PEER_CERT | Cassandra::VERIFY_PEER_IDENTITY)
            ->build()
    )
    ->build();
```

## Failures

A wrong user name or password produces `Cassandra\Exception\AuthenticationException` from
`connect()`. The exception message comes from the server.

```php
use Cassandra\Exception\AuthenticationException;

try {
    $session = $cluster->connect('shop');
} catch (AuthenticationException $e) {
    // Do not log the password. Log the user name and the server message only.
    error_log(sprintf('auth failed for %s: %s', $user, $e->getMessage()));
    throw $e;
}
```

The exception arrives at `connect()` time, not at `build()` time. `build()` performs no network
work.

## A node with no authenticator

When the cluster runs `AllowAllAuthenticator`, credentials are ignored. Sending them is harmless.
The default ScyllaDB Docker image works this way, so local development needs no credentials:

```php
$session = Cassandra::cluster()
    ->withContactPoints('127.0.0.1')
    ->build()
    ->connect();
```

## Per-user sessions

The driver has no per-request user switch. Authentication belongs to the cluster configuration, so a
second identity needs a second cluster and a second session.

```php
$readOnly  = makeCluster('reporting_user', $reportingPassword)->connect('shop');
$readWrite = makeCluster('app_user', $appPassword)->connect('shop');
```

Each session opens its own connection pool. Two identities cost twice the sockets. Prefer one
application identity plus CQL permissions on the server side.

## Role permissions

Authorization stays on the server. Grant the smallest useful set:

```sql
CREATE ROLE app_user WITH PASSWORD = '...' AND LOGIN = true;

GRANT SELECT ON KEYSPACE shop TO app_user;
GRANT MODIFY ON KEYSPACE shop TO app_user;
```

A denied operation raises `Cassandra\Exception\UnauthorizedException` at query time.
