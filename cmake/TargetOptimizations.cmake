include(CheckIPOSupported)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

function(scylladb_php_library target enable_sanitizers cpu_type lto)
    target_include_directories(
            ${target}
            PUBLIC
            ${PHP_INCLUDES}
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}
            ${LIBGMP_INCLUDE_DIRS}
            ${CASSANDRA_H}
    )

    target_compile_features(${target} PUBLIC cxx_std_20 c_std_23)

    set_target_properties(${target} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            C_STANDARD 23
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS ON
    )

    target_compile_options(
            ${target}
            PRIVATE
            -fPIC
            -Wall
            -Wextra
            -Wno-long-long
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-unused-result
            -Wno-variadic-macros
            -Wno-format
            -pthread
    )

    target_compile_definitions(${target} PRIVATE
            _TIME_BITS=64
            _FILE_OFFSET_BITS=64
    )

    if (enable_sanitizers)
        target_compile_options(${target} PRIVATE -fno-inline -fno-omit-frame-pointer)
        add_sanitize_undefined(${target})
        add_sanitize_address(${target})
    endif ()

    if (APPLE)
        target_link_options(${target} PRIVATE -undefined dynamic_lookup)
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${target} PRIVATE -O0 -g -ggdb -g3 -gdwarf-4 -Wpedantic)
        target_compile_definitions(${target} PRIVATE DEBUG)
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(${target} PRIVATE -O2 -g -ggdb -g3 -gdwarf-4 -Wpedantic)
        target_compile_definitions(${target} PRIVATE RELEASE)
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${target} PRIVATE -O3)
        target_compile_definitions(${target} PRIVATE RELEASE)
    endif ()

    if (lto)
        check_ipo_supported(RESULT LTO_SUPPORTED)
        if (LTO_SUPPORTED)
            message(STATUS "LTO is supported and enabled")
            set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
        else ()
            message(STATUS "LTO is not supported")
        endif ()
    endif ()

    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        target_compile_options(${target} PRIVATE -msse2 -msse3 -msse4.1 -msse4.2)
        if (ENABLE_AVX)
            check_c_compiler_flag(-mavx SUPPORTS_AVX)
            if (SUPPORTS_AVX)
                target_compile_options(${target} PRIVATE -mavx)
            else ()
                message(WARNING "Compiler does not support AVX")
            endif ()
        endif ()

        if (ENABLE_AVX2)
            check_c_compiler_flag(-mavx2 SUPPORTS_AVX2)
            if (SUPPORTS_AVX2)
                target_compile_options(${target} PRIVATE -mavx2)
            else ()
                message(WARNING "Compiler does not support AVX2")
            endif ()
        endif ()
    else ()
        message(STATUS "Architecture is ${CMAKE_SYSTEM_PROCESSOR}")
    endif ()

    if (cpu_type STREQUAL "native")
        message(WARNING "Be careful when using -march=native, it may cause problems when running on different CPUs")
    endif ()

    check_c_compiler_flag("-march=${cpu_type}" SUPPORT_MARCH)
    if (SUPPORT_MARCH)
        target_compile_options(${target} PRIVATE "-march=${cpu_type}")
    else ()
        message(WARNING "Compiler does not support -march=${cpu_type}")
    endif ()
endfunction()

# Convenience macro: declares a static sub-library, its namespace alias,
# and applies standard compiler flags. Sources must still be added via
# target_sources() after calling this macro.
#
# Usage: php_driver_module(<target> <alias>)
# Example: php_driver_module(datetime ext_scylladb::datetime)
macro(php_driver_module _target _alias)
    add_library(${_target} STATIC)
    add_library(${_alias} ALIAS ${_target})
    scylladb_php_library(${_target} "${ENABLE_SANITIZERS}" "${CPU_TYPE}" "${ENABLE_LTO}")
endmacro()
