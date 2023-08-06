add_library(collections STATIC)
add_library(ext_scylladb::collections ALIAS collections)
scylladb_php_library(collections OFF ${PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE} ON)

target_sources(
        collections
        PRIVATE
        Collection.cpp
        Map.cpp
        Set.cpp
        Tuple.cpp
)

