php_driver_module(numbers ext_scylladb::numbers)

target_sources(numbers PRIVATE
        Bigint.cpp
        Decimal.cpp
        Float.cpp
        Numeric.cpp
        Smallint.cpp
        Tinyint.cpp
        Varint.cpp
)
