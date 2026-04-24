#pragma once

// PHP_SCYLLADB_API is intentionally empty — all internal symbols are hidden
// by -fvisibility=hidden in TargetOptimizations.cmake. Only get_module is
// exported, via ZEND_DLEXPORT, as required by the PHP extension ABI.
#define PHP_SCYLLADB_API
