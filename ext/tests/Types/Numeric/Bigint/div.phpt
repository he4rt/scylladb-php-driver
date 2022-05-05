--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Bigint(10);
$bigint2 = new Cassandra\Bigint(10);

$result = $bigint2->div($bigint1);

echo $result->value();

unset($bigint1);
unset($bigint2);
unset($result);
?>
--EXPECT--
1