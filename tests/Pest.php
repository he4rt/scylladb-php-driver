<?php

declare(strict_types=1);

use Cassandra\Session;
use Cassandra\Tests\Feature\TestCase;
use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;

uses(TestCase::class)->in('Feature');


expect()->extend('map', function (Closure $closure) {
    return expect($closure->call($this, $this->value));
});