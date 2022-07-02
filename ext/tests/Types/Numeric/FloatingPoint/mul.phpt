--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float = new Cassandra\Float(10);
$float2 = new Cassandra\Float(10);

$result = $float2->mul($float);

echo $result->value();

?>
--EXPECT--
100