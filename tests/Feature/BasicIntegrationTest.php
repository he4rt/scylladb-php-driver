<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature;

// The original tests-old/integration/Cassandra/BasicIntegrationTest.php was an
// abstract base class providing setUp()/tearDown() for concrete integration
// tests. In Pest, that role is filled by tests/Pest.php (scyllaDbConnection,
// migrateKeyspace, dropKeyspace) plus per-file beforeAll/afterAll hooks.
//
// There were no concrete test methods on the original abstract class, so
// nothing executable migrates here. This file exists only as a migration
// marker so the migration manifest stays 1:1 with tests-old/.

it('has no executable tests (abstract base class in PHPUnit world)', function () {
    expect(true)->toBeTrue();
})->skip('Abstract base class — see Pest.php helpers (scyllaDbConnection / migrateKeyspace / dropKeyspace) for the replacement.');
