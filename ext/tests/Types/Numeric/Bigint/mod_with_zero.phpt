--TEST--
--SKIPIF--
<?php if (!extension_loaded("cassandra")) die("Skipped: cassandra extension required."); ?>
--FILE--
<?php
use Cassandra\Exceptions\DivideByZeroException;

$bigint1 = new Cassandra\Bigint(0);
$bigint2 = new Cassandra\Bigint(10);

try {
    $result = $bigint2->mod($bigint1);
} catch(DivideByZeroException $e) {
    echo $e->getMessage();
    return;
} finally {
    unset($bigint1);
    unset($bigint2);
    unset($result);
}

echo 'failed';
?>
--EXPECT--
Cannot modulo by zero
