<?php

declare(strict_types=1);

/**
 * Helper functions (env, scyllaDbConnection, migrateKeyspace, dropKeyspace)
 * live in tests/Support/helpers.php and are loaded eagerly via composer's
 * autoload-dev.files so they are available before Pest's bootstrap.
 *
 * This file is reserved for Pest-specific registration: uses() / expect()
 * extensions / shared dataset definitions.
 */

uses()->group('unit')->in('Unit');
uses()->group('feature')->in('Feature');

expect()->extend('map', function (Closure $fn) {
    return expect($fn($this->value));
});
