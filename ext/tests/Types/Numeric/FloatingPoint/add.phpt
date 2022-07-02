--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float1 = new Cassandra\Float(10.1);
$float2 = new Cassandra\Float(10.6);

$result = $float2->add($float1);

echo $result->value();

unset($float1);
unset($float2);
unset($result);
?>
--EXPECT--
20.700000762939