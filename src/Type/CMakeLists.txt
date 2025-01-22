add_library(type STATIC)
add_library(ext_scylladb::type ALIAS type)
scylladb_php_library(type OFF "${CPU_TYPE}" "${ENABLE_LTO}")


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
)

