php_driver_module(type ext_scylladb::type)

target_sources(type PRIVATE
        Collection.cpp
        Conversions.cpp
        Custom.cpp
        Map.cpp
        Scalar.cpp
        Set.cpp
        Tuple.cpp
        TypeFactory.cpp
        UserType.cpp
        ValueHash.cpp
)
