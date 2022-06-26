--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float1 = new Cassandra\Float(-10);
$float2 = new Cassandra\Float(10);

echo $float1->neg()->value() . PHP_EOL;
echo $float2->neg()->value();

?>
--EXPECT--
10.0
-10.0