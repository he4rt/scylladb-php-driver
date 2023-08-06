/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Collection.h"

#include <php_driver.h>
#include <php_driver_types.h>
#include <util/collections.h>
#include <util/hash.h>
#include <util/types.h>

#include <ZendCPP/ZendCPP.hpp>
#include <optional>
BEGIN_EXTERN_C()
#include "Collection_arginfo.h"

zend_class_entry *php_driver_collection_ce = nullptr;
static php_driver_value_handlers php_driver_collection_handlers;

void php_driver_collection_add(php_driver_collection *collection, zval *object) {
  PHP5TO7_ZEND_HASH_NEXT_INDEX_INSERT(&collection->values, object, sizeof(zval *));
  Z_TRY_ADDREF_P(object);
  collection->dirty = 1;
}

static int php_driver_collection_del(php_driver_collection *collection, ulong index) {
  if (zend_hash_index_del(&collection->values, index) == SUCCESS) {
    collection->dirty = 1;
    return 1;
  }

  return 0;
}

static zval *php_driver_collection_get(php_driver_collection *collection, ulong index) {
  zend_array *arr = &collection->values;

  if (HT_FLAGS(arr) & HASH_FLAG_PACKED) [[likely]] {
    if ((zend_ulong)(index) < (zend_ulong)(arr)->nNumUsed) [[likely]] {
      auto val = &arr->arPacked[index];

      if (Z_TYPE_P(val) == IS_UNDEF) [[unlikely]] {
        return nullptr;
      }

      return val;
    }
  }

  auto val = _zend_hash_index_find(arr, index);
  if (val == nullptr) [[unlikely]] {
    return val;
  }

  return nullptr;
}

static std::optional<int> php_driver_collection_find(php_driver_collection *collection,
                                                     zval *object) {
  zend_ulong num_key;
  zval *current;
  ZEND_HASH_FOREACH_NUM_KEY_VAL(&collection->values, num_key, current) {
    zval compare;
    if (is_identical_function(&compare, object, current) != SUCCESS) {
      zend_throw_error(zend_ce_error, "Failed to use === operator to compare values");
      return std::nullopt;
    }

    if (Z_TYPE(compare) == IS_TRUE) {
      return num_key;
    }
  }
  PHP5TO7_ZEND_HASH_FOREACH_END(&collection->values);

  return -1;
}

ZEND_METHOD(Cassandra_Collection, __construct) {
  php_driver_collection *self;
  zval *type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &type) == FAILURE) return;

  self = PHP_DRIVER_GET_COLLECTION(getThis());

  if (Z_TYPE_P(type) == IS_STRING) {
    CassValueType value_type;
    if (!php_driver_value_type(Z_STRVAL_P(type), &value_type)) return;
    self->type = php_driver_type_collection_from_value_type(value_type);
  } else if (Z_TYPE_P(type) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(type), php_driver_type_ce)) {
    if (!php_driver_type_validate(type, "type")) {
      return;
    }
    self->type = php_driver_type_collection(type);
    Z_ADDREF_P(type);
  } else {
    INVALID_ARGUMENT(type, "a string or an instance of " PHP_DRIVER_NAMESPACE "\\Type");
  }
}

ZEND_METHOD(Cassandra_Collection, type) {
  php_driver_collection *self = PHP_DRIVER_GET_COLLECTION(getThis());
  RETURN_ZVAL(&self->type, 1, 0);
}

ZEND_METHOD(Cassandra_Collection, values) {
  auto *collection = ZendCPP::ObjectFetch<php_driver_collection>(getThis());
  RETURN_ARR(zend_array_dup(&collection->values));
}

ZEND_METHOD(Cassandra_Collection, add) {
  php_driver_collection *self = NULL;
  zval *args = NULL;
  int argc = 0, i;
  php_driver_type *type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) return;

  self = PHP_DRIVER_GET_COLLECTION(getThis());
  type = PHP_DRIVER_GET_TYPE(&self->type);

  for (i = 0; i < argc; i++) {
    if (Z_TYPE_P(&args[i]) == IS_NULL) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Invalid value: null is not supported inside collections");
      RETURN_FALSE;
    }

    if (!php_driver_validate_object(&args[i], &type->data.collection.value_type)) {
      RETURN_FALSE;
    }
  }

  for (i = 0; i < argc; i++) {
    php_driver_collection_add(self, &args[i]);
  }

  RETVAL_LONG(zend_hash_num_elements(&self->values));
}

ZEND_METHOD(Cassandra_Collection, remove) {
  zend_long index;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(index)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());

  if (php_driver_collection_del(collection, (ulong)index)) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}

ZEND_METHOD(Cassandra_Collection, get) {
  zend_long key;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(key)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  auto self = ZendCPP::ObjectFetch<php_driver_collection>(getThis());

  if (auto value = php_driver_collection_get(self, key); value != nullptr) {
    RETURN_ZVAL(value, 1, 0);
  }

  zend_throw_exception(php_driver_range_exception_ce, "Index out of bounds", 0);
  RETURN_THROWS();
}

ZEND_METHOD(Cassandra_Collection, find) {
  zval *object;

  // clang-format off
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(object)
  ZEND_PARSE_PARAMETERS_END();
  // clang-format on

  auto *collection = PHP_DRIVER_GET_COLLECTION(getThis());

  if (auto idx = php_driver_collection_find(collection, object); idx) {
    if (idx.value() >= 0) {
      RETURN_LONG(idx.value());
    } else {
      zend_throw_exception(php_driver_range_exception_ce, "Value not found", 0);
    }
  }

  RETURN_THROWS();
}

ZEND_METHOD(Cassandra_Collection, count) {
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  RETURN_LONG(zend_hash_num_elements(&collection->values));
}

ZEND_METHOD(Cassandra_Collection, current) {
  zval *current;
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());

  if (PHP5TO7_ZEND_HASH_GET_CURRENT_DATA(&collection->values, current)) {
    RETURN_ZVAL(current, 1, 0);
  }
}

ZEND_METHOD(Cassandra_Collection, key) {
  zend_ulong num_key;
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  if (PHP5TO7_ZEND_HASH_GET_CURRENT_KEY(&collection->values, NULL, &num_key) == HASH_KEY_IS_LONG) {
    RETURN_LONG(num_key);
  }
}

ZEND_METHOD(Cassandra_Collection, next) {
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  zend_hash_move_forward(&collection->values);
}

ZEND_METHOD(Cassandra_Collection, valid) {
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  RETURN_BOOL(zend_hash_has_more_elements(&collection->values) == SUCCESS);
}

ZEND_METHOD(Cassandra_Collection, rewind) {
  php_driver_collection *collection = PHP_DRIVER_GET_COLLECTION(getThis());
  zend_hash_internal_pointer_reset(&collection->values);
}

static HashTable *php_driver_collection_gc(zend_object *object, zval **table, int *n) {
  *table = nullptr;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *php_driver_collection_properties(zend_object *object) {
  auto *self = ZendCPP::ObjectFetch<php_driver_collection>(object);
  HashTable *props = zend_std_get_properties(object);

  zend_hash_str_update(props, ZEND_STRS("type"), &self->type);
  Z_ADDREF(self->type);

  zval values;
  ZVAL_ARR(&values, zend_array_dup(&self->values));
  zend_hash_str_update(props, ZEND_STRS("values"), &values);
  return props;
}

static int php_driver_collection_compare(zval *obj1, zval *obj2) {
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2)

  /* different classes */
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1;
  }

  auto *collection1 = ZendCPP::ObjectFetch<php_driver_collection>(obj1);
  auto *collection2 = ZendCPP::ObjectFetch<php_driver_collection>(obj2);

  auto *type1 = ZendCPP::ObjectFetch<php_driver_type>(&collection1->type);
  auto *type2 = ZendCPP::ObjectFetch<php_driver_type>(&collection2->type);

  if (auto result = php_driver_type_compare(type1, type2); result != 0) {
    return result;
  }

  auto count1 = zend_hash_num_elements(&collection1->values);
  auto count2 = zend_hash_num_elements(&collection2->values);

  if (count1 != count2) {
    return count1 < count2 ? -1 : 1;
  }

  for (uint32_t i = 0; i < count1; i++) {
    auto current1 = _zend_hash_index_find(&collection1->values, i);
    auto current2 = _zend_hash_index_find(&collection2->values, i);

    if (auto result = php_driver_value_compare(current1, current2); result != 0) {
      return result;
    }
  }

  return 0;
}

static uint32_t php_driver_collection_hash_value(zval *obj) {
  auto *self = ZendCPP::ObjectFetch<php_driver_collection>(obj);

  if (!self->dirty) {
    return self->hashv;
  }

  zval *current = nullptr;
  uint32_t hashv = 0;

  ZEND_HASH_FOREACH_VAL(&self->values, current) {
    hashv = php_driver_combine_hash(hashv, php_driver_value_hash(current));
  }
  ZEND_HASH_FOREACH_END();

  self->hashv = hashv;
  self->dirty = 0;

  return hashv;
}

static void php_driver_collection_free(zend_object *object) {
  auto *self = ZendCPP::ObjectFetch<php_driver_collection>(object);
  zend_hash_destroy(&self->values);
  if (!Z_ISUNDEF(self->type)) {
    zval_ptr_dtor(&self->type);
    ZVAL_UNDEF(&self->type);
  }
}

static zend_object *php_driver_collection_new(zend_class_entry *ce) {
  auto *self = ZendCPP::Allocate<php_driver_collection>(ce, &php_driver_collection_handlers);
  _zend_hash_init(&self->values, 0, ZVAL_PTR_DTOR, false);
  self->dirty = 1;
  ZVAL_UNDEF(&self->type);

  return &self->zendObject;
}

void php_driver_define_Collection(zend_class_entry *value_interface) {
  php_driver_collection_ce =
      register_class_Cassandra_Collection(value_interface, zend_ce_countable, zend_ce_iterator);
  php_driver_collection_ce->create_object = php_driver_collection_new;

  ZendCPP::InitHandlers<php_driver_collection>(&php_driver_collection_handlers);

  php_driver_collection_handlers.std.get_properties = php_driver_collection_properties;
  php_driver_collection_handlers.std.get_gc = php_driver_collection_gc;
  php_driver_collection_handlers.std.compare = php_driver_collection_compare;
  php_driver_collection_handlers.hash_value = php_driver_collection_hash_value;
  php_driver_collection_handlers.std.free_obj = php_driver_collection_free;
  php_driver_collection_handlers.std.clone_obj = nullptr;
}
END_EXTERN_C()