<?php

/**
 * @generate-class-entries
 */

declare(strict_types=1);

namespace Cassandra {
    /**
     * Native protocol version used on the wire.
     *
     * The backing value is the version byte the protocol itself defines, so it
     * matches CASS_PROTOCOL_VERSION_V1 … CASS_PROTOCOL_VERSION_V5.
     */
    enum ProtocolVersion: int
    {
        case V1 = 1;
        case V2 = 2;
        case V3 = 3;
        case V4 = 4;
        case V5 = 5;
    }
}
