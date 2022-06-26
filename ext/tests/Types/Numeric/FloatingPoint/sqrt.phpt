--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$float1 = new Cassandra\Float(25);

$float2 = new Cassandra\Float(-25);

echo $float1->sqrt() . PHP_EOL;

try {
    $float2->sqrt();
}catch(Cassandra\Exception\RangeException $e) {
    echo $e->getMessage() . PHP_EOL;
}

?>
--EXPECT--
5.0
Cannot take a square root of a negative number