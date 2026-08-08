---
layout: home

hero:
  name: ScyllaDB PHP Driver
  text: Native CQL for PHP 8
  tagline: A C extension that speaks the CQL binary protocol directly. Built on the ScyllaDB C/C++ driver, with token-aware routing, prepared statements, async futures, and full CQL type support.
  image:
    src: /logo.svg
    alt: ScyllaDB PHP driver
  actions:
    - theme: brand
      text: Get started
      link: /guide/quickstart
    - theme: alt
      text: Installation
      link: /guide/installation
    - theme: alt
      text: View on GitHub
      link: https://github.com/he4rt/scylladb-php-driver

features:
  - icon: ⚡
    title: Native protocol, no PHP overhead
    details: Queries run through the ScyllaDB C/C++ driver. Result rows decode in C and arrive as PHP values. No CQL parsing or text protocol in userland.
    link: /guide/introduction
    linkText: How it works

  - icon: 🎯
    title: Token-aware and shard-aware
    details: The driver sends each request to a replica that owns the data. Add datacenter-aware and latency-aware routing to control which nodes get traffic.
    link: /guide/load-balancing
    linkText: Routing options

  - icon: 🔐
    title: TLS with peer verification
    details: Load trusted certificates, present a client certificate, and choose the verification level. Credentials use the SensitiveParameter attribute.
    link: /guide/tls
    linkText: Set up TLS

  - icon: 🧩
    title: Every CQL type
    details: Collections, tuples, user defined types, durations, decimals, varints, inet addresses, and time UUIDs all map to dedicated PHP classes.
    link: /guide/data-types
    linkText: Type mapping

  - icon: 🔀
    title: Futures for concurrency
    details: Each blocking call has an async twin. Start many queries, then collect the results. Result pages fetch asynchronously too.
    link: /guide/async
    linkText: Run queries in parallel

  - icon: 🐘
    title: PHP 8.2 through 8.5
    details: Typed signatures generated from PHP stubs, NTS and ZTS builds, and installation through PIE. Tested against ScyllaDB 4.4 to 6.x and Cassandra 3.0+.
    link: /guide/installation
    linkText: Compatibility
---

<div style="max-width: 1152px; margin: 64px auto 0; padding: 0 24px;">

## From zero to a query

```php
<?php

$session = Cassandra::cluster()
    ->withContactPoints('10.0.0.1', '10.0.0.2', '10.0.0.3')
    ->withPort(9042)
    ->withCredentials('scylla', getenv('SCYLLA_PASSWORD'))
    ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
    ->withTokenAwareRouting(true)
    ->build()
    ->connect('shop');

$statement = $session->prepare('SELECT id, email FROM users WHERE id = ?');

$rows = $session->execute($statement, [
    'arguments' => [new Cassandra\Uuid('7b5a4c1e-8f2a-4c9e-9a1b-3c2d1e0f5a6b')],
]);

foreach ($rows as $row) {
    printf("%s <%s>\n", $row['id'], $row['email']);
}
```

Read the [quick start](/guide/quickstart) for a complete first application, or go to
[clusters and sessions](/guide/connecting) for the full connection model.

</div>
