# ──────────────────────────────────────────────────────────────────────────────
# GenStubs.cmake — build-time generation of *_arginfo.h from *.stub.php
#
# The repo only stores .stub.php files. _arginfo.h is generated at configure /
# build time by the PHP that the extension is being compiled against, via the
# vendored tools/gen_stub/gen_stub.php (sourced from PHP 8.5's lib/php/build/).
# Each PHP version's gen_stub emits the API appropriate for that version (with
# PHP_VERSION_ID guards for forward-compat).
#
# Public surface:
#   php_driver_generate_arginfo(<target> <stub1.stub.php> [stub2 ...])
#       For each given stub file:
#         * Adds a custom command that runs gen_stub.php against the source-tree
#           copy, producing X_arginfo.h alongside it. The generated header is
#           gitignored.
#         * Marks the header as a generated dependency of <target> so any .cpp
#           that #includes it triggers re-generation on stub edit.
# ──────────────────────────────────────────────────────────────────────────────

set(PHP_DRIVER_GEN_STUB_SCRIPT "${PROJECT_SOURCE_DIR}/tools/gen_stub/gen_stub.php"
    CACHE FILEPATH "Path to gen_stub.php")

if (NOT EXISTS "${PHP_DRIVER_GEN_STUB_SCRIPT}")
    message(FATAL_ERROR "gen_stub.php not found at ${PHP_DRIVER_GEN_STUB_SCRIPT}")
endif ()

if (NOT PHP_BINARY)
    message(FATAL_ERROR "PHP_BINARY not set — FindPHP must run before GenStubs")
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
                COMMAND "${PHP_BINARY}" "${PHP_DRIVER_GEN_STUB_SCRIPT}" "${_stub_path}"
                DEPENDS "${_stub_path}" "${PHP_DRIVER_GEN_STUB_SCRIPT}"
                COMMENT "Generating ${_arginfo} from ${_stub}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                VERBATIM
        )

        list(APPEND _generated_headers "${_arginfo_path}")
    endforeach ()

    # Wire the generated headers into the target so consumer compilation
    # waits for them. They aren't compiled directly (headers), but listing
    # them in target_sources is the canonical way to declare the dependency.
    target_sources(${target_name} PRIVATE ${_generated_headers})
endfunction()
