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

if (NOT PHP_VERSION_NUM)
    execute_process(
            COMMAND "${PHP_CONFIG_EXECUTABLE}" --vernum
            OUTPUT_VARIABLE PHP_VERSION_NUM
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()

math(EXPR _php_major "${PHP_VERSION_NUM} / 10000")
math(EXPR _php_minor "(${PHP_VERSION_NUM} / 100) % 100")

set(_gen_stub_staged "${PROJECT_BINARY_DIR}/_deps/gen_stub-${_php_major}.${_php_minor}.php")

if (NOT EXISTS "${_gen_stub_staged}")
    set(_gen_stub_origin "")

    if (DEFINED PHP_SCYLLADB_GEN_STUB_SCRIPT AND NOT PHP_SCYLLADB_GEN_STUB_SCRIPT STREQUAL ""
            AND EXISTS "${PHP_SCYLLADB_GEN_STUB_SCRIPT}")
        set(_gen_stub_origin "${PHP_SCYLLADB_GEN_STUB_SCRIPT}")
    else ()
        execute_process(
                COMMAND "${PHP_CONFIG_EXECUTABLE}" --prefix
                OUTPUT_VARIABLE _php_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        file(GLOB _php_api_build_dirs "${_php_prefix}/lib/php/*/build")

        find_file(_gen_stub_found
                NAMES gen_stub.php
                PATHS
                  "${_php_prefix}/lib/php/build"
                  "${_php_prefix}/share/php/build"
                  "${_php_prefix}/src/build"
                  "${PHP_EXTENSION_DIR}/build"
                  ${_php_api_build_dirs}
                NO_DEFAULT_PATH
                DOC  "gen_stub.php discovered in the PHP install"
        )

        if (_gen_stub_found AND EXISTS "${_gen_stub_found}")
            set(_gen_stub_origin "${_gen_stub_found}")
        endif ()
    endif ()

    if (_gen_stub_origin STREQUAL "")
        set(_branch "PHP-${_php_major}.${_php_minor}")
        set(_url "https://raw.githubusercontent.com/php/php-src/${_branch}/build/gen_stub.php")
        message(STATUS "Downloading gen_stub.php for PHP ${_php_major}.${_php_minor} from ${_url}")
        file(DOWNLOAD "${_url}" "${_gen_stub_staged}"
                STATUS  _dl_status
                TIMEOUT 30
                TLS_VERIFY ON
        )
        list(GET _dl_status 0 _dl_rc)
        if (NOT _dl_rc EQUAL 0)
            list(GET _dl_status 1 _dl_msg)
            file(REMOVE "${_gen_stub_staged}")
            message(FATAL_ERROR
                    "Failed to download gen_stub.php from ${_url}: ${_dl_msg}\n"
                    "Install PHP build tools (`php-dev` / `php-devel`) or pass "
                    "-DPHP_SCYLLADB_GEN_STUB_SCRIPT=/path/to/gen_stub.php.")
        endif ()
    else ()
        message(STATUS "Staging gen_stub.php from ${_gen_stub_origin}")
        configure_file("${_gen_stub_origin}" "${_gen_stub_staged}" COPYONLY)
    endif ()
endif ()

set(PHP_SCYLLADB_GEN_STUB_SCRIPT "${_gen_stub_staged}" CACHE FILEPATH
        "Path to the PHP build/gen_stub.php script" FORCE)

if (NOT EXISTS "${PHP_SCYLLADB_GEN_STUB_SCRIPT}")
    message(FATAL_ERROR "gen_stub.php still not found after fallback attempts.")
endif ()

message(STATUS "Using gen_stub.php at ${PHP_SCYLLADB_GEN_STUB_SCRIPT}")

set(PHP_SCYLLADB_GEN_STUB_WRAPPER "${PROJECT_SOURCE_DIR}/tools/gen_stub/gen_arginfo.sh")

if (NOT EXISTS "${PHP_SCYLLADB_GEN_STUB_WRAPPER}")
    message(FATAL_ERROR "wrapper not found: ${PHP_SCYLLADB_GEN_STUB_WRAPPER}")
endif ()

# Serialize gen_stub invocations through a single-slot job pool (Ninja).
# gen_stub.php downloads PHP-Parser on first run; with parallel ninja builds,
# multiple gen_stub instances race on that download and one of them fails with
# "PHP-Parser-5.6.1/lib/PhpParser/Parser/Php7.php: No such file or directory".
# A 1-slot pool makes the first invocation finish (and complete the download)
# before any sibling gen_stub starts.
set_property(GLOBAL APPEND PROPERTY JOB_POOLS php_scylladb_gen_stub=1)

# Generator that emits <basename>_descriptor.c alongside each <basename>_arginfo.h.
# The descriptor.c wires the class into the self-registering descriptor system
# (see src/Registry/) — global ce, handlers struct, register fn (with weakly
# referenced create_object/free/properties/etc. callbacks), and the
# PHP_SCYLLADB_REGISTER_CLASS macro invocation.
set(PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT "${PROJECT_SOURCE_DIR}/tools/gen_descriptor/gen_class_descriptor.php")
if (NOT EXISTS "${PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT}")
    message(FATAL_ERROR "descriptor generator not found: ${PHP_SCYLLADB_GEN_DESCRIPTOR_SCRIPT}")
endif ()

# Aggregate target that materialises every generated header and source without
# compiling anything. clang-tidy needs the generated files to exist before it
# can parse a translation unit, and it must not depend on a list of module
# targets that a new module can be left out of.
#
#   cmake --build <dir> --target generated-sources
if (NOT TARGET generated-sources)
    add_custom_target(generated-sources)
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
                JOB_POOL php_scylladb_gen_stub
                VERBATIM
        )

        list(APPEND _generated_headers "${_arginfo_path}")
    endforeach ()

    target_sources(${target_name} PRIVATE ${_generated_headers})

    add_custom_target(${target_name}-arginfo DEPENDS ${_generated_headers})
    add_dependencies(generated-sources ${target_name}-arginfo)
endfunction()

# Opt-in: generate <basename>_descriptor.c for each stub passed here.
# The descriptor.c wires the class into the self-registering descriptor
# system (see src/Registry/) — global ce, handlers struct, register fn (with
# weakly referenced create_object/free/properties/etc. callbacks), and the
# PHP_SCYLLADB_REGISTER_CLASS macro invocation.
#
# Caller must ensure the companion .c does NOT also declare the ce global,
# the handlers struct, the register fn, or call the descriptor macro — those
# now come from the generated file.
function(php_scylladb_generate_descriptor target_name)
    set(_generated_descriptors)
    foreach (_stub IN LISTS ARGN)
        string(REGEX REPLACE "\\.stub\\.php$" "_arginfo.h"      _arginfo    "${_stub}")
        string(REGEX REPLACE "\\.stub\\.php$" "_descriptor.c" _descriptor "${_stub}")
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

    add_custom_target(${target_name}-descriptors DEPENDS ${_generated_descriptors})
    add_dependencies(generated-sources ${target_name}-descriptors)
endfunction()
