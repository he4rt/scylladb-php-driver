include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

if (NOT PHP_CONFIG_FOUND)
    message(FATAL_ERROR "PHP::PHP requires PHPConfig to be found first")
endif ()

# ── Helper: run one php-config flag, fatal on failure ───────────────────────
macro(_php_config _flag _outvar)
    execute_process(
            COMMAND "${PHP_CONFIG_EXECUTABLE}" "${_flag}"
            OUTPUT_VARIABLE "${_outvar}"
            ERROR_VARIABLE  _php_cfg_err
            RESULT_VARIABLE _php_cfg_rc
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT _php_cfg_rc EQUAL 0)
        message(FATAL_ERROR
                "php-config ${_flag} failed (exit ${_php_cfg_rc}): ${_php_cfg_err}")
    endif ()
endmacro()

_php_config(--version       PHP_VERSION)
_php_config(--vernum        PHP_VERSION_NUM)
_php_config(--includes      _PHP_INCLUDES_RAW)
_php_config(--extension-dir PHP_EXTENSION_DIR)
_php_config(--include-dir   PHP_INCLUDE_DIR)
_php_config(--prefix        PHP_PREFIX)

# Convert "-I/path/a -I/path/b" → CMake list, handling paths with spaces
string(REGEX REPLACE "(^| )-I" ";" PHP_INCLUDES "${_PHP_INCLUDES_RAW}")
string(REGEX REPLACE "^;" "" PHP_INCLUDES "${PHP_INCLUDES}")
unset(_PHP_INCLUDES_RAW)

# ── Locate the PHP binary (sits next to php-config) ─────────────────────────
cmake_path(GET PHP_CONFIG_EXECUTABLE PARENT_PATH _php_config_dir)
find_program(PHP_BINARY
        NAMES php
        HINTS "${_php_config_dir}"
        NO_DEFAULT_PATH
        DOC "PHP interpreter binary"
)
if (NOT PHP_BINARY)
    find_program(PHP_BINARY NAMES php DOC "PHP interpreter binary")
endif ()
unset(_php_config_dir)

# ── PHP ini scan directory ───────────────────────────────────────────────────
if (PHP_BINARY)
    execute_process(
            COMMAND "${PHP_BINARY}" -r "echo PHP_CONFIG_FILE_SCAN_DIR;"
            OUTPUT_VARIABLE PHP_INI_SCAN_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE _rc
            ERROR_QUIET
    )
    if (NOT _rc EQUAL 0)
        set(PHP_INI_SCAN_DIR "")
    endif ()
    unset(_rc)
endif ()

# ── Relative extension dir (for DESTDIR-safe installs) ──────────────────────
# cmake_path returns the original path when it cannot be made relative,
# so this is safe even when PHP_EXTENSION_DIR is on a different root.
cmake_path(RELATIVE_PATH PHP_EXTENSION_DIR
        BASE_DIRECTORY "${PHP_PREFIX}"
        OUTPUT_VARIABLE PHP_EXTENSION_RELATIVE_DIR
)

if (NOT IS_ABSOLUTE "${PHP_EXTENSION_RELATIVE_DIR}")
    # Successfully made relative — expose as cache var so the user can see it
    set(PHP_EXTENSION_RELATIVE_DIR "${PHP_EXTENSION_RELATIVE_DIR}" CACHE INTERNAL
            "Extension dir relative to PHP_PREFIX (used for DESTDIR installs)")
else ()
    # Could not relativise (different drive on Windows, etc.) — fall back to absolute
    set(PHP_EXTENSION_RELATIVE_DIR "${PHP_EXTENSION_DIR}" CACHE INTERNAL "")
endif ()

# ── Create PHP::PHP INTERFACE IMPORTED target ────────────────────────────────
if (NOT TARGET PHP::PHP)
    add_library(PHP::PHP INTERFACE IMPORTED GLOBAL)
    target_include_directories(PHP::PHP INTERFACE ${PHP_INCLUDES})
endif ()

find_package_handle_standard_args(PHP
        REQUIRED_VARS PHP_CONFIG_EXECUTABLE PHP_EXTENSION_DIR PHP_INCLUDES PHP_PREFIX
        VERSION_VAR   PHP_VERSION
)

message(STATUS "PHP ${PHP_VERSION} | ext-dir: ${PHP_EXTENSION_DIR}")
if (PHP_INI_SCAN_DIR)
    message(STATUS "PHP ini scan: ${PHP_INI_SCAN_DIR}")
endif ()
