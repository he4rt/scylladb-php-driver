<?php

/**
* @generate-class-entries
*/

declare(strict_types=1);

namespace Cassandra\RetryPolicy {
    /**
     * @strict-properties
     * @deprecated
     *
     * This still works, but should not be used in new applications.
     * It can lead to unexpected behavior when the cluster is in a degraded state.
     * Instead, applications should prefer using the lowest consistency level
     * on statements that can be tolerated by a specific use case.
 * @scylladb-struct php_scylladb_retry_policy
 */    final class DowngradingConsistency implements \Cassandra\RetryPolicy { }
}
