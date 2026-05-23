include_guard(GLOBAL)

include(CheckIPOSupported)
include(CheckCCompilerFlag)

# ── Pre-check compiler capabilities once at configure time ───────────────────
# Results are CACHE INTERNAL — subsequent calls are no-ops.
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    check_c_compiler_flag(-mavx  _PHP_SCYLLADB_CC_SUPPORTS_AVX)
    check_c_compiler_flag(-mavx2 _PHP_SCYLLADB_CC_SUPPORTS_AVX2)
endif ()

check_ipo_supported(RESULT _PHP_SCYLLADB_LTO_SUPPORTED OUTPUT _PHP_SCYLLADB_LTO_MSG)
if (NOT _PHP_SCYLLADB_LTO_SUPPORTED)
    message(STATUS "LTO not available: ${_PHP_SCYLLADB_LTO_MSG}")
endif ()

# ── scylladb_php_library ──────────────────────────────────────────────────────
# Apply standard compiler settings to a target. Reads ENABLE_SANITIZERS,
# ENABLE_LTO, ENABLE_AVX, ENABLE_AVX2, CPU_TYPE from CMake cache.
#
# Usage: scylladb_php_library(<target>)
function(scylladb_php_library target)
    # ── Language standards ────────────────────────────────────────────────────
    target_compile_features(${target} PUBLIC c_std_23)
    set_target_properties(${target} PROPERTIES
            C_STANDARD            23
            C_STANDARD_REQUIRED   ON
            C_EXTENSIONS          ON
            POSITION_INDEPENDENT_CODE ON
    )

    # ── External dependency headers via INTERFACE targets ─────────────────────
    target_link_libraries(${target} PUBLIC
            PHP::PHP
            CppDriver::Driver
            LibGMP::LibGMP
    )

    # ── Project-internal include paths ────────────────────────────────────────
    target_include_directories(${target} PUBLIC
            "${PROJECT_SOURCE_DIR}/include"
            "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_SOURCE_DIR}/src"
    )

    # ── Symbol visibility — only get_module exported (PHP extension ABI) ────────
    # All internal symbols are hidden; PHP's ZEND_DLEXPORT handles get_module.
    target_compile_options(${target} PRIVATE
            -fvisibility=hidden
    )

    # ── Compiler warnings ─────────────────────────────────────────────────────
    target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wno-long-long
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-unused-result
            -Wno-variadic-macros
            -Wno-format
            # -pthread is a no-op on macOS (pthreads always linked); Linux needs it
            $<$<NOT:$<PLATFORM_ID:Darwin>>:-pthread>
    )

    # ── Build-type flags via generator expressions (multi-config safe) ────────
    target_compile_options(${target} PRIVATE
            # -ggdb and -gdwarf-4 are GNU/Linux extensions; macOS uses DWARF natively
            $<$<AND:$<CONFIG:Debug>,$<NOT:$<PLATFORM_ID:Darwin>>>:-O0;-g;-ggdb;-g3;-gdwarf-4;-Wpedantic>
            $<$<AND:$<CONFIG:Debug>,$<PLATFORM_ID:Darwin>>:-O0;-g;-Wpedantic>
            $<$<AND:$<CONFIG:RelWithDebInfo>,$<NOT:$<PLATFORM_ID:Darwin>>>:-O2;-g;-ggdb;-g3;-gdwarf-4;-Wpedantic>
            $<$<AND:$<CONFIG:RelWithDebInfo>,$<PLATFORM_ID:Darwin>>:-O2;-g;-Wpedantic>
            $<$<CONFIG:Release>:-O3>
    )

    target_compile_definitions(${target} PRIVATE
            # _TIME_BITS/_FILE_OFFSET_BITS are glibc extensions; macOS uses 64-bit by default
            $<$<NOT:$<PLATFORM_ID:Darwin>>:_TIME_BITS=64>
            $<$<NOT:$<PLATFORM_ID:Darwin>>:_FILE_OFFSET_BITS=64>
            # PHP's zend_memrchr inlines glibc's memrchr() when HAVE_MEMRCHR is set
            # — that prototype is only exposed under _GNU_SOURCE on glibc.
            $<$<NOT:$<PLATFORM_ID:Darwin>>:_GNU_SOURCE>
            $<$<CONFIG:Debug>:DEBUG>
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:RELEASE>
    )

    # ── Platform-specific linker flags ────────────────────────────────────────
    target_link_options(${target} PRIVATE
            $<$<PLATFORM_ID:Darwin>:-undefined;dynamic_lookup>
    )

    # ── Sanitizers ────────────────────────────────────────────────────────────
    if (ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE -fno-inline -fno-omit-frame-pointer)
        add_sanitize_undefined(${target})
        add_sanitize_address(${target})
    endif ()

    # ── LTO ───────────────────────────────────────────────────────────────────
    if (ENABLE_LTO AND _PHP_SCYLLADB_LTO_SUPPORTED)
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
    endif ()

    # ── Architecture-specific SIMD ────────────────────────────────────────────
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        target_compile_options(${target} PRIVATE -msse2 -msse3 -msse4.1 -msse4.2)

        if (ENABLE_AVX AND _PHP_SCYLLADB_CC_SUPPORTS_AVX)
            target_compile_options(${target} PRIVATE -mavx)
        elseif (ENABLE_AVX AND NOT _PHP_SCYLLADB_CC_SUPPORTS_AVX)
            message(WARNING "${target}: ENABLE_AVX requested but compiler lacks -mavx")
        endif ()

        if (ENABLE_AVX2 AND _PHP_SCYLLADB_CC_SUPPORTS_AVX2)
            target_compile_options(${target} PRIVATE -mavx2)
        elseif (ENABLE_AVX2 AND NOT _PHP_SCYLLADB_CC_SUPPORTS_AVX2)
            message(WARNING "${target}: ENABLE_AVX2 requested but compiler lacks -mavx2")
        endif ()
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        # Apple Silicon / AArch64 — NEON is mandatory in ARMv8+, no flags needed.
        # ENABLE_AVX / ENABLE_AVX2 are meaningless here; silently ignore.
    endif ()

    # ── -march ────────────────────────────────────────────────────────────────
    # Skip on ARM/Apple Silicon — CPU_TYPE is "native" there by default which
    # is already the best choice; no -march x86 variants apply.
    if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        if (CPU_TYPE STREQUAL "native")
            message(WARNING
                    "-march=native produces binaries that may not run on other CPUs")
        endif ()

        check_c_compiler_flag("-march=${CPU_TYPE}" _PHP_SCYLLADB_CC_SUPPORTS_MARCH_${CPU_TYPE})
        if (_PHP_SCYLLADB_CC_SUPPORTS_MARCH_${CPU_TYPE})
            target_compile_options(${target} PRIVATE "-march=${CPU_TYPE}")
        else ()
            message(WARNING "${target}: compiler does not support -march=${CPU_TYPE}")
        endif ()
    endif ()
endfunction()

# ── php_scylladb_module ─────────────────────────────────────────────────────────
# Declare a static sub-library with its namespace alias and apply standard
# compiler settings. Add sources with target_sources() after this call.
#
# Usage: php_scylladb_module(<target> <namespace::alias>)
# Example: php_scylladb_module(datetime ext_scylladb::datetime)
macro(php_scylladb_module _target _alias)
    add_library(${_target} STATIC)
    add_library(${_alias} ALIAS ${_target})
    scylladb_php_library(${_target})
endmacro()
