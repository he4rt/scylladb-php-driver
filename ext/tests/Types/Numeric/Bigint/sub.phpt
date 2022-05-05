--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Bigint(10);
$bigint2 = new Cassandra\Bigint(10);

$result = $bigint2->sub($bigint1);

echo $result->value();
?>
--EXPECT--
0