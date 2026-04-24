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

# ── Create PHP::PHP INTERFACE IMPORTED target ────────────────────────────────
if (NOT TARGET PHP::PHP)
    add_library(PHP::PHP INTERFACE IMPORTED GLOBAL)
    target_include_directories(PHP::PHP INTERFACE ${PHP_INCLUDES})
endif ()

find_package_handle_standard_args(PHP
        REQUIRED_VARS PHP_CONFIG_EXECUTABLE PHP_EXTENSION_DIR PHP_INCLUDES
        VERSION_VAR   PHP_VERSION
)
