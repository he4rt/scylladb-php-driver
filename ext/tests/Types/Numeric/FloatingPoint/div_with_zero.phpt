--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
use Cassandra\Exception\DivideByZeroException;

$float1 = new Cassandra\Float(0);
$float2 = new Cassandra\Float(10);

try {
    $result = $bigint2->div($bigint1);
} catch(DivideByZeroException $e) {
    echo $e->getMessage();
    return;
}

echo 'failed';
?>
--EXPECT--
Cannot divide by zero
