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
#   php_driver_generate_arginfo(<target> <stub1.stub.php> [stub2 ...])
# ──────────────────────────────────────────────────────────────────────────────

if (NOT PHP_BINARY)
    message(FATAL_ERROR "PHP_BINARY not set — FindPHP must run before GenStubs")
endif ()

# ── Locate gen_stub.php from the active PHP install ──────────────────────────
# Standard install layout: ${PHP_PREFIX}/lib/php/build/gen_stub.php
# PHP source-tree layout : ${PHP_PREFIX}/src/build/gen_stub.php (compile-php.sh)
# Optional override      : -DPHP_DRIVER_GEN_STUB_SCRIPT=/abs/path/gen_stub.php

if (NOT DEFINED PHP_DRIVER_GEN_STUB_SCRIPT OR PHP_DRIVER_GEN_STUB_SCRIPT STREQUAL "")
    # Always ask php-config for the prefix — works regardless of how
    # FindPHP scoped its variables.
    execute_process(
            COMMAND "${PHP_CONFIG_EXECUTABLE}" --prefix
            OUTPUT_VARIABLE _php_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    find_file(PHP_DRIVER_GEN_STUB_SCRIPT
            NAMES gen_stub.php
            PATHS
              "${_php_prefix}/lib/php/build"
              "${_php_prefix}/share/php/build"
              "${_php_prefix}/src/build"
            NO_DEFAULT_PATH
            DOC  "Path to the PHP build/gen_stub.php script"
    )
endif ()

if (NOT PHP_DRIVER_GEN_STUB_SCRIPT OR PHP_DRIVER_GEN_STUB_SCRIPT STREQUAL "PHP_DRIVER_GEN_STUB_SCRIPT-NOTFOUND" OR NOT EXISTS "${PHP_DRIVER_GEN_STUB_SCRIPT}")
    message(FATAL_ERROR
            "gen_stub.php not found under the active PHP install (${_php_prefix}).\n"
            "Install the PHP development build tools (e.g. `php-dev` on Debian / "
            "Ubuntu) or pass -DPHP_DRIVER_GEN_STUB_SCRIPT=/path/to/gen_stub.php.")
endif ()

message(STATUS "Using gen_stub.php at ${PHP_DRIVER_GEN_STUB_SCRIPT}")

set(PHP_DRIVER_GEN_STUB_WRAPPER "${PROJECT_SOURCE_DIR}/tools/gen_stub/gen_arginfo.sh")

if (NOT EXISTS "${PHP_DRIVER_GEN_STUB_WRAPPER}")
    message(FATAL_ERROR "wrapper not found: ${PHP_DRIVER_GEN_STUB_WRAPPER}")
endif ()

function(php_driver_generate_arginfo target_name)
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
                COMMAND "${PHP_DRIVER_GEN_STUB_WRAPPER}"
                        "${_stub_path}"
                        "${PHP_BINARY}"
                        "${PHP_DRIVER_GEN_STUB_SCRIPT}"
                DEPENDS "${_stub_path}"
                        "${PHP_DRIVER_GEN_STUB_SCRIPT}"
                        "${PHP_DRIVER_GEN_STUB_WRAPPER}"
                COMMENT "Generating ${_arginfo} from ${_stub}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                VERBATIM
        )

        list(APPEND _generated_headers "${_arginfo_path}")
    endforeach ()

    target_sources(${target_name} PRIVATE ${_generated_headers})
endfunction()
