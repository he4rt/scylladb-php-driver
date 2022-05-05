--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint = new Cassandra\Bigint(10);
$bigintStr = new Cassandra\Bigint('100');
$bigintBigint = new Cassandra\Bigint($bigintStr);
$bigintDouble = new Cassandra\Bigint(10.2);

echo $bigint->value() . ' ' . $bigintStr->value() . ' ' . $bigintBigint->value() . ' ' . $bigintDouble->value();

unset($bigint1);
unset($bigintStr);
unset($bigintBigint);
unset($bigintDouble);
?>
--EXPECT--
10 100 100 10
