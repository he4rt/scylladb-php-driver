add_library(zend STATIC Zend.cpp)
add_library(Zend ALIAS zend)
scylladb_php_library(zend OFF "${CPU_TYPE}" "${ENABLE_LTO}")

add_subdirectory(String)

target_link_libraries(zend PUBLIC zend_strings)
