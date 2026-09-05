#include <php.h>
#include <php_scylladb.h>
#include <php_scylladb_types.h>

#include "DefaultClusterHandlers.h"

extern zend_object_handlers php_scylladb_default_cluster_handlers;

HashTable *php_scylladb_default_cluster_properties(zend_object *object)
{
    HashTable *props = zend_std_get_properties(object);

    return props;
}

int php_scylladb_default_cluster_compare(zval *obj1, zval *obj2)
{
    PHP_SCYLLADB_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}

void php_scylladb_default_cluster_free(zend_object *object)
{
    auto self = php_scylladb_cluster_object_fetch(object);

    if (!self->persist && self->cluster)
    {
        cass_cluster_free(self->cluster);
    }

    if (!Z_ISUNDEF(self->default_timeout))
    {
        zval_ptr_dtor(&self->default_timeout);
        ZVAL_UNDEF(&self->default_timeout);
    }

    zend_object_std_dtor(object);
}

zend_object *php_scylladb_default_cluster_new(zend_class_entry *ce)
{
    php_scylladb_cluster *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_cluster, ce, &php_scylladb_default_cluster_handlers);

    self->cluster = nullptr;
    self->default_consistency = PHP_SCYLLADB_DEFAULT_CONSISTENCY;
    self->default_page_size = 5000;
    self->persist = cass_false;
    self->cache_key = 0;

    ZVAL_UNDEF(&self->default_timeout);

    return &self->zendObject;
}
