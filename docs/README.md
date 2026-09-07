# Design notes and audits

This directory holds long-form working documents: the async design note, the memory leak audit,
the ZTS correctness audit and the plans under `plans/`.

User documentation lives in `website/` and is published to
<https://he4rt.github.io/scylladb-php-driver/>.

IDE stubs are generated from the `src/**/*.stub.php` sources:

```bash
php -d extension=cassandra tools/gen_ide_stubs.php build/ide-stubs
```

The release workflow runs that command and publishes the result as the IDE stubs asset.
