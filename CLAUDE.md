# ScyllaDB PHP Driver — Development Guide

## Project Overview

PHP extension (C/C++) for ScyllaDB/Cassandra. Extension name is `cassandra`, PHP namespace is `Cassandra\`.
The project is **migrating from C++ to pure C23**. New code must be C23; existing C++ code will be removed incrementally.

## Language Standard

- **Target**: C23 (ISO/IEC 9899:2024)
- **Transitional**: C++23 is still present but being removed — do not add new C++ code
- In `cmake/TargetOptimizations.cmake`: C23 (`c_std_23`, extensions ON for GNU C), C++20 for legacy files
- New files use `.c` extension; existing `.cpp` files stay until explicitly ported

## Module Architecture — The Canonical Pattern

`src/Cluster/` is the **reference implementation**. Every module must follow this exact structure:

```
src/MyModule/
├── MyClass.stub.php          # PHP interface/class definition — source of truth
├── MyClass_arginfo.h         # GENERATED at build time — gitignored, never edit, never commit
├── MyClass.c                 # C implementation of ZEND_METHOD()s
├── MyClassHandlers.h         # Handler declarations
├── MyClassHandlers.c         # Object handlers (new/free/gc/properties/compare)
├── MyModule.h                # Public header (register_* function declarations)
└── CMakeLists.txt            # Build config
```

### PHP Stub → C Code Flow

1. **Write the stub** — defines PHP-visible API with full type hints:
   ```php
   /** @generate-class-entries */
   namespace Cassandra\MyModule {
       final class MyClass {
           public function myMethod(int $value): MyClass {}
       }
   }
   ```

2. **Generate arginfo** — the build does this for you. `cmake/GenStubs.cmake` runs
   `tools/gen_stub/gen_arginfo.sh` on every stub, so a plain `cmake --build` regenerates
   `MyClass_arginfo.h` whenever the stub changes. Commit the stub only — `_arginfo.h` is
   gitignored (`.gitignore`: `src/**/*_arginfo.h`).

   To run it by hand, call the wrapper, not `gen_stub.php` directly:
   ```bash
   tools/gen_stub/gen_arginfo.sh src/MyModule/MyClass.stub.php php path/to/build/gen_stub.php
   ```
   The wrapper strips `declare(strict_types=1);` before parsing, because upstream `gen_stub.php`
   fails on it with `Unexpected node Stmt_Declare`. It also casts the emitted
   `zend_add_*_attribute()` arguments so the output compiles as C++.

3. **Include arginfo in implementation**:
   ```c
   BEGIN_EXTERN_C()
   #include "MyClass_arginfo.h"   // generated — provides ZEND_METHOD table & register_class_*()
   ```

4. **Implement methods** using `ZEND_METHOD(Cassandra_MyModule_MyClass, methodName)`

5. **Register in define function**:
   ```c
   void php_driver_define_MyClass(void) {
       php_driver_my_class_ce = register_class_Cassandra_MyModule_MyClass();
       php_driver_initialize_my_class_handlers();
       php_driver_my_class_ce->create_object = php_driver_my_class_new;
   }
   ```

## C23 Coding Standards

### Naming Conventions

| Thing | Convention | Example |
|-------|-----------|---------|
| Internal struct | `php_driver_<module>_t` | `php_driver_cluster_builder_t` |
| Class entry global | `php_driver_<module>_ce` | `php_driver_cluster_builder_ce` |
| Object fetch macro | `PHP_DRIVER_GET_<MODULE>(obj)` | `PHP_DRIVER_GET_CLUSTER_BUILDER(obj)` |
| Handler init fn | `php_driver_initialize_<module>_handlers()` | |
| Constructor fn | `php_driver_<module>_new(ce)` | |
| Module define fn | `php_driver_define_<ClassName>()` | |

### Internal Struct Layout

Always place `zend_object zendObject` **last** in the struct — required for `XtOffsetOf`:

```c
typedef struct {
    int32_t     field_one;
    zend_string *field_str;
    zend_object  zendObject;   // must be last
} php_driver_my_class_t;

// Object fetch via offsetof arithmetic:
static inline php_driver_my_class_t *php_driver_my_class_fetch(zend_object *obj) {
    return (php_driver_my_class_t *)((char *)obj - XtOffsetOf(php_driver_my_class_t, zendObject));
}
#define PHP_DRIVER_GET_MY_CLASS(zval_p) php_driver_my_class_fetch(Z_OBJ_P(zval_p))
```

### Parameter Parsing

Always use the modern API — never `zend_parse_parameters()`:

```c
ZEND_METHOD(Cassandra_MyModule_MyClass, withPort) {
    zend_long port = 0;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END();

    // validate ...
    PHP_DRIVER_GET_MY_CLASS(ZEND_THIS)->port = (uint16_t)port;
    RETURN_ZVAL(ZEND_THIS, 1, 0);   // fluent: return $this
}
```

### Object Handlers (C23 style)

```c
// In *Handlers.c — use compound-literal designated-initialiser style where possible
static zend_object_handlers php_driver_my_class_handlers;

void php_driver_initialize_my_class_handlers(void) {
    memcpy(&php_driver_my_class_handlers,
           zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));
    php_driver_my_class_handlers.offset       = XtOffsetOf(php_driver_my_class_t, zendObject);
    php_driver_my_class_handlers.free_obj     = php_driver_my_class_free;
    php_driver_my_class_handlers.get_gc       = php_driver_my_class_gc;
    php_driver_my_class_handlers.get_properties = php_driver_my_class_properties;
    php_driver_my_class_handlers.compare      = php_driver_my_class_compare;
}
```

### Resource Management

- Release `zend_string *` with `zend_string_release()` — never `free()`
- Release zend objects with `zend_object_release(&obj->zendObject)`
- Use `ZVAL_UNDEF` to mark zvals as unset; check with `Z_ISUNDEF()`
- Free with `zval_ptr_dtor()` before overwriting a live zval

### C23 Specifics

- Use `bool` / `true` / `false` (C23 keywords, no `<stdbool.h>` needed)
- Use `[[nodiscard]]`, `[[maybe_unused]]` attributes
- Prefer `nullptr` over `NULL` in new C code (C23 keyword)
- `typeof` and `typeof_unqual` over `__typeof__`
- `_Static_assert(cond, msg)` for compile-time checks

### What to Avoid

- No `zend_parse_parameters()` — use `ZEND_PARSE_PARAMETERS_START/END` only
- No `PHP_ME` / `PHP_ABSTRACT_ME` / `PHP_FE_END` in new code — use stub-generated tables
- No manual `ZEND_BEGIN_ARG_INFO_EX` / `ZEND_END_ARG_INFO` — stubs generate these
- No raw `malloc`/`free` — use `emalloc`/`efree` for request-lifetime, `pemalloc`/`pefree` for persistent
- No C++ casts in C files (`static_cast`, `reinterpret_cast`) — use C casts
- No new `.cpp` files — port to `.c` when touching a file

## Build System

### CMake Presets (generated)

```bash
php generate-presets.php   # regenerate CMakePresets.json after changing versions
```

### Local Development Build

```bash
cmake --preset DebugPHP8.4NTS        # configure
cmake --build out/DebugPHP8.4NTS     # compile
```

### Adding a New Module

1. Create `src/MyModule/CMakeLists.txt`:
   ```cmake
   target_sources(ext_scylladb::my_module INTERFACE
       MyClass.c
       MyClassHandlers.c
   )
   ```

2. Add `add_subdirectory(src/MyModule)` to root `CMakeLists.txt`

3. Add `ext_scylladb::my_module` to `target_link_libraries` in root `CMakeLists.txt`

## Development Workflow

### Setting Up Locally

```bash
# Install dependencies (configurable prefix, see scripts/)
./scripts/compile-libuv.sh --prefix ~/.local
./scripts/compile-cpp-driver.sh --driver scylladb --prefix ~/.local
./scripts/compile-php.sh -v 8.4 -o ./php
```

### Regenerating Arginfo After Stub Changes

Rebuild. `cmake/GenStubs.cmake` regenerates every `*_arginfo.h` whose stub changed:

```bash
cmake --build out/DebugPHP8.4NTS
```

Commit the `*.stub.php` only. The generated `*_arginfo.h` is gitignored. Do not call
`gen_stub.php` directly — it fails on `declare(strict_types=1);`. Use
`tools/gen_stub/gen_arginfo.sh` if you need to run it outside a build.

### Running Tests

```bash
./scripts/run-scylladb.sh          # start ScyllaDB via docker compose
cmake --preset DebugPHP8.4NTS && cmake --build out/DebugPHP8.4NTS
composer install
php ./vendor/bin/pest
```

## Modules Status

| Module | Status | Notes |
|--------|--------|-------|
| `src/Cluster` | Refactored | Reference implementation |
| `src/RetryPolicy` | Partial | Has stubs, handlers need C port |
| `src/DateTime` | Partial | Has stubs |
| `src/SSLOptions` | Partial | Has stubs |
| `src/Database` | Legacy | No stubs, old arg-info style |
| `src/Type` | Legacy | No stubs |
| `src/Exception` | Legacy | Thin wrappers, low priority |
| `src/Numbers` | Legacy | |
| `src/TimestampGenerator` | Legacy | |

## Claude Code Skills

### Built-in skills (always available)

| Skill | When to use |
|-------|------------|
| `/simplify` | After refactoring a module — checks reuse, quality, efficiency |
| `/security-review` | After any C code that handles user input, pointer arithmetic, or cass_* calls |
| `/review` | Before merging a PR |
| `/fewer-permission-prompts` | After a few sessions — scans history and adds allowlist entries |

### Project skills (in `.claude/skills/`)

| Skill | When to use |
|-------|------------|
| `/dev-setup` | Setting up a new machine for development — installs all deps, builds PHP, compiles extension, starts ScyllaDB |
| `/php-ext-internals` | Zend Engine API reference — module lifecycle, Z_PARAM_* macros, zvals, zend_string, HashTable, object handlers, memory, INI, globals |
| `/php-ext-review [path]` | After implementing or modifying any `src/` module — checks C23, Zend API, memory, handlers, arginfo |
| `/scylladb-review [path]` | After touching any code with `cass_*` calls — checks error handling, lifecycle, type mapping, futures |
| `/cmake-review [path]` | After modifying any CMakeLists.txt or cmake/ module — checks double-eval bugs, IMPORTED_TARGET usage, global state, find-module correctness |
| `/new-module <Module> <Class> [final\|abstract]` | Scaffolds a complete new module: stub, handlers, implementation, CMakeLists, wires into build |

### Typical workflow

```bash
# Add a new module
/new-module Session DefaultSession final

# After implementing methods
/php-ext-review src/Session/

# If it touches cass_* calls
/scylladb-review src/Session/

# Before committing
/simplify
/security-review
```
