php_driver_module(ssl_options ext_scylladb::ssl_options)

target_sources(ssl_options PRIVATE
        Builder.cpp
        SSLOptions.cpp
)

php_driver_generate_arginfo(ssl_options
        Builder.stub.php
        SSLOptions.stub.php
)
