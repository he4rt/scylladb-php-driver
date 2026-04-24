find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBGMP REQUIRED IMPORTED_TARGET gmp)

target_link_libraries(ext_scylladb PRIVATE PkgConfig::LIBGMP)
