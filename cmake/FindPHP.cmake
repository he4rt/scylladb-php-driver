if (NOT PHP_CONFIG_FOUND)
    message(FATAL_ERROR "php-config not found — run FindPHPConfig first")
endif ()

macro(_php_exec _flag _outvar)
    execute_process(
            COMMAND "${PHP_CONFIG_EXECUTABLE}" "${_flag}"
            OUTPUT_VARIABLE "${_outvar}"
            ERROR_VARIABLE  _php_exec_err
            RESULT_VARIABLE _php_exec_rc
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT _php_exec_rc EQUAL 0)
        message(FATAL_ERROR "php-config ${_flag} failed: ${_php_exec_err}")
    endif ()
endmacro()

_php_exec(--prefix        PHP_LIB_PREFIX)
_php_exec(--includes      _PHP_INCLUDES_RAW)
_php_exec(--libs          PHP_LIBS)
_php_exec(--version       PHP_VERSION)
_php_exec(--vernum        ZEND_API)
_php_exec(--extension-dir PHP_EXTENSION_DIR)
_php_exec(--include-dir   PHP_INCLUDES_DIR)

# Convert "-I/path/a -I/path/b" into a CMake list
string(REGEX REPLACE "(^| )-I" ";" PHP_INCLUDES "${_PHP_INCLUDES_RAW}")
string(REGEX REPLACE "^;" "" PHP_INCLUDES "${PHP_INCLUDES}")   # strip leading ;
unset(_PHP_INCLUDES_RAW)

get_filename_component(PHP_LIB_DIR "${PHP_LIBS}" DIRECTORY)

set(PHP_FOUND ON)

message(STATUS "Found PHP ${PHP_VERSION} (${PHP_CONFIG_EXECUTABLE})")
message(STATUS "  Extension dir: ${PHP_EXTENSION_DIR}")
message(STATUS "  Includes: ${PHP_INCLUDES_DIR}")
