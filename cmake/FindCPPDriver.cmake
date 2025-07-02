find_package(PkgConfig REQUIRED)

if(USE_LIBCASSANDRA)
    if (PHP_DRIVER_STATIC)
        pkg_check_modules(LIBCASSANDRA IMPORTED_TARGET cassandra_static)
    else ()
        pkg_check_modules(LIBCASSANDRA IMPORTED_TARGET cassandra)
    endif ()

    # If pkg-config fails, try to find Cassandra manually (e.g., Homebrew installation)
    if(NOT LIBCASSANDRA_FOUND)
        message(STATUS "pkg-config failed to find cassandra, trying manual detection...")
        
        # Try common paths (Homebrew on macOS)
        find_path(CASSANDRA_INCLUDE_DIR cassandra.h
            PATHS /opt/homebrew/include /usr/local/include
            PATH_SUFFIXES cassandra
        )
        
        find_library(CASSANDRA_LIBRARY
            NAMES cassandra libcassandra
            PATHS /opt/homebrew/lib /usr/local/lib
        )
        
        if(CASSANDRA_INCLUDE_DIR AND CASSANDRA_LIBRARY)
            set(LIBCASSANDRA_FOUND TRUE)
            set(LIBCASSANDRA_INCLUDE_DIRS ${CASSANDRA_INCLUDE_DIR})
            set(LIBCASSANDRA_LIBRARIES ${CASSANDRA_LIBRARY})
            message(STATUS "Found Cassandra manually: ${CASSANDRA_LIBRARY}")
        else()
            message(FATAL_ERROR "Could not find Cassandra driver")
        endif()
    endif()

    target_link_libraries(ext_scylladb PRIVATE ${LIBCASSANDRA_LIBRARIES})
    if(LIBCASSANDRA_LIBRARY_DIRS)
        target_link_directories(ext_scylladb PRIVATE ${LIBCASSANDRA_LIBRARY_DIRS})
    endif()
    target_include_directories(ext_scylladb PUBLIC ${LIBCASSANDRA_INCLUDE_DIRS})
    set(CASSANDRA_H ${LIBCASSANDRA_INCLUDE_DIRS})
else()
    if (PHP_DRIVER_STATIC)
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver_static)
    else ()
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver)
    endif ()

    target_include_directories(ext_scylladb PUBLIC "${LIBSCYLLADB_INCLUDE_DIRS}")
    target_link_libraries(ext_scylladb PRIVATE "${LIBSCYLLADB_LIBRARIES}")
    target_link_directories(ext_scylladb PRIVATE "${LIBSCYLLADB_LIBRARY_DIRS}")
    set(CASSANDRA_H ${LIBSCYLLADB_INCLUDE_DIRS})
endif()
