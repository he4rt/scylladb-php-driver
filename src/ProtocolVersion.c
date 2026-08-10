#include <cassandra.h>

#include <php_scylladb.h>
#include <php_scylladb_types.h>

/* gen_stub ignores docblocks on enum cases, so `@cvalue` cannot bind a case to
 * a CASS_* macro the way it does for class constants. The cases carry literal
 * values and these assertions hold them to the driver header instead. */
static_assert(CASS_PROTOCOL_VERSION_V1 == 1, "Cassandra\\ProtocolVersion::V1 no longer matches CASS_PROTOCOL_VERSION_V1");
static_assert(CASS_PROTOCOL_VERSION_V2 == 2, "Cassandra\\ProtocolVersion::V2 no longer matches CASS_PROTOCOL_VERSION_V2");
static_assert(CASS_PROTOCOL_VERSION_V3 == 3, "Cassandra\\ProtocolVersion::V3 no longer matches CASS_PROTOCOL_VERSION_V3");
static_assert(CASS_PROTOCOL_VERSION_V4 == 4, "Cassandra\\ProtocolVersion::V4 no longer matches CASS_PROTOCOL_VERSION_V4");
static_assert(CASS_PROTOCOL_VERSION_V5 == 5, "Cassandra\\ProtocolVersion::V5 no longer matches CASS_PROTOCOL_VERSION_V5");
