php_scylladb_module(numbers ext_scylladb::numbers)

target_sources(numbers PRIVATE
        Bigint.c
        Decimal.c
        Float.c
        NumberParser.c
        Smallint.c
        Tinyint.c
        Varint.c
)

php_scylladb_generate_arginfo(numbers
        Bigint.stub.php
        Smallint.stub.php
        Decimal.stub.php
        Tinyint.stub.php
        Float.stub.php
        Varint.stub.php
        Numeric.stub.php
)

php_scylladb_generate_descriptor(numbers
        Bigint.stub.php
        Smallint.stub.php
        Decimal.stub.php
        Tinyint.stub.php
        Float.stub.php
        Varint.stub.php
        Numeric.stub.php
)
