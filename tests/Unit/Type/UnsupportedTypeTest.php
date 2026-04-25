<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit\Type;

use Cassandra\Type;
use Cassandra\Type\UnsupportedType;

require_once __DIR__ . '/UnsupportedType.php';

// The original tests-old/unit/Cassandra/Type/UnsupportedType.php was a test
// fixture (not a test class). It is kept as a helper in UnsupportedType.php
// alongside this file. These trivial assertions guard against accidental
// regressions in the helper itself.

it('is a subclass of Cassandra\\Type', function () {
    expect(new UnsupportedType())->toBeInstanceOf(Type::class);
});

it('reports the unsupported name', function () {
    $type = new UnsupportedType();
    expect($type->name())->toEqual('unsupported')
        ->and((string) $type)->toEqual('unsupported');
});

it('returns null from create', function () {
    expect((new UnsupportedType())->create())->toBeNull()
        ->and((new UnsupportedType())->create('value'))->toBeNull();
});
