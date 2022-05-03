--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Bigint(-10);
$bigint2 = new Cassandra\Bigint(10);

echo $bigint1->neg()->value() . PHP_EOL;
echo $bigint2->neg()->value();

?>
--EXPECT--
10
-10