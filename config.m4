PHP_ARG_ENABLE([cassandra],
  [whether to enable cassandra support],
  [AS_HELP_STRING([--enable-cassandra],
    [Enable cassandra support])],
  [no])

PHP_ARG_ENABLE([debug], [whether to enable cassandra debug mode],
[  --enable-debug           Enable cassandra debug], no, no)

PHP_ARG_ENABLE([lto], [whether to enable lto support],
[  --enable-lto           Enable lto], no, no)

PHP_ARG_WITH(cpu-type,
  [cpu type For x86_64 = x86-64|x86-64-v2|x86-64-v3|x86-64-v4|native],
  [x86-64-v2],
  [no],
  [no]
)

PHP_ARG_WITH(driver-version,
  [for the driver],
  [master],
  [no],
  [no]
)

PHP_ARG_ENABLE([avx], [whether to compile with avx instructions],
[  --enable-avx          ], no, no)

PHP_ARG_ENABLE([sanitizers], [whether to link with address and mem sanitizers],
[  --enable-sanitizers          ], no, no)

PHP_ARG_ENABLE([avx2], [whether to compile with avx2 instructions],
[  --enable-avx2          ], no, no)

PHP_ARG_ENABLE([libuv-static], [whether to link with libuv statically],
[  --enable-libuv-static          ], no, no)

PHP_ARG_ENABLE([driver-static], [whether to link with scylladb/cassandra driver statically],
[  --enable-driver-static          ], no, no)

PHP_ARG_ENABLE([libcassandra], [whether to link with cassandra driver instead of scylladb],
[  --enable-libcassandra          ], no, no)

if test "$PHP_cassandra" != "no"; then
    CMAKE_GENERATOR="Unix Makefiles"
    CMAKE_BUILD_TYPE="Release"
    CMAKE_CPU_TYPE="x86-64-v2"
    CMAKE_PHP_DRIVER_VERSION="master"
    CMAKE_ENABLE_LTO="OFF"
    CMAKE_SANITIZERS="OFF"
    CMAKE_AVX="OFF"
    CMAKE_AVX2="OFF"
    CMAKE_LINK_LIBUV_STATIC="OFF"
    CMAKE_DRIVER_STATIC="OFF"
    CMAKE_USE_CASSANDRA="OFF"

    AC_PATH_PROG(CMAKE, cmake, no)
    if ! test -x "$CMAKE"; then
        AC_MSG_ERROR([cmake command missing])
    fi

    AC_PATH_PROG(NINJA, ninja, no)
    if ! test -x "$NINJA"; then
        CMAKE_GENERATOR="Ninja"
    fi

	if test "$PHP_DEBUG" = "yes"; then
		CMAKE_BUILD_TYPE="Debug"
	fi

	if test "$PHP_CPU_TYPE" != "no"; then
		CMAKE_CPU_TYPE="$PHP_CPU_TYPE"
	fi
	if test "$PHP_DRIVER_VERSION" != "no"; then
		CMAKE_PHP_DRIVER_VERSION="$PHP_DRIVER_VERSION"
	fi

    if test "$PHP_LTO" = "yes"; then
        CMAKE_ENABLE_LTO="ON"
    fi

    if test "$PHP_SANITIZERS" = "yes"; then
        CMAKE_SANITIZERS="ON"
    fi

    if test "$PHP_AVX" = "yes"; then
        CMAKE_AVX="ON"
    fi

    if test "$PHP_AVX2" = "yes"; then
        CMAKE_AVX2="ON"
    fi

    if test "$PHP_LINK_LIBUV_STATIC" = "yes"; then
        CMAKE_LINK_LIBUV_STATIC="ON"
    fi

    if test "$PHP_DRIVER_STATIC" = "yes"; then
        CMAKE_DRIVER_STATIC="ON"
    fi

    if test "$PHP_LIBCASSANDRA" = "yes"; then
        CMAKE_USE_CASSANDRA="ON"
    fi

    AC_DEFINE(HAVE_cassandra, 1, [ Have cassandra support ])

    PHP_NEW_EXTENSION(cassandra, [ ], $ext_shared)

	  # PHP_INSTALL_HEADERS([ext/cassandra], [include/php_ext.h])

    AC_CONFIG_LINKS([ \
        CMakeLists.txt:CMakeLists.txt \
        include:include \
        src:src \
    ])

  cat >>Makefile.objects<< EOF
all: cmake_build

clean: cmake_clean

.PHONY: cmake_build
cmake_build:
		$CMAKE -G "$CMAKE_GENERATOR" -B out/ \\
			-DCUSTOM_PHP_CONFIG=$PHP_CONFIG \\
			-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \\
			-DCPU_TYPE=$CMAKE_CPU_TYPE \\
			-DENABLE_LTO=$CMAKE_ENABLE_LTO \\
			-DENABLE_AVX=$CMAKE_AVX \\
			-DENABLE_AVX2=$CMAKE_AVX2 \\
			-DENABLE_SANITIZERS=$CMAKE_SANITIZERS \\
			-DLINK_LIBUV_STATIC=$CMAKE_LINK_LIBUV_STATIC \\
			-DPHP_DRIVER_STATIC=$CMAKE_DRIVER_STATIC \\
			-DUSE_LIBCASSANDRA=$CMAKE_USE_CASSANDRA \\
			-DCMAKE_INSTALL_PREFIX=./out/install \\
			-DPHP_SCYLLADB_DRIVER_VERSION=$CMAKE_PHP_DRIVER_VERSION
		cmake --build out/

	if [[ -f ./out/cassandra.dylib ]] ; then \\
		cp ./out/cassandra.dylib ./modules/cassandra.so ; fi
	if [[ -f ./out/cassandra.so ]] ; then \\
		cp ./out/cassandra.so ./modules/cassandra.so ; fi

.PHONY: cmake_clean
cmake_clean:
		rm -rf build
EOF
fi
