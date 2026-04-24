include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGMP REQUIRED IMPORTED_TARGET gmp)

# ── Create LibGMP::LibGMP INTERFACE IMPORTED target ──────────────────────────
if (NOT TARGET LibGMP::LibGMP)
    add_library(LibGMP::LibGMP INTERFACE IMPORTED GLOBAL)
    target_link_libraries(LibGMP::LibGMP INTERFACE PkgConfig::LIBGMP)
endif ()

find_package_handle_standard_args(LibGMP
        REQUIRED_VARS LIBGMP_LIBRARIES
        VERSION_VAR   LIBGMP_VERSION
)
