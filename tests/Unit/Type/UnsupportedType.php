<?php

declare(strict_types=1);

namespace Cassandra\Type;

use Cassandra\Type;

/**
 * Test helper — a Cassandra\Type subclass that is intentionally not a recognised
 * built-in type, used to verify rejection paths in the extension. Lives in the
 * Cassandra\Type namespace on purpose so error messages produced by the driver
 * read "Cassandra\Type\UnsupportedType" exactly as in the upstream tests.
 */
class UnsupportedType extends Type
{
    public function name(): string
    {
        return 'unsupported';
    }

    public function __toString(): string
    {
        return 'unsupported';
    }

    public function create($value = null)
    {
        return null;
    }
}
