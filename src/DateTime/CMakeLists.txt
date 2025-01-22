add_library(datetime STATIC)
add_library(ext_scylladb::datetime ALIAS datetime)
scylladb_php_library(datetime OFF "${CPU_TYPE}" "${ENABLE_LTO}")


target_sources(
        datetime
        PRIVATE
        Date.cpp
        Time.cpp
        Timestamp.cpp
        Timeuuid.cpp
        DateTimeInternal.cpp
        Duration.cpp
)

