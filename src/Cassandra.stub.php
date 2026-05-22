<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace {

/**
 * @strict-properties
 */
final class Cassandra
{
    /* CassConsistency — matches values in cassandra.h */
    public const int CONSISTENCY_ANY          = 0x0000;
    public const int CONSISTENCY_ONE          = 0x0001;
    public const int CONSISTENCY_TWO          = 0x0002;
    public const int CONSISTENCY_THREE        = 0x0003;
    public const int CONSISTENCY_QUORUM       = 0x0004;
    public const int CONSISTENCY_ALL          = 0x0005;
    public const int CONSISTENCY_LOCAL_QUORUM = 0x0006;
    public const int CONSISTENCY_EACH_QUORUM  = 0x0007;
    public const int CONSISTENCY_SERIAL       = 0x0008;
    public const int CONSISTENCY_LOCAL_SERIAL = 0x0009;
    public const int CONSISTENCY_LOCAL_ONE    = 0x000A;

    /* CassSslVerifyFlags */
    public const int VERIFY_NONE          = 0x00;
    public const int VERIFY_PEER_CERT     = 0x01;
    public const int VERIFY_PEER_IDENTITY = 0x02;

    /* CassBatchType */
    public const int BATCH_LOGGED   = 0x00;
    public const int BATCH_UNLOGGED = 0x01;
    public const int BATCH_COUNTER  = 0x02;

    /* CassLogLevel */
    public const int LOG_DISABLED = 0;
    public const int LOG_CRITICAL = 1;
    public const int LOG_ERROR    = 2;
    public const int LOG_WARN     = 3;
    public const int LOG_INFO     = 4;
    public const int LOG_DEBUG    = 5;
    public const int LOG_TRACE    = 6;

    /* CQL primitive type name constants (used by Type\Scalar) */
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

    /* VERSION and CPP_DRIVER_VERSION are populated at MINIT
       (CPP_DRIVER_VERSION is runtime-computed from the linked libcassandra
       or libscylla-cpp-driver). The literals here are placeholders
       overridden by src/Core.cpp before any user code runs. */
    public const string VERSION            = "0.0.0";
    public const string CPP_DRIVER_VERSION = "0.0.0";

    public static function cluster(): \Cassandra\Cluster\Builder {}
    public static function ssl(): \Cassandra\SSLOptions\Builder {}
}

}
