--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float1 = new Cassandra\Float(-10);
$float2 = new Cassandra\Float(10);

$float1 = new Cassandra\Float(-10.345);
$float2 = new Cassandra\Float(10.345);

echo $float1->abs()->value() . PHP_EOL;
echo $float2->abs()->value();

unset($float1);
unset($float2);
?>
--EXPECT--
10.345000267029
10.345000267029