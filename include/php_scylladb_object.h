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
 *   zero-init it, call zend_object_std_init / object_properties_init on the
 *   embedded zend_object, and wire its handlers. Returns the user struct.
 */

#include <php.h>

#define PHP_SCYLLADB_OBJ_FETCH(T, obj) \
    ((T *)((char *)(obj) - XtOffsetOf(T, zendObject)))

static zend_always_inline void *php_scylladb_obj_allocate(
    size_t size, size_t obj_offset, zend_class_entry *ce, void *handlers)
{
    void *self = emalloc(size + zend_object_properties_size(ce));
    memset(self, 0, size);
    zend_object *zo = (zend_object *)((char *)self + obj_offset);
    zend_object_std_init(zo, ce);
    if (zend_object_properties_size(ce) > 0) {
        object_properties_init(zo, ce);
    }
    zo->handlers = (zend_object_handlers *)handlers;
    return self;
}

#define PHP_SCYLLADB_OBJ_ALLOCATE(T, ce, handlers) \
    ((T *)php_scylladb_obj_allocate(sizeof(T), XtOffsetOf(T, zendObject), (ce), (handlers)))
