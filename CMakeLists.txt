cmake_minimum_required(VERSION 3.18)

project(libphpcassandra LANGUAGES C)

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(LIBUV_ROOT_DIR ${PROJECT_SOURCE_DIR}/lib/libuv)
set(PHP_CASSANDRA_DRIVER_VERSION 2.0.0)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 -Wall -Ofast -flto -fstrict-aliasing")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=gnu17 -Wall -Ofast -flto -fstrict-aliasing")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 -Wall -O0 -fstrict-aliasing")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=gnu17 -Wall -O0 -fstrict-aliasing")
endif ()


message("C FLAGS ${CMAKE_C_FLAGS}")
message("CXX FLAGS ${CMAKE_CXX_FLAGS}")

set(PHP_INI_DIR /usr/local/lib/php)

option(LINK_STATICALLY "Links LibCassandra and LibUV statically" ON)

include_directories(${PROJECT_SOURCE_DIR}/lib/cpp-driver/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/libuv/include)
include_directories(${PROJECT_SOURCE_DIR}/ext/include)


find_library(GMP_LIBRARY NAMES gmp)

if (${LINK_STATICALLY})
    set(CASSANDRA_LINK_LIBRARIES cassandra_static uv_a m gmp)
else ()
    set(CASSANDRA_LINK_LIBRARIES cassandra uv m.a gmp.a)
endif ()

execute_process(COMMAND phpize WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/ext)

add_definitions(-DHAVE_CONFIG_H)

execute_process(COMMAND php-config --includes OUTPUT_VARIABLE PHP_INCLUDES OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND php-config --extension-dir OUTPUT_VARIABLE PHP_EXTENSION_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PHP_INCLUDES}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PHP_INCLUDES}")

file(GLOB_RECURSE HEAD_FILES FOLLOW_SYMLINKS ext/include/*.h)
file(GLOB_RECURSE SRC_LIST FOLLOW_SYMLINKS ext/src/*.c)
file(GLOB_RECURSE UTILS_LIST FOLLOW_SYMLINKS ext/util/*.c)

include_directories(BEFORE ext/include ext lib/cpp-driver/include lib/libuv/include)

set(php_cassandra_sources ${UTILS_LIST} ${SRC_LIST} ext/php_driver.c)

add_library(ext-cassandra SHARED ${php_cassandra_sources})


#corrosion_import_crate(
#    MANIFEST_PATH utils/Cargo.toml
#    PROFILE release
#)

set_target_properties(ext-cassandra PROPERTIES PREFIX "")
set_target_properties(ext-cassandra PROPERTIES OUTPUT_NAME "cassandra")

target_link_libraries(ext-cassandra ${CASSANDRA_LINK_LIBRARIES})

install(TARGETS ext-cassandra LIBRARY DESTINATION ${PHP_EXTENSION_DIR})
install(FILES ${HEAD_FILES} DESTINATION include/cassandra)
install(FILES ext/cassandra.ini DESTINATION ${PHP_INI_DIR})
