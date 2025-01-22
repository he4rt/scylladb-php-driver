add_library(retry_policy STATIC)
add_library(ext_scylladb::retry_policy ALIAS retry_policy)
scylladb_php_library(retry_policy OFF "${CPU_TYPE}" "${ENABLE_LTO}")


target_sources(
        retry_policy
        PRIVATE
        DefaultPolicy.cpp
        DowngradingConsistency.cpp
        Logging.cpp
        Fallthrough.cpp
        RetryPolicy.cpp
)