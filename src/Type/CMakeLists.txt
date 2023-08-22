add_library(type STATIC)
add_library(ext_scylladb::type ALIAS type)

scylladb_php_library(type OFF ${PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE} ON)

target_sources(
        type
        PRIVATE
        Collection.cpp
        Custom.cpp
        Map.cpp
        Scalar.cpp
        Set.cpp
        Tuple.cpp
        UserType.cpp
        Type.cpp
)

