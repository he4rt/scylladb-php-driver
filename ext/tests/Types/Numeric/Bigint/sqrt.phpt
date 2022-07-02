--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
$bigint1 = new Cassandra\Bigint(25);

$bigint2 = new Cassandra\Bigint(-25);

echo $bigint1->sqrt() . PHP_EOL;

try {
    $bigint2->sqrt();
}catch(RangeException $e) {
    echo $e->getMessage() . PHP_EOL;
}

?>
--EXPECT--
5
Cannot take a square root of a negative number