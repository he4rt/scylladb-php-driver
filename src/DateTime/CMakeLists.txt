php_driver_module(datetime ext_scylladb::datetime)

target_sources(datetime PRIVATE
        Date.cpp
        Time.cpp
        Timestamp.cpp
        Timeuuid.cpp
        DateTimeInternal.cpp
        Duration.cpp
)
