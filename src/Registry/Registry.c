/*
 * Class registry — see Registry.h for the contract.
 */

#include "Registry.h"

#include <string.h>

#include <Zend/zend.h>
#include <Zend/zend_API.h>

#define MAX_DEPS_PER_CLASS 8

/* Singly linked list of all descriptors registered at .so load time.
 * Constructors run single-threaded during dlopen, so no locking needed. */
static php_scylladb_class_descriptor_t *registry_head = nullptr;

void php_scylladb_class_registry_add(php_scylladb_class_descriptor_t *d) {
    d->next       = registry_head;
    d->registered = false;
    registry_head = d;
}

static php_scylladb_class_descriptor_t *registry_find(const char *name) {
    for (php_scylladb_class_descriptor_t *d = registry_head; d; d = d->next) {
        if (strcmp(d->name, name) == 0) {
            return d;
        }
    }
    return nullptr;
}

/* Resolve a single FQN to a zend_class_entry — either from the registry (if
 * the parent is also registry-owned and already registered) or via Zend's
 * class table (for SPL/external classes).
 * Returns:
 *   nullptr + *deferred = true  — registry-owned but not yet registered, retry later
 *   nullptr + *deferred = false — unresolved external; caller errors out
 *   ce*                          — resolved
 */
static zend_class_entry *resolve_dep(const char *fqn, bool *deferred) {
    *deferred = false;
    php_scylladb_class_descriptor_t *p = registry_find(fqn);
    if (p != nullptr) {
        if (!p->registered) {
            *deferred = true;
            return nullptr;
        }
        return *(p->ce_out);
    }
    zend_string *lookup = zend_string_init(fqn, strlen(fqn), 0);
    zend_class_entry *ce = zend_lookup_class(lookup);
    zend_string_release(lookup);
    return ce;
}

void php_scylladb_class_registry_minit(void) {
    /* Kahn-style topological registration. Each pass registers every node
     * whose deps are all resolved. Stops when a full pass makes no progress.
     * O(n²) worst case but n is small (low hundreds) and runs exactly once
     * per process. */
    bool progress = true;
    while (progress) {
        progress = false;
        for (php_scylladb_class_descriptor_t *d = registry_head; d; d = d->next) {
            if (d->registered) {
                continue;
            }

            zend_class_entry *resolved[MAX_DEPS_PER_CLASS] = { nullptr };
            size_t n_deps = 0;
            bool defer = false;

            if (d->deps != nullptr) {
                for (size_t i = 0; d->deps[i] != nullptr; i++) {
                    if (i >= MAX_DEPS_PER_CLASS) {
                        zend_error_noreturn(E_CORE_ERROR,
                            "scylladb registry: class '%s' has more than %d dependencies — raise MAX_DEPS_PER_CLASS",
                            d->name, MAX_DEPS_PER_CLASS);
                    }
                    bool dep_deferred = false;
                    zend_class_entry *ce = resolve_dep(d->deps[i], &dep_deferred);
                    if (dep_deferred) {
                        defer = true;
                        break;
                    }
                    if (ce == nullptr) {
                        zend_error_noreturn(E_CORE_ERROR,
                            "scylladb registry: class '%s' declares dep '%s' which is neither registered nor known to Zend",
                            d->name, d->deps[i]);
                    }
                    resolved[i] = ce;
                    n_deps = i + 1;
                }
            }

            if (defer) {
                continue; /* try again after a registry-owned dep registers */
            }

            zend_class_entry *ce = d->register_(n_deps > 0 ? resolved : nullptr);
            if (ce == nullptr) {
                zend_error_noreturn(E_CORE_ERROR,
                    "scylladb registry: register_ for '%s' returned NULL", d->name);
            }
            *(d->ce_out)  = ce;
            d->registered = true;
            progress      = true;
        }
    }

    /* Anything still un-registered means an unresolvable dependency. */
    for (php_scylladb_class_descriptor_t *d = registry_head; d; d = d->next) {
        if (!d->registered) {
            zend_error_noreturn(E_CORE_ERROR,
                "scylladb registry: class '%s' could not be registered (cyclic or missing registry-owned dep)",
                d->name);
        }
    }
}
