<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;

describe('Cassandra\Cluster\Builder credentials', function () {

    it('does not expose the password through object properties', function () {
        $builder = Cassandra::cluster()->withCredentials('bob', 'hunter2');

        $props = (array) $builder;

        expect($props)->toHaveKey('password');
        expect($props['password'])->toBe('***');
        expect($props['username'])->toBe('bob');
    });

    it('does not expose the password through print_r or var_export', function () {
        $builder = Cassandra::cluster()->withCredentials('bob', 'hunter2');

        expect(print_r($builder, true))->not->toContain('hunter2');
        expect(var_export($builder, true))->not->toContain('hunter2');
    });

    it('keeps password null when no credentials are set', function () {
        $props = (array) Cassandra::cluster();

        expect($props['username'])->toBeNull();
        expect($props['password'])->toBeNull();
    });
});
