PHP_ARG_ENABLE([cassandra],
  [whether to enable cassandra support],
  [AS_HELP_STRING([--enable-cassandra], [Enable ScyllaDB/Cassandra PHP extension])],
  [no])

PHP_ARG_ENABLE([debug],
  [whether to enable debug mode],
  [  --enable-debug           Enable debug build],
  [no], [no])

PHP_ARG_ENABLE([lto],
  [whether to enable link-time optimisation],
  [  --enable-lto             Enable LTO],
  [no], [no])

PHP_ARG_WITH([cpu-type],
  [x86-64 micro-architecture level],
  [  --with-cpu-type=TYPE     x86-64|x86-64-v2|x86-64-v3|x86-64-v4|native (default: x86-64-v2)],
  [x86-64-v2], [no])

PHP_ARG_ENABLE([avx],
  [whether to enable AVX instructions],
  [  --enable-avx             Enable -mavx on x86_64],
  [no], [no])

PHP_ARG_ENABLE([avx2],
  [whether to enable AVX2 instructions],
  [  --enable-avx2            Enable -mavx2 on x86_64],
  [no], [no])

PHP_ARG_ENABLE([sanitizers],
  [whether to enable address/UB sanitizers],
  [  --enable-sanitizers      Enable AddressSanitizer + UBSan],
  [no], [no])

PHP_ARG_ENABLE([driver-static],
  [whether to link the C++ driver statically],
  [  --enable-driver-static   Link ScyllaDB/Cassandra C++ driver statically],
  [no], [no])

PHP_ARG_ENABLE([libcassandra],
  [whether to use the DataStax libcassandra driver],
  [  --enable-libcassandra    Use DataStax libcassandra instead of libscylla-cpp-driver],
  [no], [no])

if test "$PHP_CASSANDRA" != "no"; then
    dnl ── Locate cmake ──────────────────────────────────────────────────────
    AC_PATH_PROG([CMAKE], [cmake], [no])
    if test "$CMAKE" = "no"; then
        AC_MSG_ERROR([cmake is required to build the ScyllaDB PHP extension])
    fi

    dnl ── Choose generator: Ninja if available, otherwise Unix Makefiles ────
    AC_PATH_PROG([NINJA], [ninja], [no])
    if test "$NINJA" != "no"; then
        CMAKE_GENERATOR="Ninja"
    else
        CMAKE_GENERATOR="Unix Makefiles"
    fi

    dnl ── Map configure flags → cmake cache variables ───────────────────────
    CMAKE_BUILD_TYPE="Release"
    if test "$PHP_DEBUG" = "yes"; then
        CMAKE_BUILD_TYPE="Debug"
    fi

    CMAKE_CPU_TYPE="x86-64-v2"
    if test "$PHP_CPU_TYPE" != "no" && test "$PHP_CPU_TYPE" != "yes"; then
        CMAKE_CPU_TYPE="$PHP_CPU_TYPE"
    fi

    CMAKE_ENABLE_LTO="OFF"
    if test "$PHP_LTO" = "yes"; then CMAKE_ENABLE_LTO="ON"; fi

    CMAKE_ENABLE_AVX="OFF"
    if test "$PHP_AVX" = "yes"; then CMAKE_ENABLE_AVX="ON"; fi

    CMAKE_ENABLE_AVX2="OFF"
    if test "$PHP_AVX2" = "yes"; then CMAKE_ENABLE_AVX2="ON"; fi

    CMAKE_ENABLE_SANITIZERS="OFF"
    if test "$PHP_SANITIZERS" = "yes"; then CMAKE_ENABLE_SANITIZERS="ON"; fi

    CMAKE_DRIVER_STATIC="OFF"
    if test "$PHP_DRIVER_STATIC" = "yes"; then CMAKE_DRIVER_STATIC="ON"; fi

    CMAKE_USE_LIBCASSANDRA="OFF"
    if test "$PHP_LIBCASSANDRA" = "yes"; then CMAKE_USE_LIBCASSANDRA="ON"; fi

    dnl ── Use phpize's build dir so make install picks up the .so ──────────
    CMAKE_BUILD_DIR="cmake-build"

    AC_DEFINE([HAVE_CASSANDRA], [1], [Have ScyllaDB/Cassandra extension])

    PHP_NEW_EXTENSION([cassandra], [ ], [$ext_shared])

    dnl ── Inject cmake targets into the phpize Makefile ─────────────────────
    dnl We append rules to Makefile.objects. The 'all' target here is picked
    dnl up by phpize because Makefile.objects is included after the main rules;
    dnl phpize's own shared-lib rule is replaced by cmake_build.
    cat >> Makefile.objects << MAKEFILE_EOF

# ── ScyllaDB PHP Driver: cmake-based build ────────────────────────────────────
CMAKE          = $CMAKE
CMAKE_FLAGS    = -G "$CMAKE_GENERATOR" \\
                 -DCUSTOM_PHP_CONFIG=$(PHP_CONFIG) \\
                 -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \\
                 -DCPU_TYPE=$CMAKE_CPU_TYPE \\
                 -DENABLE_LTO=$CMAKE_ENABLE_LTO \\
                 -DENABLE_AVX=$CMAKE_ENABLE_AVX \\
                 -DENABLE_AVX2=$CMAKE_ENABLE_AVX2 \\
                 -DENABLE_SANITIZERS=$CMAKE_ENABLE_SANITIZERS \\
                 -DPHP_SCYLLADB_STATIC=$CMAKE_DRIVER_STATIC \\
                 -DUSE_LIBCASSANDRA=$CMAKE_USE_LIBCASSANDRA
CMAKE_BUILD_DIR = $CMAKE_BUILD_DIR

.PHONY: cmake_build cmake_install cmake_clean

cmake_build:
	\$(CMAKE) \$(CMAKE_FLAGS) -B \$(CMAKE_BUILD_DIR)
	\$(CMAKE) --build \$(CMAKE_BUILD_DIR) --parallel
	@mkdir -p modules
	@if [ -f \$(CMAKE_BUILD_DIR)/cassandra.dylib ]; then \\
	    cp \$(CMAKE_BUILD_DIR)/cassandra.dylib modules/cassandra.so; fi
	@if [ -f \$(CMAKE_BUILD_DIR)/cassandra.so ]; then \\
	    cp \$(CMAKE_BUILD_DIR)/cassandra.so modules/cassandra.so; fi

cmake_install: cmake_build
	\$(CMAKE) --install \$(CMAKE_BUILD_DIR) --component extension
	\$(CMAKE) --install \$(CMAKE_BUILD_DIR) --component ini

cmake_clean:
	rm -rf \$(CMAKE_BUILD_DIR) modules

all: cmake_build
install: cmake_install
clean: cmake_clean

MAKEFILE_EOF

fi
