<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace {

/**
 * @strict-properties
 */
final class Cassandra
{
    /* CassConsistency — values bound to cassandra.h via @cvalue. */

    /** @cvalue CASS_CONSISTENCY_ANY */
    public const int CONSISTENCY_ANY = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_ONE */
    public const int CONSISTENCY_ONE = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_TWO */
    public const int CONSISTENCY_TWO = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_THREE */
    public const int CONSISTENCY_THREE = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_QUORUM */
    public const int CONSISTENCY_QUORUM = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_ALL */
    public const int CONSISTENCY_ALL = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_LOCAL_QUORUM */
    public const int CONSISTENCY_LOCAL_QUORUM = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_EACH_QUORUM */
    public const int CONSISTENCY_EACH_QUORUM = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_SERIAL */
    public const int CONSISTENCY_SERIAL = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_LOCAL_SERIAL */
    public const int CONSISTENCY_LOCAL_SERIAL = UNKNOWN;
    /** @cvalue CASS_CONSISTENCY_LOCAL_ONE */
    public const int CONSISTENCY_LOCAL_ONE = UNKNOWN;

    /* CassSslVerifyFlags */
    /** @cvalue CASS_SSL_VERIFY_NONE */
    public const int VERIFY_NONE = UNKNOWN;
    /** @cvalue CASS_SSL_VERIFY_PEER_CERT */
    public const int VERIFY_PEER_CERT = UNKNOWN;
    /** @cvalue CASS_SSL_VERIFY_PEER_IDENTITY */
    public const int VERIFY_PEER_IDENTITY = UNKNOWN;

    /* CassBatchType */
    /** @cvalue CASS_BATCH_TYPE_LOGGED */
    public const int BATCH_LOGGED = UNKNOWN;
    /** @cvalue CASS_BATCH_TYPE_UNLOGGED */
    public const int BATCH_UNLOGGED = UNKNOWN;
    /** @cvalue CASS_BATCH_TYPE_COUNTER */
    public const int BATCH_COUNTER = UNKNOWN;

    /* CassLogLevel */
    /** @cvalue CASS_LOG_DISABLED */
    public const int LOG_DISABLED = UNKNOWN;
    /** @cvalue CASS_LOG_CRITICAL */
    public const int LOG_CRITICAL = UNKNOWN;
    /** @cvalue CASS_LOG_ERROR */
    public const int LOG_ERROR = UNKNOWN;
    /** @cvalue CASS_LOG_WARN */
    public const int LOG_WARN = UNKNOWN;
    /** @cvalue CASS_LOG_INFO */
    public const int LOG_INFO = UNKNOWN;
    /** @cvalue CASS_LOG_DEBUG */
    public const int LOG_DEBUG = UNKNOWN;
    /** @cvalue CASS_LOG_TRACE */
    public const int LOG_TRACE = UNKNOWN;

    /* CQL primitive type name constants (used by Type\Scalar).
       These are the canonical wire names; no C-side identifier to bind to. */
    public const string TYPE_TEXT      = "text";
    public const string TYPE_ASCII     = "ascii";
    public const string TYPE_VARCHAR   = "varchar";
    public const string TYPE_BIGINT    = "bigint";
    public const string TYPE_SMALLINT  = "smallint";
    public const string TYPE_TINYINT   = "tinyint";
    public const string TYPE_BLOB      = "blob";
    public const string TYPE_BOOLEAN   = "boolean";
    public const string TYPE_COUNTER   = "counter";
    public const string TYPE_DECIMAL   = "decimal";
    public const string TYPE_DOUBLE    = "double";
    public const string TYPE_FLOAT     = "float";
    public const string TYPE_INT       = "int";
    public const string TYPE_TIMESTAMP = "timestamp";
    public const string TYPE_UUID      = "uuid";
    public const string TYPE_VARINT    = "varint";
    public const string TYPE_TIMEUUID  = "timeuuid";
    public const string TYPE_INET      = "inet";

    /** Driver version. @cvalue resolves to the PHP_SCYLLADB_VERSION macro. */
    /** @cvalue PHP_SCYLLADB_VERSION */
    public const string VERSION = UNKNOWN;

    /** Linked Cassandra C/C++ driver version. Runtime-computed; the stub
        placeholder is overridden in src/Core.c#php_scylladb_core_post_register. */
    public const string CPP_DRIVER_VERSION = "0.0.0";

    public static function cluster(): \Cassandra\Cluster\Builder {}
    public static function ssl(): \Cassandra\SSLOptions\Builder {}
}

}
