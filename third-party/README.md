# Bundled dependencies

## Local Library Builds

The build scripts in `scripts/` will clone and compile the following libraries into this directory:

- **libuv** - Cloned to `libuv/`, installed to `libuv-install/`
- **ScyllaDB C++ Driver** - Cloned to `scylladb-driver/`, installed to `scylladb-driver-install/`
- **DataStax C++ Driver** - Cloned to `datastax-driver/`, installed to `datastax-driver-install/`

All libraries are built with debug symbols using the following CFLAGS:
```
-g -ggdb -g3 -gdwarf-5 -fno-omit-frame-pointer
```

This enables comprehensive debugging with detailed DWARF-5 debug information and preserves frame pointers for better stack traces.

## CMake Integration

The main `CMakeLists.txt` automatically adds the local installation directories to `PKG_CONFIG_PATH`:
- `third-party/libuv-install/lib/pkgconfig`
- `third-party/scylladb-driver-install/lib/pkgconfig`
- `third-party/datastax-driver-install/lib/pkgconfig`

This allows CMake's `find_package()` to locate the locally-built libraries without requiring system installation.

## Building

Run `./scripts/setup` from the project root to build all dependencies locally.
