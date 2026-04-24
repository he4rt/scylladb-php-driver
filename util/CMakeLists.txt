php_driver_module(util ext_scylladb::utils)

target_sources(util PRIVATE
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
