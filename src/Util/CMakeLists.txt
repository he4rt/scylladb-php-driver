php_driver_module(util ext_scylladb::utils)

target_sources(util PRIVATE
        collections.cpp
        hash.cpp
        math.cpp
        result.cpp
        types.cpp
)
