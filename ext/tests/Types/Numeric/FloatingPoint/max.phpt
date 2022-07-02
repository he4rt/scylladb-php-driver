--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float = Cassandra\Float::max();

echo $float->value();
?>
--EXPECT--
3.4028234663853E+38