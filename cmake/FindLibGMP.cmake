include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

# Debian ships gmp.pc, RHEL and its derivatives do not. pkg-config is therefore
# the fast path, not the only path.
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(LIBGMP QUIET IMPORTED_TARGET gmp)
endif ()

if (TARGET PkgConfig::LIBGMP)
    set(_libgmp_link PkgConfig::LIBGMP)
else ()
    find_path(LIBGMP_INCLUDE_DIR NAMES gmp.h)
    find_library(LIBGMP_LIBRARY  NAMES gmp)
    mark_as_advanced(LIBGMP_INCLUDE_DIR LIBGMP_LIBRARY)

    set(LIBGMP_LIBRARIES    "${LIBGMP_LIBRARY}")
    set(LIBGMP_INCLUDE_DIRS "${LIBGMP_INCLUDE_DIR}")

    # RHEL installs a multilib wrapper as gmp.h that only includes the real
    # per-architecture header, so the version macros are often absent. The
    # version is informational here, so report nothing rather than a partial
    # string.
    if (LIBGMP_INCLUDE_DIR AND EXISTS "${LIBGMP_INCLUDE_DIR}/gmp.h")
        file(STRINGS "${LIBGMP_INCLUDE_DIR}/gmp.h" _libgmp_major_line
                REGEX "^#define[ \t]+__GNU_MP_VERSION[ \t]+[0-9]+")
        file(STRINGS "${LIBGMP_INCLUDE_DIR}/gmp.h" _libgmp_minor_line
                REGEX "^#define[ \t]+__GNU_MP_VERSION_MINOR[ \t]+[0-9]+")
        file(STRINGS "${LIBGMP_INCLUDE_DIR}/gmp.h" _libgmp_patch_line
                REGEX "^#define[ \t]+__GNU_MP_VERSION_PATCHLEVEL[ \t]+[0-9]+")

        if (_libgmp_major_line AND _libgmp_minor_line AND _libgmp_patch_line)
            string(REGEX MATCH "[0-9]+$" _libgmp_major "${_libgmp_major_line}")
            string(REGEX MATCH "[0-9]+$" _libgmp_minor "${_libgmp_minor_line}")
            string(REGEX MATCH "[0-9]+$" _libgmp_patch "${_libgmp_patch_line}")
            set(LIBGMP_VERSION "${_libgmp_major}.${_libgmp_minor}.${_libgmp_patch}")
        endif ()

        unset(_libgmp_major_line)
        unset(_libgmp_minor_line)
        unset(_libgmp_patch_line)
        unset(_libgmp_major)
        unset(_libgmp_minor)
        unset(_libgmp_patch)
    endif ()

    if (LIBGMP_LIBRARY AND NOT TARGET LibGMP::gmp)
        add_library(LibGMP::gmp UNKNOWN IMPORTED GLOBAL)
        set_target_properties(LibGMP::gmp PROPERTIES
                IMPORTED_LOCATION             "${LIBGMP_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBGMP_INCLUDE_DIR}"
        )
    endif ()

    set(_libgmp_link LibGMP::gmp)
endif ()

# ── Create LibGMP::LibGMP INTERFACE IMPORTED target ──────────────────────────
if (NOT TARGET LibGMP::LibGMP)
    add_library(LibGMP::LibGMP INTERFACE IMPORTED GLOBAL)
    target_link_libraries(LibGMP::LibGMP INTERFACE ${_libgmp_link})
endif ()

find_package_handle_standard_args(LibGMP
        REQUIRED_VARS LIBGMP_LIBRARIES
        VERSION_VAR   LIBGMP_VERSION
)
