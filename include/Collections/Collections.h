#pragma once

#include <php.h>

BEGIN_EXTERN_C()

typedef struct {
  zval type;
  zend_array values;
  zend_object zendObject;
} php_scylladb_collection;

static zend_always_inline php_scylladb_collection *php_driver_collection_object_fetch(
    zend_object *obj) {
  return (php_scylladb_collection *)((char *)obj -
                                     ((size_t)(&(((php_scylladb_collection *)0)->zendObject))));
}

END_EXTERN_C()

zend_always_inline void php_driver_collection_add(php_scylladb_collection *collection,
                                                  zval *object) {
  (void)zend_hash_next_index_insert(&collection->values, object);
  Z_TRY_ADDREF_P(object);
}
