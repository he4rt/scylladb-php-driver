<?php
declare(strict_type=1);

namespace Cassandra;

final class Bigint
{
    public function __construct(int|float|Bigint|string $value) { }

    public function add(Bigint $other): Bigint { }

    public function value(Bigint $other): string { }

    public function __toString(): string { }
}