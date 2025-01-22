add_library(timestamp_generator STATIC)
add_library(ext_scylladb::timestamp_generator ALIAS timestamp_generator)
scylladb_php_library(timestamp_generator OFF "${CPU_TYPE}" "${ENABLE_LTO}")

target_sources(
        timestamp_generator
        PRIVATE
        Monotonic.cpp
        ServerSide.cpp
        TimestampGenerator.cpp
)