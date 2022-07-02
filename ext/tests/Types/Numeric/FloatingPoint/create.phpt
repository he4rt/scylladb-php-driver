--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float1 = new Cassandra\Float(10);
$floatStr = new Cassandra\Float('100');
$floatStrFloating = new Cassandra\Float('500.6543');
$floatFloat = new Cassandra\Float($floatStrFloating);
$floatDouble = new Cassandra\Float(10.2);

echo $float1->value() . ' ' . $floatStr->value() . ' ' . $floatStrFloating->value() . ' ' . $floatFloat->value() . ' ' . $floatDouble->value();

?>
--EXPECT--
10 100 500.654296875 500.654296875 10.199999809265
