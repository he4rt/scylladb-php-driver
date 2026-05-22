php_driver_module(util ext_scylladb::utils)

target_sources(util PRIVATE
        src/collections.cpp
        src/hash.cpp
        src/math.cpp
        src/result.cpp
        src/types.cpp
)
