<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Statements;

use Cassandra;
use Cassandra\Uuid;

/*
 * Migrated from tests-old/features/consistency_level.feature
 *
 * Both original scenarios required a 3-node ccm cluster with RF=3 plus
 * tracing enabled, then verified that system_traces.events showed all three
 * coordinators (127.0.0.1/2/3) — that's only meaningful with cluster
 * lifecycle control we don't have under Pest. The single-node Scylla docker
 * compose used by run-scylladb.sh cannot satisfy CONSISTENCY_ALL with RF=3.
 *
 * The scenarios are preserved here as skipped to keep the inventory honest.
 * Note: ConsistencyIntegrationTest.php in this directory already exercises
 * the consistency-level API surface against the single-node fixture, so
 * these scenarios are partially redundant with that test.
 */

it('accepts a consistency level via a deprecated ExecutionOptions object', function () {
    // Original: build a 3-node cluster, RF=3, INSERT with CONSISTENCY_ALL via
    // new Cassandra\ExecutionOptions(['consistency' => Cassandra::CONSISTENCY_ALL]),
    // then SELECT system_traces.events and assert sources include 127.0.0.1/2/3.
    $_unused = Cassandra::CONSISTENCY_ALL;
})->skip('Needs a 3-node cluster with RF=3 + tracing enabled to verify CONSISTENCY_ALL hits all replicas; not available in the single-node test fixture.');

it('accepts a consistency level via an array of execution options', function () {
    // Same as above but uses ['consistency' => Cassandra::CONSISTENCY_ALL] directly.
})->skip('Needs a 3-node cluster with RF=3 + tracing enabled; covered surface-wise by ConsistencyIntegrationTest.');
