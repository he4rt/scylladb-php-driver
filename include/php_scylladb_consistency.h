/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#pragma once

#include <cassandra.h>
#include <stdint.h>
#include <strings.h>

#define PHP_SCYLLADB_CONSISTENCY_MAP(XX)      \
    XX("any", CASS_CONSISTENCY_ANY)           \
    XX("one", CASS_CONSISTENCY_ONE)           \
    XX("two", CASS_CONSISTENCY_TWO)           \
    XX("three", CASS_CONSISTENCY_THREE)       \
    XX("quorum", CASS_CONSISTENCY_QUORUM)     \
    XX("all", CASS_CONSISTENCY_ALL)           \
    XX("local_quorum", CASS_CONSISTENCY_LOCAL_QUORUM) \
    XX("each_quorum", CASS_CONSISTENCY_EACH_QUORUM)   \
    XX("serial", CASS_CONSISTENCY_SERIAL)     \
    XX("local_serial", CASS_CONSISTENCY_LOCAL_SERIAL) \
    XX("local_one", CASS_CONSISTENCY_LOCAL_ONE)

static inline int32_t php_scylladb_validate_consistency(uint32_t consistency)
{
    switch (consistency)
    {
    case CASS_CONSISTENCY_ANY:
    case CASS_CONSISTENCY_ONE:
    case CASS_CONSISTENCY_TWO:
    case CASS_CONSISTENCY_THREE:
    case CASS_CONSISTENCY_QUORUM:
    case CASS_CONSISTENCY_ALL:
    case CASS_CONSISTENCY_LOCAL_QUORUM:
    case CASS_CONSISTENCY_EACH_QUORUM:
    case CASS_CONSISTENCY_SERIAL:
    case CASS_CONSISTENCY_LOCAL_SERIAL:
    case CASS_CONSISTENCY_LOCAL_ONE:
        return 0;
    default:
        return -1;
    }
}

/* Maps a php.ini spelling ("LOCAL_QUORUM", "local_quorum") to its CassConsistency.
 * Returns -1 when the name is not one of PHP_SCYLLADB_CONSISTENCY_MAP. */
static inline int32_t php_scylladb_consistency_from_name(const char *name)
{
#define XX(literal, value) \
    if (strcasecmp(name, literal) == 0) return (int32_t)(value);
    PHP_SCYLLADB_CONSISTENCY_MAP(XX)
#undef XX
    return -1;
}

static inline int32_t php_scylladb_validate_serial_consistency(uint32_t serial_consistency)
{
    switch (serial_consistency)
    {
    case CASS_CONSISTENCY_SERIAL:
    case CASS_CONSISTENCY_LOCAL_SERIAL:
        return 0;
    default:
        return -1;
    }
}
