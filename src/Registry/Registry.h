#pragma once

/*
 * Self-registering class descriptors.
 *
 * Each class file declares a descriptor at file scope via
 * PHP_SCYLLADB_REGISTER_CLASS(...) or PHP_SCYLLADB_REGISTER_CLASS_DEPS(...).
 * A constructor function appends it to a global linked list at .so load time
 * (before MINIT). During MINIT, php_scylladb_class_registry_minit() walks the
 * list, topologically sorts by declared dependencies (FQN strings), resolves
 * each dep to a zend_class_entry*, and invokes the descriptor's register_
 * callback in dependency order.
 *
 * Dependencies that are not themselves registry-owned (e.g. SPL classes) are
 * looked up via zend_lookup_class — so cross-extension parents work too.
 *
 * The payoff: no central define_*() list, no order-dependent MINIT,
 * dependencies are explicit by FQN string, missing/cyclic deps fail loudly at
 * MINIT with a clear message naming the culprit.
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <Zend/zend_API.h>

/* Register callback. `deps` is a parallel array to the descriptor's `deps`
 * field, with each FQN resolved to its zend_class_entry*. Length = however
 * many strings were declared (no terminator slot exposed). When the
 * descriptor was declared with no deps, `deps` may be NULL. */
typedef zend_class_entry *(*php_scylladb_class_register_fn)(zend_class_entry *const *deps);

typedef struct php_scylladb_class_descriptor {
    const char                          *name;        /* FQN, e.g. "Cassandra\\RetryPolicy\\DefaultPolicy" */
    const char *const                   *deps;        /* NULL-terminated FQN array, or NULL for no deps */
    zend_class_entry                   **ce_out;      /* where to publish the resolved ce */
    php_scylladb_class_register_fn       register_;
    struct php_scylladb_class_descriptor *next;       /* internals */
    bool                                 registered;
} php_scylladb_class_descriptor_t;

void php_scylladb_class_registry_add(php_scylladb_class_descriptor_t *d);
void php_scylladb_class_registry_minit(void);

/*
 * Declare a class with zero or one parent.
 *
 *   slug         — unique C identifier suffix
 *   _name        — FQN string literal
 *   _ce_out      — &php_scylladb_*_ce
 *   _parent      — FQN string literal, or NULL/nullptr for no parent
 *   _register_fn — zend_class_entry *(*)(zend_class_entry *const *deps).
 *                  For zero-parent it can ignore deps; for single-parent
 *                  read deps[0].
 */
#define PHP_SCYLLADB_REGISTER_CLASS(slug, _name, _ce_out, _parent, _register_fn) \
    static const char *const scylladb_cls_##slug##_deps[] = { (_parent), nullptr }; \
    static php_scylladb_class_descriptor_t scylladb_cls_##slug = {              \
        .name       = (_name),                                                  \
        .deps       = ((_parent) == nullptr ? nullptr : scylladb_cls_##slug##_deps), \
        .ce_out     = (_ce_out),                                                \
        .register_  = (_register_fn),                                           \
        .next       = nullptr,                                                  \
        .registered = false,                                                    \
    };                                                                           \
    __attribute__((constructor))                                                \
    static void scylladb_cls_register_##slug##_ctor(void) {                     \
        php_scylladb_class_registry_add(&scylladb_cls_##slug);                  \
    }

/*
 * Declare a class with N (>=1) dependencies. `_deps_array` must be a
 * NULL-terminated `static const char *const xxx[]` defined in the same file.
 * register_fn reads deps[0], deps[1], … in the same order.
 *
 * Use this for classes that extend one class AND implement an interface, or
 * implement multiple interfaces. Example: Bigint implements Value, Numeric.
 */
#define PHP_SCYLLADB_REGISTER_CLASS_DEPS(slug, _name, _ce_out, _deps_array, _register_fn) \
    static php_scylladb_class_descriptor_t scylladb_cls_##slug = {              \
        .name       = (_name),                                                  \
        .deps       = (_deps_array),                                            \
        .ce_out     = (_ce_out),                                                \
        .register_  = (_register_fn),                                           \
        .next       = nullptr,                                                  \
        .registered = false,                                                    \
    };                                                                           \
    __attribute__((constructor))                                                \
    static void scylladb_cls_register_##slug##_ctor(void) {                     \
        php_scylladb_class_registry_add(&scylladb_cls_##slug);                  \
    }

#ifdef __cplusplus
} /* extern "C" */
#endif
