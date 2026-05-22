php_scylladb_module(type ext_scylladb::type)

target_sources(type PRIVATE
        Collection.cpp
        Conversions.cpp
        Custom.cpp
        Map.cpp
        Scalar.cpp
        Set.cpp
        Tuple.cpp
        TypeFactory.cpp
        UserType.cpp
        ValueHash.cpp
)

php_scylladb_generate_arginfo(type
        Collection.stub.php
        Set.stub.php
        Custom.stub.php
        Tuple.stub.php
        Map.stub.php
        Type.stub.php
        Scalar.stub.php
        UserType.stub.php
)

php_scylladb_generate_descriptor(type
        Collection.stub.php
        Set.stub.php
        Custom.stub.php
        Tuple.stub.php
        Map.stub.php
        Scalar.stub.php
        UserType.stub.php
)
