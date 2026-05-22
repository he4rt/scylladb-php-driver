# ──────────────────────────────────────────────────────────────────────────────
# GenStubs.cmake — build-time generation of *_arginfo.h from *.stub.php
#
# The repo only stores .stub.php files. _arginfo.h is generated at configure /
# build time by the PHP that the extension is being compiled against, using
# the OFFICIAL `build/gen_stub.php` shipped with that PHP install. We do not
# vendor gen_stub.php — picking it up from the active PHP means each PHP
# version's tool decides what API/features to emit.
#
# A tiny wrapper (tools/gen_stub/gen_arginfo.sh) handles two project-specific
# fixups around upstream gen_stub:
#   1. Strip `declare(strict_types=1);` before parsing (upstream throws on it).
#   2. Inject `(zend_function *)` cast on `zend_add_*_attribute(zend_hash_str_find_ptr(...))`
#      calls so the emitted code compiles as C++.
#
# Public surface:
#   php_scylladb_generate_arginfo(<target> <stub1.stub.php> [stub2 ...])
# ──────────────────────────────────────────────────────────────────────────────

if (NOT PHP_BINARY)
    message(FATAL_ERROR "PHP_BINARY not set — FindPHP must run before GenStubs")
endif ()

# ── Locate gen_stub.php from the active PHP install ──────────────────────────
# Standard install layout: ${PHP_PREFIX}/lib/php/build/gen_stub.php
# PHP source-tree layout : ${PHP_PREFIX}/src/build/gen_stub.php (compile-php.sh)
# Optional override      : -DPHP_SCYLLADB_GEN_STUB_SCRIPT=/abs/path/gen_stub.php

if (NOT DEFINED PHP_SCYLLADB_GEN_STUB_SCRIPT OR PHP_SCYLLADB_GEN_STUB_SCRIPT STREQUAL "")
    # First try: ask php-config for the prefix and look in the install tree.
    execute_process(
            COMMAND "${PHP_CONFIG_EXECUTABLE}" --prefix
            OUTPUT_VARIABLE _php_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    find_file(PHP_SCYLLADB_GEN_STUB_SCRIPT
            NAMES gen_stub.php
            PATHS
              "${_php_prefix}/lib/php/build"
              "${_php_prefix}/share/php/build"
              "${_php_prefix}/src/build"
            NO_DEFAULT_PATH
            DOC  "Path to the PHP build/gen_stub.php script"
    )
endif ()

# ── Fallback: download gen_stub.php from php-src for the exact PHP version ──
# Useful in CI environments (e.g. setup-php) that install the PHP runtime
# without the build/ directory. We map PHP_VERSION_NUM → branch name
# (PHP-8.3, PHP-8.4, …) and pull the matching gen_stub.php from the
# canonical php-src GitHub raw URL into the binary dir, where it stays
# valid across builds (cached by CMake's file(DOWNLOAD) idempotency check).
if (NOT PHP_SCYLLADB_GEN_STUB_SCRIPT OR PHP_SCYLLADB_GEN_STUB_SCRIPT STREQUAL "PHP_SCYLLADB_GEN_STUB_SCRIPT-NOTFOUND" OR NOT EXISTS "${PHP_SCYLLADB_GEN_STUB_SCRIPT}")
    if (NOT PHP_VERSION_NUM)
        execute_process(
                COMMAND "${PHP_CONFIG_EXECUTABLE}" --vernum
                OUTPUT_VARIABLE PHP_VERSION_NUM
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()

    # PHP_VERSION_NUM is e.g. 80305 for 8.3.5; we only need MAJOR.MINOR.
    math(EXPR _php_major "${PHP_VERSION_NUM} / 10000")
    math(EXPR _php_minor "(${PHP_VERSION_NUM} / 100) % 100")
    set(_branch "PHP-${_php_major}.${_php_minor}")

    set(_cached "${PROJECT_BINARY_DIR}/_deps/gen_stub-${_php_major}.${_php_minor}.php")
    if (NOT EXISTS "${_cached}")
        set(_url "https://raw.githubusercontent.com/php/php-src/${_branch}/build/gen_stub.php")
        message(STATUS "Downloading gen_stub.php for PHP ${_php_major}.${_php_minor} from ${_url}")
        file(DOWNLOAD "${_url}" "${_cached}"
                STATUS  _dl_status
                TIMEOUT 30
                TLS_VERIFY ON
        )
        list(GET _dl_status 0 _dl_rc)
        if (NOT _dl_rc EQUAL 0)
            list(GET _dl_status 1 _dl_msg)
            file(REMOVE "${_cached}")
            message(FATAL_ERROR
                    "Failed to download gen_stub.php from ${_url}: ${_dl_msg}\n"
                    "Install PHP build tools (`php-dev` / `php-devel`) or pass "
                    "-DPHP_SCYLLADB_GEN_STUB_SCRIPT=/path/to/gen_stub.php.")
        endif ()
    endif ()
    set(PHP_SCYLLADB_GEN_STUB_SCRIPT "${_cached}" CACHE FILEPATH
            "Path to the PHP build/gen_stub.php script" FORCE)
endif ()

if (NOT EXISTS "${PHP_SCYLLADB_GEN_STUB_SCRIPT}")
    message(FATAL_ERROR "gen_stub.php still not found after fallback attempts.")
endif ()

message(STATUS "Using gen_stub.php at ${PHP_SCYLLADB_GEN_STUB_SCRIPT}")

set(PHP_SCYLLADB_GEN_STUB_WRAPPER "${PROJECT_SOURCE_DIR}/tools/gen_stub/gen_arginfo.sh")

if (NOT EXISTS "${PHP_SCYLLADB_GEN_STUB_WRAPPER}")
    message(FATAL_ERROR "wrapper not found: ${PHP_SCYLLADB_GEN_STUB_WRAPPER}")
endif ()

# Generator that emits <basename>_descriptor.cpp alongside each <basename>_arginfo.h.
# The descriptor.cpp wires the class into the self-registering descriptor system
# (see src/Registry/) — global ce, handlers struct, register fn (with weakly
# referenced create_object/free/properties/etc. callbacks), and the
# PHP_SCYLLADB_REGISTER_CLASS macro invocation.
set(PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT "${PROJECT_SOURCE_DIR}/tools/gen_descriptor/gen_class_descriptor.php")
if (NOT EXISTS "${PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT}")
    message(FATAL_ERROR "descriptor generator not found: ${PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT}")
endif ()

function(php_scylladb_generate_arginfo target_name)
    set(_generated_headers)
    foreach (_stub IN LISTS ARGN)
        # "Builder.stub.php" → "Builder_arginfo.h"
        string(REGEX REPLACE "\\.stub\\.php$" "_arginfo.h" _arginfo "${_stub}")

        set(_stub_path    "${CMAKE_CURRENT_SOURCE_DIR}/${_stub}")
        set(_arginfo_path "${CMAKE_CURRENT_SOURCE_DIR}/${_arginfo}")

        if (NOT EXISTS "${_stub_path}")
            message(FATAL_ERROR "stub file not found: ${_stub_path}")
        endif ()

        add_custom_command(
                OUTPUT  "${_arginfo_path}"
                COMMAND "${PHP_SCYLLADB_GEN_STUB_WRAPPER}"
                        "${_stub_path}"
                        "${PHP_BINARY}"
                        "${PHP_SCYLLADB_GEN_STUB_SCRIPT}"
                DEPENDS "${_stub_path}"
                        "${PHP_SCYLLADB_GEN_STUB_SCRIPT}"
                        "${PHP_SCYLLADB_GEN_STUB_WRAPPER}"
                COMMENT "Generating ${_arginfo} from ${_stub}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                VERBATIM
        )

        list(APPEND _generated_headers "${_arginfo_path}")
    endforeach ()

    target_sources(${target_name} PRIVATE ${_generated_headers})
endfunction()

# Opt-in: generate <basename>_descriptor.cpp for each stub passed here.
# The descriptor.cpp wires the class into the self-registering descriptor
# system (see src/Registry/) — global ce, handlers struct, register fn (with
# weakly referenced create_object/free/properties/etc. callbacks), and the
# PHP_SCYLLADB_REGISTER_CLASS macro invocation.
#
# Caller must ensure the companion .cpp does NOT also declare the ce global,
# the handlers struct, the register fn, or call the descriptor macro — those
# now come from the generated file.
function(php_scylladb_generate_descriptor target_name)
    set(_generated_descriptors)
    foreach (_stub IN LISTS ARGN)
        string(REGEX REPLACE "\\.stub\\.php$" "_arginfo.h"      _arginfo    "${_stub}")
        string(REGEX REPLACE "\\.stub\\.php$" "_descriptor.cpp" _descriptor "${_stub}")
        get_filename_component(_arginfo_name "${_arginfo}" NAME)

        set(_stub_path       "${CMAKE_CURRENT_SOURCE_DIR}/${_stub}")
        set(_descriptor_path "${CMAKE_CURRENT_SOURCE_DIR}/${_descriptor}")

        if (NOT EXISTS "${_stub_path}")
            message(FATAL_ERROR "stub file not found: ${_stub_path}")
        endif ()

        add_custom_command(
                OUTPUT  "${_descriptor_path}"
                COMMAND "${PHP_BINARY}"
                        "${PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT}"
                        "${_stub_path}"
                        "${_descriptor_path}"
                        "${_arginfo_name}"
                DEPENDS "${_stub_path}"
                        "${PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT}"
                COMMENT "Generating ${_descriptor} from ${_stub}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                VERBATIM
        )

        list(APPEND _generated_descriptors "${_descriptor_path}")
    endforeach ()

    target_sources(${target_name} PRIVATE ${_generated_descriptors})
endfunction()
