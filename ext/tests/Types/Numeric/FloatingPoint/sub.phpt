--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float1 = new Cassandra\Float(10.5);
$float2 = new Cassandra\Float(10);

$result = $float2->sub($float1);

echo $result->value();
?>
--EXPECT--
-0.5