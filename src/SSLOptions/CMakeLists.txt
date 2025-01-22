add_library(ssl_options STATIC)
add_library(ext_scylladb::ssl_options ALIAS ssl_options)
scylladb_php_library(ssl_options OFF "${CPU_TYPE}" "${ENABLE_LTO}")

target_sources(
        ssl_options
        PRIVATE
        Builder.cpp
        SSLOptions.cpp
)