php_driver_module(timestamp_generator ext_scylladb::timestamp_generator)

target_sources(timestamp_generator PRIVATE
        Monotonic.cpp
        ServerSide.cpp
        TimestampGenerator.cpp
)
