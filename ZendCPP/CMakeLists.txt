# ZendCPP - Modern C++ Wrapper for Zend API
# This library provides RAII wrappers and helper classes for PHP extension development

# Main library
add_library(zend STATIC Zend.cpp)
add_library(Zend ALIAS zend)
scylladb_php_library(zend OFF "${CPU_TYPE}" "${ENABLE_LTO}")

# Add string builder subdirectory
add_subdirectory(String)

# Link string library
target_link_libraries(zend PUBLIC zend_strings)

# Install all header files for the library
set(ZENDCPP_HEADERS
    ZendCPP.hpp
    ZVal.hpp
    String.hpp
    Exception.hpp
    Helpers.hpp
    Class.hpp
    Runtime.hpp
    Utilities.hpp
    Examples.hpp
)

# Documentation files
set(ZENDCPP_DOCS
    README.md
    MIGRATION.md
    CHEATSHEET.md
)

# Make headers available to other targets
target_sources(zend INTERFACE ${ZENDCPP_HEADERS})

# Set include directory
target_include_directories(zend PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

