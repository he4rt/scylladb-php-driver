--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$date = new Cassandra\Date(500);

echo $date->type();
?>
--EXPECT--
date
