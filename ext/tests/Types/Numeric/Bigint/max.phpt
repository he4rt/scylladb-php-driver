--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint = Cassandra\Bigint::max();

echo $bigint->toInt()  === PHP_INT_MAX;
?>
--EXPECT--
1