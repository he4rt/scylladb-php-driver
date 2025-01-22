add_library(numbers STATIC)
add_library(ext_scylladb::numbers ALIAS numbers)
scylladb_php_library(numbers OFF "${CPU_TYPE}" "${ENABLE_LTO}")


target_sources(
        numbers
        PRIVATE

        Bigint.cpp
        Decimal.cpp
        Float.cpp
        Numeric.cpp
        Smallint.cpp
        Tinyint.cpp
        Varint.cpp
)

