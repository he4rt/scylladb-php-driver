--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
use Cassandra\TimestampGenerators\{MonotonicGenerator, TimestampGeneratorInterface};

try {
    $gen = new MonotonicGenerator();

    echo $gen instanceof TimestampGeneratorInterface . "\n";

    echo 'Success';
} catch(Throwable $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
1
Success