<?php

declare(strict_types=1);

/**
 * PhpBench bootstrap.
 *
 * Runs once inside every benchmark subprocess before any subject executes.
 * Its only jobs are to make the benchmark/support classes autoloadable and
 * to fail fast — with an actionable message — if the extension under test is
 * not actually loaded in the PHP that PhpBench spawned.
 */

$autoload = dirname(__DIR__) . '/vendor/autoload.php';

if (! is_file($autoload)) {
    fwrite(STDERR, "vendor/autoload.php not found — run `composer install` first.\n");
    exit(1);
}

require $autoload;

if (! extension_loaded('cassandra')) {
    $dylib = dirname(__DIR__) . '/out/DebugPHP8.5NTS/cassandra.dylib';
    $hint  = is_file($dylib) ? $dylib : '/abs/path/to/cassandra.(so|dylib)';

    fwrite(STDERR, <<<MSG
        ┌─ scylladb-php-driver benchmarks ───────────────────────────────────────┐
        │ The 'cassandra' extension is NOT loaded in the PHP PhpBench is using.   │
        └────────────────────────────────────────────────────────────────────────┘

        phpbench.json asks the runner for `extension=cassandra`, which only works
        when the module is installed in the active PHP's extension_dir.

        For a local dev build that lives under out/, point PhpBench at it directly:

            vendor/bin/phpbench run \\
                --php-binary=php/8.5-debug-nts/bin/php \\
                --php-config='{"extension":"$hint"}'

        (or export it once and drop the --php-config flag)

        MSG);
    exit(1);
}

