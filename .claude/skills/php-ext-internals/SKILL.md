---
name: php-ext-internals
description: |
  Comprehensive Zend Engine API reference for writing PHP 8.x extensions in C23.
  Covers module lifecycle, parameter parsing, zvals, zend_string, HashTable, object
  handlers, memory management, globals, INI settings, arginfo/stub workflow, and
  PHP 8 breakage traps derived from real codebase audit findings.
  Use whenever implementing or reviewing PHP extension C code.
allowed-tools: Read Grep Glob Bash
---

You are acting as a PHP Internals expert. Apply the knowledge below precisely when
writing or reviewing C23 extension code. Prefer source reality over memory:
when unsure about a specific macro signature, grep `php/8.5-debug-nts/src/Zend/`.

---

## 1. Module Entry & Lifecycle

Every extension exposes one `zend_module_entry` with a `get_module()` symbol:

```c
#include <php.h>

// Per-request global struct (ZTS-safe) — define before module entry
ZEND_BEGIN_MODULE_GLOBALS(myext)
    zend_long     request_count;
    zend_string  *last_error;
ZEND_END_MODULE_GLOBALS(myext)

ZEND_DECLARE_MODULE_GLOBALS(myext)

// ZTS-safe accessor — always use this, never the struct directly
#define MYEXT_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(myext, v)

// Lifecycle callbacks
PHP_GINIT_FUNCTION(myext) {   // called once at startup to zero globals
    myext_globals->request_count = 0;
    myext_globals->last_error    = nullptr;
}

PHP_MINIT_FUNCTION(myext) {   // module init — register classes, constants, INI
    REGISTER_INI_ENTRIES();
    php_driver_define_MyClass();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(myext) {
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_RINIT_FUNCTION(myext) {   // request init — reset per-request state
    MYEXT_G(request_count)++;
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(myext) {
    if (MYEXT_G(last_error)) {
        zend_string_release(MYEXT_G(last_error));
        MYEXT_G(last_error) = nullptr;
    }
    return SUCCESS;
}

PHP_MINFO_FUNCTION(myext) {
    php_info_print_table_start();
    php_info_print_table_row(2, "MyExt support", "enabled");
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}

static const zend_function_entry myext_functions[] = {
    PHP_FE(my_function, arginfo_my_function)
    PHP_FE_END
};

zend_module_entry myext_module_entry = {
    STANDARD_MODULE_HEADER,
    "myext",
    myext_functions,
    PHP_MINIT(myext),
    PHP_MSHUTDOWN(myext),
    PHP_RINIT(myext),
    PHP_RSHUTDOWN(myext),
    PHP_MINFO(myext),
    PHP_MYEXT_VERSION,
    PHP_MODULE_GLOBALS(myext),
    PHP_GINIT(myext),
    nullptr,  // gshutdown
    nullptr,  // post_deactivate
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_MYEXT
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(myext)
#endif
```

**Lifecycle order (startup → shutdown):**
`GINIT → MINIT → [RINIT → RSHUTDOWN]* → MSHUTDOWN → GSHUTDOWN`

---

## 2. Stub → Arginfo Workflow (preferred over manual arginfo)

**Never write `ZEND_BEGIN_ARG_INFO_EX` by hand.** Write a stub, generate arginfo.

### Write stub (`MyClass.stub.php`)

```php
<?php
/** @generate-class-entries */
declare(strict_types=1);
namespace Cassandra\MyModule {
    /**
     * @strict-properties
     */
    final class MyClass {
        public function withPort(int $port): static {}
        public function withContactPoints(string ...$hosts): static {}
        public function getPort(): int {}
        public function getLabel(): ?string {}
    }
}
```

### Generate (`_arginfo.h`)

```bash
php php/8.5-debug-nts/src/build/gen_stub.php src/MyModule/MyClass.stub.php
```

Commit both `MyClass.stub.php` and `MyClass_arginfo.h`.

### Include in implementation

```c
BEGIN_EXTERN_C()
#include "MyClass_arginfo.h"   // provides ZEND_METHOD table + register_class_*()
END_EXTERN_C()
```

### Wiring a stub to an existing implementation

When a stub + generated arginfo exist but the `.cpp` still uses legacy patterns:

1. Add `#include "MyClass_arginfo.h"` inside `BEGIN_EXTERN_C()`.
2. Delete all `ZEND_BEGIN_ARG_INFO_EX` / `ZEND_END_ARG_INFO` blocks.
3. Delete the `PHP_ME(...)` table and replace with the stub-generated method table
   (named `class_Cassandra_MyModule_MyClass_methods` in the generated header).
4. Replace `INIT_CLASS_ENTRY(ce, ...)` + `zend_register_internal_class(...)` with
   `register_class_Cassandra_MyModule_MyClass()`.
5. Change every `PHP_METHOD(ClassName, method)` body header to
   `ZEND_METHOD(Cassandra_MyModule_ClassName, method)`.

---

## 3. Function & Method Declaration

```c
// Standalone function (exposed to PHP userland)
PHP_FUNCTION(my_function) { }

// Class method — ALWAYS use ZEND_METHOD, NOT PHP_METHOD
// Classname uses underscores for every namespace separator
ZEND_METHOD(Cassandra_MyModule_MyClass, withPort) { }

// The macro expands to:
// void zim_Cassandra_MyModule_MyClass_withPort(INTERNAL_FUNCTION_PARAMETERS)
// INTERNAL_FUNCTION_PARAMETERS = zend_execute_data *execute_data, zval *return_value
```

**`ZEND_THIS` vs `getThis()`** — always use `ZEND_THIS` in new code:

```c
// WRONG — legacy, 412 occurrences in this codebase:
php_driver_foo_t *self = PHP_DRIVER_GET_FOO(getThis());

// CORRECT — ZEND_THIS = &EX(This), always valid inside a method, no null check needed:
php_driver_foo_t *self = PHP_DRIVER_GET_FOO(ZEND_THIS);
```

---

## 4. Parameter Parsing (modern API only)

```c
ZEND_METHOD(Cassandra_MyModule_MyClass, withPort) {
    zend_long port;

    ZEND_PARSE_PARAMETERS_START(1, 1)    // (min, max)
        Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END();

    if (port < 1 || port > 65535) {
        zend_argument_value_error(1, "must be between 1 and 65535, %ld given", port);
        RETURN_THROWS();
    }

    PHP_DRIVER_GET_MY_CLASS(ZEND_THIS)->port = (uint16_t)port;
    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

// Zero-argument method:
ZEND_PARSE_PARAMETERS_NONE();
```

### Complete Z_PARAM_* macro list

| Macro | C type | PHP type |
|---|---|---|
| `Z_PARAM_BOOL(dest)` | `bool` | bool |
| `Z_PARAM_BOOL_OR_NULL(dest, is_null)` | `bool`, `bool` | ?bool |
| `Z_PARAM_LONG(dest)` | `zend_long` | int |
| `Z_PARAM_LONG_OR_NULL(dest, is_null)` | `zend_long`, `bool` | ?int |
| `Z_PARAM_DOUBLE(dest)` | `double` | float |
| `Z_PARAM_DOUBLE_OR_NULL(dest, is_null)` | `double`, `bool` | ?float |
| `Z_PARAM_STR(dest)` | `zend_string *` | string — **prefer over Z_PARAM_STRING** |
| `Z_PARAM_STR_OR_NULL(dest)` | `zend_string *` | ?string |
| `Z_PARAM_STRING(dest, dest_len)` | `char *`, `size_t` | string — only when you need a raw `char*` |
| `Z_PARAM_ARRAY(dest)` | `zval *` | array |
| `Z_PARAM_ARRAY_OR_NULL(dest)` | `zval *` | ?array |
| `Z_PARAM_ARRAY_HT(dest)` | `HashTable *` | array — use when you only need the HashTable |
| `Z_PARAM_ARRAY_HT_OR_NULL(dest)` | `HashTable *` | ?array |
| `Z_PARAM_OBJ(dest)` | `zend_object *` | object |
| `Z_PARAM_OBJ_OR_NULL(dest)` | `zend_object *` | ?object |
| `Z_PARAM_OBJ_OF_CLASS(dest, ce)` | `zend_object *` | ClassName — replaces `Z_PARAM_OBJECT` + manual instanceof |
| `Z_PARAM_OBJ_OF_CLASS_OR_NULL(dest, ce)` | `zend_object *` | ?ClassName |
| `Z_PARAM_ZVAL(dest)` | `zval *` | mixed — use sparingly; prefer typed variants |
| `Z_PARAM_FUNC(fci, fcc)` | `zend_fcall_info`, `zend_fcall_info_cache` | callable |
| `Z_PARAM_FUNC_OR_NULL(fci, fcc)` | same | ?callable |
| `Z_PARAM_VARIADIC('*', dest, count)` | `zval *`, `uint32_t` | mixed... |
| `Z_PARAM_OPTIONAL` | — | separates required/optional |

**`Z_PARAM_STR` vs `Z_PARAM_STRING`**: `Z_PARAM_STR` gives you `zend_string*` directly — prefer it. Only use `Z_PARAM_STRING` when you genuinely need a raw `char*` (e.g. passing to a C library). Using `Z_PARAM_STRING` then calling `zend_string_init()` on the result is a double-allocation.

---

## 5. Return Value Macros

`RETURN_*` sets `return_value` and **returns from the C function**.
`RETVAL_*` sets `return_value` without returning — use when you need cleanup after.

```c
RETURN_NULL()
RETURN_TRUE  / RETURN_FALSE
RETURN_BOOL(b)
RETURN_LONG(l)
RETURN_DOUBLE(d)
RETURN_STR(zend_string *)       // takes ownership — refcount NOT incremented
RETURN_STR_COPY(zend_string *)  // copies — increments refcount
RETURN_STRING(const char *)     // allocates new zend_string from C literal
RETURN_STRINGL(char *, len)     // same, with explicit length
RETURN_EMPTY_STRING()
RETURN_ARR(HashTable *)         // takes ownership
RETURN_OBJ(zend_object *)       // takes ownership
RETURN_OBJ_COPY(zend_object *)  // increments refcount
RETURN_ZVAL(zv, copy, dtor)     // generic — builder methods: RETURN_ZVAL(ZEND_THIS, 1, 0)
RETURN_THROWS()                 // after setting an exception — do not call after RETURN_*
```

**Fluent builder methods always end with:**
```c
RETURN_ZVAL(ZEND_THIS, 1, 0);
```

---

## 6. Zval Types & Operations

Zvals are **never individually heap-allocated** — they live on the stack or embedded in larger structures.

### Type constants

```c
IS_UNDEF, IS_NULL, IS_FALSE, IS_TRUE,
IS_LONG, IS_DOUBLE, IS_STRING, IS_ARRAY,
IS_OBJECT, IS_RESOURCE, IS_REFERENCE
```

### Initialization macros

```c
ZVAL_UNDEF(&z)
ZVAL_NULL(&z)
ZVAL_BOOL(&z, b)
ZVAL_LONG(&z, l)
ZVAL_DOUBLE(&z, d)
ZVAL_STR(&z, s)           // s: zend_string* — transfers ownership
ZVAL_STR_COPY(&z, s)      // copies — increments refcount
ZVAL_STRING(&z, "literal")
ZVAL_STRINGL(&z, str, len)
ZVAL_ARR(&z, ht)          // transfers ownership of HashTable
ZVAL_EMPTY_ARRAY(&z)      // shared immutable empty array
ZVAL_OBJ(&z, obj)         // transfers ownership
ZVAL_OBJ_COPY(&z, obj)    // increments refcount
```

### Copying & moving

```c
ZVAL_COPY_VALUE(&dst, &src)   // bitwise copy — no refcount change; src must not be reused
ZVAL_COPY(&dst, &src)         // copy + safe Z_TRY_ADDREF; use this in most cases
ZVAL_COPY_DEREF(&dst, &src)   // dereference reference + copy in one step
ZVAL_DEREF(&z)                // follow IS_REFERENCE pointer to inner value (no copy)
```

### Destruction

```c
zval_ptr_dtor(&z)             // decrement refcount + destroy; runs GC cycle check
zval_ptr_dtor_nogc(&z)        // same but skips GC — safe for non-circular data
i_zval_ptr_dtor(&z)           // inlined fast path
```

**Always call `zval_ptr_dtor()` before overwriting a live zval.**

### Type checking & reading

```c
Z_TYPE(z)                // IS_* constant
Z_ISUNDEF(z)             // true when IS_UNDEF
Z_LVAL(z)                // zend_long
Z_DVAL(z)                // double
Z_STR(z)                 // zend_string*
Z_STRVAL(z)              // const char* (ZSTR_VAL of the string)
Z_STRLEN(z)              // size_t
Z_ARR(z)                 // HashTable*
Z_OBJ(z)                 // zend_object*
// _P suffix: dereferences a zval pointer first
Z_LVAL_P(zv)  etc.
```

### References

```c
Z_ISREF_P(zv)             // check if zval is a reference wrapper
ZVAL_DEREF(zv)            // follow the reference (zv now points to inner value)
ZVAL_UNREF(zv)            // unwrap reference — either moves or copies inner value
```

---

## 7. zend_string API

`zend_string` is a refcounted, length-prefixed, NUL-terminated string. Never use raw `char*` for PHP strings.

### Accessors

```c
ZSTR_VAL(s)         // char*  — the string data (NUL-terminated)
ZSTR_LEN(s)         // size_t — byte length (excluding NUL)
ZSTR_HASH(s)        // cached hash (computes if zero)
```

### Allocation

```c
// persistent=false for request-bound, =true for module-lifetime
zend_string *s = zend_string_init("hello", 5, false);
zend_string *s = ZSTR_INIT_LITERAL("hello", false);   // strlen computed at compile time

zend_string *copy = zend_string_copy(s);    // increment refcount only
zend_string *dup  = zend_string_dup(s, false);  // always allocates new buffer
```

### Release

```c
zend_string_release(s);          // decrement refcount; free if reaches 0
zend_string_release_ex(s, persistent);  // explicit persistent flag
// NEVER efree() or free() a zend_string directly
```

### Comparison

```c
zend_string_equals(s1, s2)                    // bool, case-sensitive
zend_string_equals_ci(s1, s2)                 // bool, case-insensitive
zend_string_equals_literal(s, "literal")      // compare against C string literal
zend_string_equals_literal_ci(s, "literal")   // case-insensitive
```

### Interned strings

Interned strings are immutable, deduplicated, and shared across requests. Use for known constant identifiers:

```c
zend_string *key = zend_string_init_interned("status", 6, true);  // true = permanent
// Do NOT release interned strings — they live for the process lifetime
```

### Smart string (building strings dynamically)

```c
#include <ext/standard/php_smart_string.h>

smart_string buf = {0};
smart_string_appends(&buf, "hello ");
smart_string_append_long(&buf, 42);
smart_string_0(&buf);                // NUL-terminate
// Use buf.c (char*), buf.len (size_t)
smart_string_free(&buf);
```

---

## 8. HashTable / Array API

`HashTable` is the underlying type for PHP arrays. Keys are either `zend_string*` (string keys) or `zend_ulong` (integer keys).

### Creation

```c
// Return an array from a method:
array_init(return_value);               // empty array
array_init_size(return_value, 8);       // pre-allocate 8 buckets

// Stand-alone hashtable (rare — prefer array zval):
HashTable *ht = pemalloc(sizeof(HashTable), false);
zend_hash_init(ht, 8, nullptr, ZVAL_PTR_DTOR, false);
```

### Adding values

```c
// String key — all take the array zval:
add_assoc_null(arr, "key");
add_assoc_bool(arr, "key", 1);
add_assoc_long(arr, "key", 42L);
add_assoc_double(arr, "key", 3.14);
add_assoc_string(arr, "key", "value");    // copies the C string
add_assoc_stringl(arr, "key", str, len);
add_assoc_zval(arr, "key", &zv);          // transfers ownership

// Integer (index) key:
add_index_long(arr, 0, 42L);
add_index_string(arr, 0, "value");
add_next_index_long(arr, 42L);            // appends to next slot
add_next_index_string(arr, "value");
add_next_index_zval(arr, &zv);            // transfers ownership — do NOT addref before calling

// Low-level HashTable functions (string key):
zend_hash_add(ht, key_str, &zv);          // fails if key exists
zend_hash_update(ht, key_str, &zv);       // always sets
zend_hash_add_or_update(ht, key_str, &zv, HASH_ADD | HASH_UPDATE);

// Low-level (integer key):
zend_hash_index_add(ht, idx, &zv);
zend_hash_index_update(ht, idx, &zv);
```

### Lookup

```c
zval *val = zend_hash_find(ht, key_str);             // nullptr if missing
zval *val = zend_hash_str_find(ht, "key", sizeof("key")-1);
zval *val = zend_hash_index_find(ht, 0);
// Always NULL-check the result before using it

uint32_t n = zend_hash_num_elements(ht);
```

### Iteration

```c
zval *val;
ZEND_HASH_FOREACH_VAL(ht, val) {
    // use val
} ZEND_HASH_FOREACH_END();

zend_string *key;
zval *val;
ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
    // key may be nullptr for integer-keyed entries
} ZEND_HASH_FOREACH_END();

zend_ulong idx;
zend_string *key;
zval *val;
ZEND_HASH_FOREACH_KEY_VAL(ht, idx, key, val) {
    // key == nullptr means integer key idx
} ZEND_HASH_FOREACH_END();
```

---

## 9. Object System

### Internal struct layout (zendObject must be last)

```c
// in php_driver_types.h:
typedef struct {
    zend_long        port;
    zend_string     *host;
    zval             callback;    // zval fields: mark ZVAL_UNDEF in constructor
    zend_object      zendObject;  // MUST be the last field
} php_driver_my_class_t;

static inline php_driver_my_class_t *php_driver_my_class_fetch(zend_object *obj) {
    return (php_driver_my_class_t *)((char *)obj - XtOffsetOf(php_driver_my_class_t, zendObject));
}
#define PHP_DRIVER_GET_MY_CLASS(zv) php_driver_my_class_fetch(Z_OBJ_P(zv))
```

### Object handlers (`MyClassHandlers.c`)

The handler variable is a plain `zend_object_handlers` — **not** a wrapper struct. Access fields directly, never via `.std.`:

```c
// CORRECT — canonical C23 pattern:
static zend_object_handlers php_driver_my_class_handlers;

void php_driver_initialize_my_class_handlers(void) {
    memcpy(&php_driver_my_class_handlers,
           zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));
    php_driver_my_class_handlers.offset         = XtOffsetOf(php_driver_my_class_t, zendObject);
    php_driver_my_class_handlers.free_obj       = php_driver_my_class_free;
    php_driver_my_class_handlers.get_gc         = php_driver_my_class_gc;
    php_driver_my_class_handlers.get_properties = php_driver_my_class_properties;
    php_driver_my_class_handlers.compare        = php_driver_my_class_compare;
}

// WRONG — legacy wrapper struct pattern (php_driver_value_handlers):
// php_driver_collection_handlers.std.get_gc = ...   // .std. prefix = old union layout
// php_driver_collection_handlers.std.compare_objects = ...  // compare_objects REMOVED in PHP 8
```

### Constructor

```c
zend_object *php_driver_my_class_new(zend_class_entry *ce) {
    php_driver_my_class_t *self =
        ecalloc(1, sizeof(php_driver_my_class_t) + zend_object_properties_size(ce));

    // ecalloc zeroes memory, but be explicit for embedded zvals:
    ZVAL_UNDEF(&self->callback);
    self->port = 9042;

    zend_object_std_init(&self->zendObject, ce);
    object_properties_init(&self->zendObject, ce);
    self->zendObject.handlers = &php_driver_my_class_handlers;
    return &self->zendObject;
}
```

### Destructor

```c
static void php_driver_my_class_free(zend_object *obj) {
    php_driver_my_class_t *self = php_driver_my_class_fetch(obj);

    if (self->host) {
        zend_string_release(self->host);
        self->host = nullptr;
    }
    if (!Z_ISUNDEF(self->callback)) {
        zval_ptr_dtor(&self->callback);
        ZVAL_UNDEF(&self->callback);
    }

    zend_object_std_dtor(obj);    // releases declared properties; must be last
}
```

### GC handler

```c
static HashTable *php_driver_my_class_gc(zend_object *obj, zval **table, int *n) {
    php_driver_my_class_t *self = php_driver_my_class_fetch(obj);
    zend_get_gc_buffer *gc_buf = zend_get_gc_buffer_create();
    zend_get_gc_buffer_add_zval(gc_buf, &self->callback);
    zend_get_gc_buffer_use(gc_buf, table, n);
    return zend_std_get_properties(obj);
}
```

### Properties handler

```c
// CORRECT — initialize zv before each update; no manual Z_ADDREF:
static HashTable *php_driver_my_class_properties(zend_object *obj) {
    php_driver_my_class_t *self = php_driver_my_class_fetch(obj);
    HashTable *props = zend_std_get_properties(obj);

    zval zv;
    ZVAL_LONG(&zv, self->port);
    zend_hash_str_update(props, "port", sizeof("port")-1, &zv);
    if (self->host) {
        ZVAL_STR_COPY(&zv, self->host);   // STR_COPY increments refcount — no separate Z_ADDREF
        zend_hash_str_update(props, "host", sizeof("host")-1, &zv);
    }
    return props;
}

// WRONG — double refcount trap seen in Map.cpp, Tuple.cpp:
// ZVAL_COPY_VALUE(&zv, &self->type);   // no refcount increment
// Z_ADDREF_P(&zv);                     // manual addref
// add_next_index_zval(array, &zv);     // add_next_index_zval transfers ownership — refcount already right
// This leaves refcount one too high. Use ZVAL_COPY (not COPY_VALUE + addref) instead.
```

### Compare handler

```c
static int php_driver_my_class_compare(zval *a, zval *b) {
    if (Z_TYPE_P(a) != IS_OBJECT || Z_TYPE_P(b) != IS_OBJECT
        || Z_OBJCE_P(a) != Z_OBJCE_P(b)) {
        return 1;
    }
    php_driver_my_class_t *left  = PHP_DRIVER_GET_MY_CLASS(a);
    php_driver_my_class_t *right = PHP_DRIVER_GET_MY_CLASS(b);
    return left->port != right->port;
}
// Assign to handlers.compare — never to handlers.compare_objects (removed in PHP 8)
```

### Class registration (in `MyClass.c`)

```c
zend_class_entry *php_driver_my_class_ce = nullptr;

void php_driver_define_MyClass(void) {
    php_driver_my_class_ce = register_class_Cassandra_MyModule_MyClass();
    php_driver_initialize_my_class_handlers();
    php_driver_my_class_ce->create_object = php_driver_my_class_new;
}
```

### Calling PHP from C (call_user_function)

```c
zval retval;
zval args[2];
ZVAL_LONG(&args[0], 42);
ZVAL_STRING(&args[1], "hello");

if (call_user_function(nullptr, nullptr, &callable_zval,
                       &retval, 2, args) == SUCCESS
    && !EG(exception)) {
    zval_ptr_dtor(&retval);
}
zval_ptr_dtor(&args[1]);
```

---

## 10. Memory Management

### Allocators

| Lifetime | Alloc | Free |
|---|---|---|
| Request-bound | `emalloc(n)` | `efree(p)` |
| Request-bound, zeroed | `ecalloc(1, n)` | `efree(p)` |
| Request-bound, safe (overflow check) | `safe_emalloc(n, size, extra)` | `efree(p)` |
| Request-bound, string dup | `estrdup(s)` / `estrndup(s,n)` | `efree(p)` |
| Persistent (survives requests) | `pemalloc(n, 1)` | `pefree(p, 1)` |
| External C library | `malloc(n)` | `free(p)` |

**Never mix allocators.** `emalloc` memory freed with `free()` = heap corruption.

**`ecalloc` argument order**: `ecalloc(count, size)` — for objects always `ecalloc(1, sizeof(T) + zend_object_properties_size(ce))`. Do not swap the arguments.

### Rules

- `zend_string*` → `zend_string_release()` (not `efree`)
- `zend_object*` → `zend_object_release()` or let Zend free via `free_obj`
- Zvals embedded in structs → `zval_ptr_dtor()` before struct free
- `USE_ZEND_ALLOC=0` in env disables ZendMM for valgrind/asan runs

---

## 11. INI Settings

```c
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("myext.connect_timeout", "5000", PHP_INI_ALL,
                      OnUpdateLong, connect_timeout,
                      zend_myext_globals, myext_globals)
    STD_PHP_INI_BOOLEAN("myext.persistent", "1", PHP_INI_SYSTEM,
                        OnUpdateBool, use_persistent,
                        zend_myext_globals, myext_globals)
PHP_INI_END()

// Modifiability levels:
// PHP_INI_USER   — ini_set() allowed
// PHP_INI_PERDIR — .htaccess or httpd.conf
// PHP_INI_SYSTEM — php.ini or httpd.conf only
// PHP_INI_ALL    — any context

// Built-in validators: OnUpdateLong, OnUpdateDouble, OnUpdateBool,
//                      OnUpdateString, OnUpdateStringUnempty
```

---

## 12. Constants & Error Handling

### Constants

```c
PHP_MINIT_FUNCTION(myext) {
    REGISTER_LONG_CONSTANT("MYEXT_OPT_SSL",  1, CONST_CS | CONST_PERSISTENT);
    REGISTER_STRING_CONSTANT("MYEXT_VERSION", PHP_MYEXT_VERSION, CONST_CS | CONST_PERSISTENT);
    return SUCCESS;
}
// CONST_CS = case-sensitive; CONST_PERSISTENT = survives requests
```

### Error reporting

```c
// Throw typed exceptions (preferred for bad arguments):
zend_argument_type_error(1, "must be of type int, %s given", zend_zval_value_name(arg));
zend_argument_value_error(1, "must be positive, %ld given", val);
// Then RETURN_THROWS() — do NOT also call RETURN_NULL/RETURN_FALSE

// PHP warnings/errors (use sparingly, prefer exceptions):
php_error_docref(nullptr, E_WARNING, "could not connect: %s", msg);
php_error_docref(nullptr, E_ERROR,   "fatal: %s", msg);

// Check & re-throw after call_user_function:
if (EG(exception)) { RETURN_THROWS(); }
```

---

## 13. Object Handler Reference Table

All handlers live in `zend_object_handlers` (from `zend_object_handlers.h`).
Only set handlers you actually need; unset ones fall back to std handlers via `memcpy`.

| Handler | Purpose | When to implement |
|---|---|---|
| `free_obj` | Release resources | Always |
| `get_gc` | GC roots | When struct holds zvals/objects |
| `get_properties` | var_dump, print_r | When struct has PHP-visible state |
| `compare` | `==`, `<=>` | When equality is meaningful |
| `clone_obj` | `clone $obj` | When clone must deep-copy state |
| `read_property` | `$obj->prop` | Custom property access |
| `write_property` | `$obj->prop = x` | Custom property assignment |
| `has_property` | `isset($obj->prop)` | Custom isset |
| `unset_property` | `unset($obj->prop)` | Custom unset |
| `read_dimension` | `$obj[$k]` | ArrayAccess-like |
| `write_dimension` | `$obj[$k] = v` | ArrayAccess-like |
| `has_dimension` | `isset($obj[$k])` | ArrayAccess-like |
| `unset_dimension` | `unset($obj[$k])` | ArrayAccess-like |
| `count_elements` | `count($obj)` | Countable |
| `cast_object` | `(string)$obj` etc. | Type coercions |
| `do_operation` | `$a + $b` | Operator overload |
| `get_debug_info` | `var_dump` detail | Custom debug output |

**Removed in PHP 8 — do not use:**
- `compare_objects` — replaced by `compare`. Assigning it writes into unrelated memory.
- `ZEND_ACC_CTOR` flag — removed from `PHP_ME`/`ZEND_ACC_*`. Use `ZEND_ACC_PUBLIC` alone for constructors.

---

## 14. PHP 8 Breakage Traps (patterns that compiled in PHP 7, corrupt or crash in PHP 8)

These are anti-patterns found in this codebase during audit. Treat every occurrence as a bug.

### `compare_objects` — out-of-bounds write

```c
// WRONG — compare_objects does not exist in zend_object_handlers in PHP 8:
handlers.std.compare_objects = my_compare_fn;   // writes past the struct

// CORRECT — only compare exists:
handlers.compare = my_compare_fn;
```

Verified: `grep -n compare /path/to/php/8.5-debug-nts/src/Zend/zend_object_handlers.h` shows only `compare`, no `compare_objects`.

### `.std.` handler prefix — wrong struct type

This codebase has a legacy `php_driver_value_handlers` wrapper:
```c
typedef struct {
    zend_object_handlers std;
    php_driver_value_hash_t hash_value;
} php_driver_value_handlers;
```

Legacy modules declare `static php_driver_value_handlers foo_handlers` and access `foo_handlers.std.get_gc`. New modules must declare `static zend_object_handlers foo_handlers` and access `foo_handlers.get_gc` directly. Never use the wrapper type for new code.

### `ZEND_ACC_CTOR` — removed flag

```c
// WRONG — ZEND_ACC_CTOR was removed in PHP 8:
PHP_ME(Foo, __construct, arginfo__construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)

// CORRECT — just ZEND_ACC_PUBLIC; stub-generated tables handle constructors automatically:
// (Don't write method tables at all — use stub-generated arginfo)
```

### `PHP_METHOD` vs `ZEND_METHOD`

```c
// WRONG — PHP_METHOD uses short unqualified name; causes symbol collisions in namespaced code:
PHP_METHOD(Timeuuid, __construct) { ... }

// CORRECT — ZEND_METHOD uses fully qualified underscore-separated name:
ZEND_METHOD(Cassandra_DateTime_Timeuuid, __construct) { ... }
```

`PHP_METHOD(Cls, method)` expands to `zim_Cls_method`. `ZEND_METHOD(Cassandra_Foo_Bar, method)` expands to `zim_Cassandra_Foo_Bar_method`. The stub-generated method table references the `ZEND_METHOD` symbol — using `PHP_METHOD` with a short name produces an undefined reference at link time.

### `getThis()` — unnecessary null check

```c
// WRONG — getThis() returns NULL outside object context; adds branch that can never trigger in a method:
php_driver_foo_t *self = PHP_DRIVER_GET_FOO(getThis());

// CORRECT — ZEND_THIS = &EX(This), always valid inside ZEND_METHOD:
php_driver_foo_t *self = PHP_DRIVER_GET_FOO(ZEND_THIS);
```

### C++ casts → C casts

```c
// WRONG — C++ casts in .cpp files being ported:
self->persist = static_cast<cass_bool_t>(enabled);
self->consistency = static_cast<CassConsistency>(val);

// CORRECT — C casts:
self->persist      = (cass_bool_t)enabled;
self->consistency  = (CassConsistency)val;
```

### Double-refcount in `get_properties`

```c
// WRONG — ZVAL_COPY_VALUE does not increment refcount; manual Z_ADDREF then
// add_next_index_zval (which transfers ownership) leaves refcount one too high:
ZVAL_COPY_VALUE(&zv, &self->type);
Z_ADDREF_P(&zv);
add_next_index_zval(array, &zv);   // BUG: refcount is now 2 when it should be 1 extra

// CORRECT — ZVAL_COPY increments refcount; add_next_index_zval then consumes one ref:
ZVAL_COPY(&zv, &self->type);
add_next_index_zval(array, &zv);
```

---

## 15. Quick Patterns

### Check argument is an instance of a specific class

```c
zend_object *obj;
ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJ_OF_CLASS(obj, php_driver_my_class_ce)
ZEND_PARSE_PARAMETERS_END();

php_driver_my_class_t *self = php_driver_my_class_fetch(obj);
```

### Return a new object

```c
object_init_ex(return_value, php_driver_my_class_ce);
php_driver_my_class_t *result = PHP_DRIVER_GET_MY_CLASS(return_value);
result->port = 9042;
// return_value is already set — just return
```

### Throw a custom exception

```c
// Declare: extern zend_class_entry *php_driver_invalid_argument_exception_ce;
zend_throw_exception(php_driver_invalid_argument_exception_ce,
                     "Port must be between 1 and 65535", 0);
RETURN_THROWS();
```

### Iterate an array argument

```c
HashTable *ht;
ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY_HT(ht)
ZEND_PARSE_PARAMETERS_END();

zval *item;
ZEND_HASH_FOREACH_VAL(ht, item) {
    ZVAL_DEREF(item);
    if (Z_TYPE_P(item) != IS_STRING) {
        zend_argument_type_error(1, "must contain only strings");
        RETURN_THROWS();
    }
} ZEND_HASH_FOREACH_END();
```

### Thread-safe module globals

```c
// Define in extension header:
ZEND_EXTERN_MODULE_GLOBALS(myext)
#define MYEXT_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(myext, v)

// Access anywhere:
MYEXT_G(connect_timeout) = 5000;
```

### `nullptr` not `NULL` in new C23 code

```c
// WRONG — NULL in C23 extension code:
zend_string *s = NULL;
self->host = NULL;

// CORRECT — nullptr is a C23 keyword:
zend_string *s = nullptr;
self->host = nullptr;
```
