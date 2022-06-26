--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float = Cassandra\Float::max();

echo $float->value() === PHP_FLOAT_MIN;
?>
--EXPECT--
1