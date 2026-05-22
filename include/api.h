#pragma once

// PHP_SCYLLADB_API is intentionally empty — all internal symbols are hidden
// by -fvisibility=hidden in TargetOptimizations.cmake. Only get_module is
// exported, via ZEND_DLEXPORT, as required by the PHP extension ABI.
#define PHP_SCYLLADB_API

// BEGIN_EXTERN_C / END_EXTERN_C come from <Zend/zend_portability.h> via php.h,
// which every TU includes. No project-local definition needed.
