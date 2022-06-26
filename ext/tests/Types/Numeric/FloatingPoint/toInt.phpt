--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Float(25.6789976);

echo is_int($bigint1->toInt()) . ' ' . $bigint1->toInt();
?>
--EXPECT--
1 25