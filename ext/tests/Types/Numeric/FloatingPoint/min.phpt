--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float = Cassandra\Float::min();

echo $float->value();
?>
--EXPECT--
1.1754943508223E-38