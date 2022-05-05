--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint = new Cassandra\Bigint(10);

echo (string)$bigint . PHP_EOL;
echo $bigint->value() . PHP_EOL;
echo $bigint->__toString() . PHP_EOL;

?>
--EXPECT--
10
10
10
