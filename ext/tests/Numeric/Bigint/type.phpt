--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint = new Cassandra\Bigint(10);

echo $bigint->type();

?>
--EXPECT--
bigint
