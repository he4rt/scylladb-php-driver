---
name: cmake-review
description: Review CMakeLists.txt and cmake/ module files for modern CMake correctness, anti-patterns, portability bugs, and find-module issues. Use after modifying any CMake file.
allowed-tools: Read Grep Glob
---

Review the CMake file(s) at `$ARGUMENTS` (or all cmake/ files and CMakeLists.txt if no argument).

For each finding: **file:line — issue — fix**. Skip false positives and style nits; flag only real bugs and meaningful improvements.

## 1. Double-Evaluation Bugs

- `if (${VAR})` → should be `if (VAR)` — the `${}` dereference causes double-evaluation; if `VAR` contains special characters or is empty, behaviour is undefined
- `if (${VAR} STREQUAL "something")` → `if (VAR STREQUAL "something")`
- `if (NOT ${VAR} STREQUAL "")` → `if (NOT VAR STREQUAL "")`
- Common offenders: `if (${APPLE})`, `if (${lto})`, `if (${PHP_DEBUG})`, `if (${ENABLE_*})`

## 2. Global State Mutations Inside Functions/Macros

- `set(CMAKE_CXX_STANDARD ...)` inside a function affects ALL subsequent targets, not just the current one — use `set_target_properties(target PROPERTIES CXX_STANDARD ...)` instead
- Same for `CMAKE_C_STANDARD`, `CMAKE_C_EXTENSIONS`, `CMAKE_CXX_EXTENSIONS`
- `set(CMAKE_*_FLAGS ...)` concatenation to add `-fPIC` or similar — use `set_property(TARGET ... PROPERTY POSITION_INDEPENDENT_CODE ON)` or `target_compile_options()` with explicit visibility

## 3. Incorrect `target_compile_definitions` Usage

- `-D` prefix in `target_compile_definitions()` is wrong — CMake adds it automatically
  - BAD: `target_compile_definitions(t PRIVATE -DDEBUG -DCOMPILE_DL_CASSANDRA)`
  - GOOD: `target_compile_definitions(t PRIVATE DEBUG COMPILE_DL_CASSANDRA)`
- `-D` prefix in `target_compile_options()` is technically valid but definitions belong in `target_compile_definitions()`

## 4. Missing `IMPORTED_TARGET` Usage in Find Modules

When `pkg_check_modules()` is called with `IMPORTED_TARGET`, use the created target directly:
- BAD: manually list `${LIBFOO_LIBRARIES}`, `${LIBFOO_LIBRARY_DIRS}`, `${LIBFOO_INCLUDE_DIRS}` across three separate calls
- GOOD: `target_link_libraries(my_target PRIVATE PkgConfig::LIBFOO)`
The imported target handles include dirs, library dirs, and link flags automatically and correctly handles spaces in paths.

## 5. execute_process Without Error Checking

Every `execute_process()` call must capture `RESULT_VARIABLE` and check it:
```cmake
execute_process(
    COMMAND some_program --flag
    OUTPUT_VARIABLE OUT
    ERROR_VARIABLE  ERR
    RESULT_VARIABLE RC
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT RC EQUAL 0)
    message(FATAL_ERROR "some_program failed: ${ERR}")
endif ()
```

## 6. Fragile String Parsing

- Splitting `-I/path/a -I/path/b` with naive space replacement breaks for paths with spaces
- Use `string(REGEX REPLACE "(^| )-I" ";" result "${raw}")` then strip leading `;`
- Or `separate_arguments(result UNIX_COMMAND "${raw}")` for shell-like splitting
- Never construct CMake lists by direct space manipulation

## 7. Redundant Self-Assignments

`set(VAR ${VAR})` does nothing — remove these lines. They appear as leftovers after refactors.

## 8. Redundant Policy Blocks

With `cmake_minimum_required(VERSION 3.30)`, all policies up to CMake 3.30 are already NEW. Individual `if(POLICY CMP0NNN) cmake_policy(SET CMP0NNN NEW) endif()` blocks are dead code for policies before 3.30.

## 9. GLOB Without CONFIGURE_DEPENDS

`file(GLOB_RECURSE FILES ...)` without `CONFIGURE_DEPENDS` means CMake won't re-glob when files are added — the build silently misses new files.
- Either add `CONFIGURE_DEPENDS` (slightly slower configure) or explicitly list sources (preferred)
- Explicitly listing source files in `target_sources()` is the modern CMake recommendation

## 10. Build Type String Consistency

Check that the build type strings used in `if (CMAKE_BUILD_TYPE STREQUAL "...")` match exactly what the presets and toolchain files use. Common mismatch: `"RelWithInfo"` vs `"RelWithDebInfo"` (the standard CMake name).

## 11. Hardcoded Paths in Find Modules

- Hardcoded `/root/php`, `/opt/...`, `/usr/local` hints in `find_program()`/`find_library()` are fragile
- Prefer reading from environment variables or cache variables: `$ENV{PHP_ROOT}`, `${PHP_ROOT}`
- Always include `/usr` and `/usr/local` as fallback hints, not primary

## 12. find_package / pkg_check_modules Error Context

When a `REQUIRED` find fails, the generic CMake error is unhelpful. Add context:
```cmake
if (NOT LIBFOO_FOUND)
    message(FATAL_ERROR
        "libfoo not found. Install with: apt-get install libfoo-dev "
        "or set LIBFOO_ROOT to the install prefix.")
endif ()
```

## 13. Target Visibility Correctness

- Include directories that callers need: `PUBLIC` or `INTERFACE`
- Include directories internal to the target: `PRIVATE`
- Libraries only this target needs: `PRIVATE`
- Libraries whose headers callers include: `PUBLIC`
- Mixing PUBLIC/PRIVATE incorrectly causes missing includes in dependent targets or leaks build details

## 14. cmake_minimum_required Placement

`cmake_minimum_required()` must be the first command in the top-level CMakeLists.txt, before `project()`. Nothing (not even `set()`) should precede it.

## 15. Unset Temporary Variables

Local variables used during configuration (path hints, temporary strings, loop vars) should be `unset()` after use to avoid polluting the cache or leaking into subdirectory scopes:
```cmake
set(_my_hint "/some/path")
find_program(FOO_EXE foo HINTS "${_my_hint}")
unset(_my_hint)
```
Convention: prefix temporary variables with `_` to mark them as local.

---

Report all findings by section. If a section is clean, say "✓ clean". End with: PASS, NEEDS FIXES (n issues), or CRITICAL (build correctness broken).
