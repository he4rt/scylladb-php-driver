add_library(util STATIC)
add_library(ext_scylladb::utils ALIAS util)

scylladb_php_library(util OFF ${PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE} ON)

target_sources(
        util
        PRIVATE
        src/bytes.cpp
        src/collections.cpp
        src/future.cpp
        src/hash.cpp
        src/inet.cpp
        src/math.cpp
        src/ref.cpp
        src/result.cpp
        src/types.cpp
        src/uuid_gen.cpp
)
