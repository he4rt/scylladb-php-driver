--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
use Cassandra\Exceptions\DivideByZeroException;

$float1 = new Cassandra\Float(0);
$float2 = new Cassandra\Float(10);

try {
    $result = $float2->div($float1);
} catch(DivideByZeroException $e) {
    echo $e->getMessage();
    return;
}

echo 'failed';
?>
--EXPECT--
Cannot divide by zero
