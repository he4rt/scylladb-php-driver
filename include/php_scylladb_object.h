#pragma once

/*
 * Generic helpers replacing ZendCPP::ObjectFetch / ZendCPP::Allocate.
 *
 * Pulled into a standalone header so it can be included by other public
 * headers (SSLOptions/, RetryPolicy/, DateTime/, …) without dragging the
 * heavyweight php_scylladb_types.h transitive closure with it.
 *
 * PHP_SCYLLADB_OBJ_FETCH(T, obj):
 *   Recover a pointer to the user struct from the embedded zend_object*.
 *
 * PHP_SCYLLADB_OBJ_ALLOCATE(T, ce, handlers):
 *   Allocate a user struct (size = sizeof(T), offset = XtOffsetOf(T, zendObject)),
 *   zero-init the user-fields prefix, call zend_object_std_init /
 *   object_properties_init on the embedded zend_object, and wire its
 *   handlers. Returns the user struct.
 */

#include <php.h>

#define PHP_SCYLLADB_OBJ_FETCH(T, obj) \
    ((T *)((char *)(obj) - XtOffsetOf(T, zendObject)))

[[nodiscard]] static zend_always_inline void *php_scylladb_obj_allocate(
    size_t size, size_t obj_offset, zend_class_entry *ce, void *handlers)
{
    /* Mirrors PHP's internal `zend_object_alloc()` (Zend/zend_objects_API.h).
     *
     * Two non-obvious points:
     *
     *   1. `zend_object_properties_size(ce)` evaluates to
     *        sizeof(zval) * (default_properties_count - (USE_GUARDS ? 0 : 1))
     *      For a class with zero default properties and no USE_GUARDS flag,
     *      the `(0 - 1)` underflows to (size_t)-1 ⇒ huge. Combined with
     *      `size` in `emalloc(size + props)`, the modular wrap cancels out
     *      the trailing zval slot that `sizeof(zend_object)` already
     *      includes (via the `properties_table[1]` flex placeholder), so
     *      the total allocation is correct.
     *
     *   2. Because the trailing zval slot is NOT part of the user fields,
     *      we must NOT memset across the full `size` — that would write
     *      `sizeof(zval)` bytes past the (true, post-wrap) allocation, and
     *      glibc fortify on Linux flags it as a buffer overflow. We zero
     *      only the user-fields prefix `[0, obj_offset)` and let
     *      `zend_object_std_init` + `object_properties_init` initialize
     *      the embedded zend_object and properties trailer.
     */
    void *self = emalloc(size + zend_object_properties_size(ce));
    memset(self, 0, obj_offset);
    zend_object *zo = (zend_object *)((char *)self + obj_offset);
    zend_object_std_init(zo, ce);
    object_properties_init(zo, ce);
    zo->handlers = (zend_object_handlers *)handlers;
    return self;
}

#define PHP_SCYLLADB_OBJ_ALLOCATE(T, ce, handlers) \
    ((T *)php_scylladb_obj_allocate(sizeof(T), XtOffsetOf(T, zendObject), (ce), (handlers)))
