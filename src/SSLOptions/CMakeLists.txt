php_driver_module(ssl_options ext_scylladb::ssl_options)

target_sources(ssl_options PRIVATE
        Builder.cpp
        SSLOptions.cpp
)
