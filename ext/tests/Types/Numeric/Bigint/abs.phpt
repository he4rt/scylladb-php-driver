--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Bigint(-10);
$bigint2 = new Cassandra\Bigint(10);


echo $bigint1->abs()->value() . PHP_EOL;
echo $bigint2->abs()->value();

unset($bigint1);
unset($bigint2);
?>
--EXPECT--
10
10