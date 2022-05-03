--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Bigint(25);

echo is_int($bigint1->toInt());
?>
--EXPECT--
1