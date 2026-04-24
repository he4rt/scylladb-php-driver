include_guard(GLOBAL)

if (NOT CUSTOM_PHP_CONFIG STREQUAL "")
    message(STATUS "Using custom php-config: ${CUSTOM_PHP_CONFIG}")
    set(PHP_CONFIG_EXECUTABLE "${CUSTOM_PHP_CONFIG}" CACHE FILEPATH "php-config binary" FORCE)
else ()
    if (PHP_VERSION_FOR_PHP_CONFIG STREQUAL "")
        message(FATAL_ERROR "PHP_VERSION_FOR_PHP_CONFIG is not set")
    endif ()

    set(_php_hint "${PROJECT_SOURCE_DIR}/php/${PHP_VERSION_FOR_PHP_CONFIG}")
    if (PHP_DEBUG)
        string(APPEND _php_hint "-debug")
    else ()
        string(APPEND _php_hint "-release")
    endif ()
    if (PHP_THREAD_SAFE)
        string(APPEND _php_hint "-zts")
    else ()
        string(APPEND _php_hint "-nts")
    endif ()

    find_program(PHP_CONFIG_EXECUTABLE
            NAMES php-config
                  php-config${PHP_VERSION_FOR_PHP_CONFIG}
            HINTS
                "${_php_hint}"
                # macOS Homebrew (ARM: /opt/homebrew, Intel: /usr/local)
                "/opt/homebrew/opt/php@${PHP_VERSION_FOR_PHP_CONFIG}"
                "/opt/homebrew/opt/php"
                "/opt/homebrew"
                "/usr/local/opt/php@${PHP_VERSION_FOR_PHP_CONFIG}"
                "/usr/local/opt/php"
                "/usr/local"
                "/usr"
                "${PROJECT_SOURCE_DIR}/php"
            PATH_SUFFIXES bin
            DOC "Path to php-config binary"
            REQUIRED
    )
    unset(_php_hint)
endif ()

message(STATUS "Found php-config: ${PHP_CONFIG_EXECUTABLE}")
set(PHP_CONFIG_FOUND ON)
