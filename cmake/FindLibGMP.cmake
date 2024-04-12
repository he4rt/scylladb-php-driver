find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBGMP REQUIRED IMPORTED_TARGET gmp)

target_link_libraries(ext_scylladb PRIVATE ${LIBGMP_LIBRARIES})
target_link_directories(ext_scylladb PRIVATE ${LIBGMP_LIBRARY_DIRS})
target_include_directories(ext_scylladb PUBLIC ${LIBGMP_INCLUDE_DIRS})
