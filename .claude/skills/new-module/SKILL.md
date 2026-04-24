---
name: new-module
description: Scaffold a new PHP extension module following the src/Cluster canonical pattern. Creates stub, handlers, implementation skeleton, CMakeLists.txt, and wires it into the build.
allowed-tools: Read Glob Grep Write Edit Bash
---

Scaffold a new PHP extension module. The argument format is:

  /new-module <ModuleName> <ClassName> [interface|abstract|final]

Examples:
  /new-module Session DefaultSession final
  /new-module PreparedStatement PreparedStatement final
  /new-module RetryPolicy DefaultRetryPolicy final

Use `src/Cluster/` as the canonical reference for every file created.

## Step 1 — Gather context

Read these files before generating anything:
- `src/Cluster/Builder.stub.php` — stub format reference
- `src/Cluster/BuilderHandlers.h` — handler header format
- `src/Cluster/BuilderHandlers.cpp` — handler implementation format  
- `src/Cluster/Builder.cpp` — implementation format (BEGIN_EXTERN_C, arginfo include, ZEND_METHOD)
- `src/Cluster/Cluster.h` — public header format
- `src/Cluster/CMakeLists.txt` — build config format
- `include/php_driver_types.h` — existing struct definitions
- `CMakeLists.txt` (root) — to know where to add_subdirectory and target_link_libraries

## Step 2 — Create files

Create ALL of the following in `src/<ModuleName>/`:

### `<ClassName>.stub.php`
```php
<?php
/** @generate-class-entries */
namespace Cassandra\<ModuleName> {
    /**
     * @strict-properties
     */
    <final|abstract> class <ClassName> {
        // Add methods here based on what the user describes, or leave empty skeleton
    }
}
```

### `<ModuleName>.h`
```c
#pragma once

BEGIN_EXTERN_C()
extern zend_class_entry *php_driver_<module>_<class>_ce;
void php_driver_define_<ClassName>(void);
END_EXTERN_C()
```

### `<ClassName>Handlers.h`
```c
#pragma once
#include <php.h>

void php_driver_initialize_<module>_<class>_handlers(void);

BEGIN_EXTERN_C()
zend_object *php_driver_<module>_<class>_new(zend_class_entry *ce);
END_EXTERN_C()
```

### `<ClassName>Handlers.c`
Full object handler implementation:
- `memcpy` from `zend_get_std_object_handlers()`
- Set `offset`, `free_obj`, `get_gc`, `get_properties`, `compare`
- Constructor allocates with `emalloc(sizeof(php_driver_<module>_<class>_t) + zend_object_properties_size(ce))`
- Initialize all fields to safe zero/null values
- Call `zend_object_std_init` and `object_properties_init`
- Return `&self->zendObject`
- Destructor releases all owned resources (strings, nested objects, zvals)

### `<ClassName>.c`  
```c
#include <php_driver.h>
#include <php_driver_types.h>
#include "<ModuleName>.h"
#include "<ClassName>Handlers.h"

BEGIN_EXTERN_C()
#include "<ClassName>_arginfo.h"

zend_class_entry *php_driver_<module>_<class>_ce = NULL;

// ZEND_METHOD implementations go here

END_EXTERN_C()

void php_driver_define_<ClassName>(void) {
    php_driver_<module>_<class>_ce = register_class_Cassandra_<ModuleName>_<ClassName>();
    php_driver_initialize_<module>_<class>_handlers();
    php_driver_<module>_<class>_ce->create_object = php_driver_<module>_<class>_new;
}
```

### `CMakeLists.txt`
```cmake
add_library(ext_scylladb::<module> INTERFACE)
target_sources(ext_scylladb::<module> INTERFACE
    <ClassName>.c
    <ClassName>Handlers.c
)
target_include_directories(ext_scylladb::<module> INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
```

### Add struct to `include/php_driver_types.h`
```c
typedef struct {
    // fields go here
    zend_object zendObject;  // MUST be last
} php_driver_<module>_<class>_t;

static inline php_driver_<module>_<class>_t *php_driver_<module>_<class>_fetch(zend_object *obj) {
    return (php_driver_<module>_<class>_t *)((char *)obj - XtOffsetOf(php_driver_<module>_<class>_t, zendObject));
}
#define PHP_DRIVER_GET_<MODULE>_<CLASS>(zval_p) php_driver_<module>_<class>_fetch(Z_OBJ_P(zval_p))
```

## Step 3 — Wire into build

Edit root `CMakeLists.txt`:
1. Add `add_subdirectory(src/<ModuleName>)` with the other subdirectories
2. Add `ext_scylladb::<module>` to `target_link_libraries(ext_scylladb ...)`

Edit `src/php_driver.cpp`:  
1. Include the new module header
2. Call `php_driver_define_<ClassName>()` in `PHP_MINIT(php_driver)`

## Step 4 — Generate arginfo

Run:
```bash
php php/8.4-debug-nts/src/build/gen_stub.php src/<ModuleName>/<ClassName>.stub.php
```

If the PHP build doesn't exist yet, note the command for the user to run manually.

## Step 5 — Report

List every file created/modified with its path. Remind the user to:
1. Add method signatures to the stub
2. Re-run `gen_stub.php` after editing the stub
3. Implement `ZEND_METHOD` bodies in `<ClassName>.c`
4. Run `/php-ext-review src/<ModuleName>/` when implementation is complete
