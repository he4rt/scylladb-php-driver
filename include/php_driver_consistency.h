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

static inline int32_t php_driver_validate_consistency(uint32_t consistency)
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

static inline int32_t php_driver_validate_serial_consistency(uint32_t serial_consistency)
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
