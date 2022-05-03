cmake_minimum_required(VERSION 3.18)

project(libphpcassandra LANGUAGES C)

# find_package(Corrosion REQUIRED)

set(LIBUV_ROOT_DIR ${PROJECT_SOURCE_DIR}/lib/libuv)

set(PHP_CASSANDRA_DRIVER_VERSION 1.3.7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 -Wall -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=gnu17 -Wall")
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Release)
set(PHP_INI_DIR /usr/local/lib/php)

option(LINK_STATICALLY "Links LibCassandra and LibUV statically" ON)

include_directories(${PROJECT_SOURCE_DIR}/lib/cpp-driver/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/libuv/include)
include_directories(${PROJECT_SOURCE_DIR}/ext/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libuv)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/cpp-driver)

if(${LINK_STATICALLY})
    set(CASSANDRA_LINK_LIBRARIES cassandra_static uv_a)
else()
    set(CASSANDRA_LINK_LIBRARIES cassandra uv)
endif()
execute_process(COMMAND phpize WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/ext)
execute_process(COMMAND ./configure WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/ext)

# file(READ ./config.h CASSANDRA_CONFIG_FILE)

add_definitions(-DHAVE_CONFIG_H)

execute_process(COMMAND php-config --includes OUTPUT_VARIABLE PHP_INCLUDES OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND php-config --extension-dir OUTPUT_VARIABLE PHP_EXTENSION_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PHP_INCLUDES}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PHP_INCLUDES}")

file(GLOB_RECURSE HEAD_FILES FOLLOW_SYMLINKS ext/include/*.h)
file(GLOB_RECURSE SRC_LIST FOLLOW_SYMLINKS ext/src/*.c)
file(GLOB_RECURSE UTILS_LIST FOLLOW_SYMLINKS ext/util/*.c)

SET(LIBRARY_OUTPUT_PATH lib)

link_directories(${LIBRARY_OUTPUT_PATH})
include_directories(BEFORE ext/include ext)

set(php_cassandra_sources ${UTILS_LIST} ${SRC_LIST} ext/php_driver.c)


add_library(ext-cassandra SHARED ${php_cassandra_sources})


#corrosion_import_crate(
#    MANIFEST_PATH utils/Cargo.toml
#    PROFILE release
#)

set_target_properties(ext-cassandra PROPERTIES PREFIX "")
set_target_properties(ext-cassandra PROPERTIES OUTPUT_NAME "cassandra")

target_link_libraries(ext-cassandra ${CASSANDRA_LINK_LIBRARIES})
# target_link_libraries(ext-cassandra cassandra)
# target_link_libraries(ext-cassandra libuv)
# target_link_libraries(ext-cassandra cpp-driver)

install(TARGETS ext-cassandra LIBRARY DESTINATION ${PHP_EXTENSION_DIR})
install(FILES ${HEAD_FILES} DESTINATION include/cassandra)
install(FILES ext/cassandra.ini DESTINATION ${PHP_INI_DIR})
