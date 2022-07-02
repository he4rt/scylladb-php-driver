--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
use Cassandra\TimestampGenerators\{ServerSideGenerator, TimestampGeneratorInterface};

try {
    $gen = new ServerSideGenerator();

    echo $gen instanceof TimestampGeneratorInterface . "\n";

    echo 'Success';
} catch(Throwable $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
1
Success