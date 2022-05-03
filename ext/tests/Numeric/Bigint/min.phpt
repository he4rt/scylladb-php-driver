--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint = Cassandra\Bigint::min();


echo $bigint->toInt() === PHP_INT_MIN;
?>
--EXPECT--
1