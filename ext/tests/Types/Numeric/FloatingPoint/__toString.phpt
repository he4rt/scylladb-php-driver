--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$floatingPoint = new Cassandra\Float(10.0);

echo (string)$floatingPoint . PHP_EOL;
echo $floatingPoint->value() . PHP_EOL;
echo $floatingPoint->__toString() . PHP_EOL;

?>
--EXPECT--
10.0
10.0
10.0
