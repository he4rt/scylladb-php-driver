---
name: php-ext-review
description: Review a PHP extension module (C23 style) for correctness, memory safety, and Zend API usage. Use after implementing or modifying any src/ module.
allowed-tools: Read Grep Glob
---

Review the PHP extension module at `$ARGUMENTS` (or the most recently changed src/ files if no argument given).

Check every item below. For each finding, output: **file:line — issue — fix**.

## 1. C23 Compliance

- No `static_cast`, `reinterpret_cast`, `dynamic_cast` — use C casts
- No new `.cpp` files added — all new code must be `.c`
- `bool`/`true`/`false` used directly (C23 keywords, no `#include <stdbool.h>` needed)
- `nullptr` used for pointer null (C23 keyword) — not `NULL` in new code
- `[[nodiscard]]` on functions returning `CassError` or allocated pointers
- `zend_object` is the **last** field in every internal struct (required for `XtOffsetOf`)

## 2. Parameter Parsing

- Uses `ZEND_PARSE_PARAMETERS_START` / `ZEND_PARSE_PARAMETERS_END` — never `zend_parse_parameters()`
- `ZEND_PARSE_PARAMETERS_NONE()` for zero-parameter methods
- Optional params declared with `Z_PARAM_OPTIONAL` before optional `Z_PARAM_*` macros
- No bare `zend_parse_parameters_ex()` or `zend_parse_parameters_none_ex()` calls

## 3. Object Fetch

- Object fetch via `XtOffsetOf` arithmetic helper (pattern: `(MyStruct *)((char *)obj - XtOffsetOf(MyStruct, zendObject))`)
- Macro wraps the fetch: `PHP_DRIVER_GET_<TYPE>(zval_p)` calls the helper — not inline everywhere
- `ZEND_THIS` used instead of `getThis()` in new code where both work

## 4. Memory Management

- `zend_string *` released with `zend_string_release()` — never `efree()` or `free()`
- Zend objects released with `zend_object_release(&obj->zendObject)` — not `efree()`
- `ZVAL_UNDEF` to mark unset zvals; `Z_ISUNDEF()` to test them
- `zval_ptr_dtor()` called before overwriting a live zval that may hold a reference
- `emalloc`/`efree` for request-lifetime allocations; `pemalloc`/`pefree(ptr, 1)` for persistent
- No `malloc`/`free` except when calling external C libraries

## 5. Handler Registration

- `Handlers.c` initialises handlers with `memcpy` from `zend_get_std_object_handlers()`
- `handlers.offset = XtOffsetOf(MyStruct, zendObject)` set
- `handlers.free_obj` set — releases all owned resources
- `handlers.get_gc` set — returns table of zvals that hold PHP references (for GC)
- `handlers.get_properties` set if the object exposes readable properties
- `handlers.compare` set (returns 1 for different class, compares handle for same)
- `ce->create_object` assigned to the constructor function in the `php_driver_define_*` function

## 6. Stub / Arginfo

- Every public class has a `*.stub.php` with `/** @generate-class-entries */`
- `*_arginfo.h` is included inside `BEGIN_EXTERN_C()` / `END_EXTERN_C()` block
- No manual `ZEND_BEGIN_ARG_INFO_EX` / `PHP_ME` / `PHP_FE_END` in new code
- Stub hash comment in arginfo matches current stub content (stale = regenerate)

## 7. Error Reporting

- Validation errors use `throw_invalid_argument(zval *, "param_name", "expected")` — not `php_error_docref`
- `ASSERT_SUCCESS(rc)` used for `cass_*` calls that return `CassError` where failure is fatal
- Where failure is recoverable, check `rc != CASS_OK` and emit `E_WARNING` with `php_error_docref`

## 8. Fluent Interface (Builder pattern)

- Every builder method ends with `RETURN_ZVAL(ZEND_THIS, 1, 0)` — not `RETURN_OBJ` or manual copy
- No `RETURN_NULL()` on builder methods unless intentional (breaks chaining)

## 9. Thread Safety

- No module-level mutable globals without `PHP_DRIVER_G()` macro wrapper (ZTS-safe globals)
- No static local variables holding PHP values or pointers to Zend memory

Report all findings. If a section is clean, say "✓ clean". End with a one-line verdict: PASS, NEEDS FIXES (n issues), or CRITICAL (memory/crash risk).
