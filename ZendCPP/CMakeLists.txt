add_library(zend STATIC Zend.cpp)
add_library(Zend ALIAS zend)
scylladb_php_library(zend)

add_subdirectory(String)

target_link_libraries(zend PUBLIC Zend::String)
