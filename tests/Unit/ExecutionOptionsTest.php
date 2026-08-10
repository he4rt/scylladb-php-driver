<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;
use Cassandra\ExecutionOptions;

beforeEach(function () {
    error_reporting(E_ALL ^ E_DEPRECATED);
});

afterEach(function () {
    error_reporting(E_ALL);
});

it('allows retrieving settings by name', function () {
    $options = new ExecutionOptions([
        'consistency'        => Cassandra::CONSISTENCY_ANY,
        'serial_consistency' => Cassandra::CONSISTENCY_LOCAL_SERIAL,
        'page_size'          => 15000,
        'timeout'            => 15,
        'arguments'          => ['a', 1, 'b', 2, 'c', 3],
    ]);

    expect($options->consistency)->toEqual(Cassandra::CONSISTENCY_ANY);
    expect($options->serialConsistency)->toEqual(Cassandra::CONSISTENCY_LOCAL_SERIAL);
    expect($options->pageSize)->toEqual(15000);
    expect($options->timeout)->toEqual(15);
    expect($options->arguments)->toEqual(['a', 1, 'b', 2, 'c', 3]);
});

it('returns null values when retrieving undefined settings by name', function () {
    $options = new ExecutionOptions([]);

    expect($options->consistency)->toBeNull();
    expect($options->serialConsistency)->toBeNull();
    expect($options->pageSize)->toBeNull();
    expect($options->timeout)->toBeNull();
    expect($options->arguments)->toBeNull();
});
