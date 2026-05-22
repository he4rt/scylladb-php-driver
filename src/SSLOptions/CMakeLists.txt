php_scylladb_module(ssl_options ext_scylladb::ssl_options)

target_sources(ssl_options PRIVATE
        Builder.c
        SSLOptions.c
)

php_scylladb_generate_arginfo(ssl_options
        Builder.stub.php
        SSLOptions.stub.php
)

php_scylladb_generate_descriptor(ssl_options
        Builder.stub.php
        SSLOptions.stub.php
)
