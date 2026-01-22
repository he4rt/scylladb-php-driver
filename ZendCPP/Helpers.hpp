#pragma once

#include <php.h>
#include <zend_API.h>
#include <zend_types.h>

namespace ZendCPP {

// ============================================================================
// Return Value Helper
// ============================================================================

/**
 * Helper class for managing function return values
 */
class ReturnValue {
 public:
  explicit ReturnValue(zval* return_value) noexcept : rv_(return_value) {}

  void SetNull() noexcept {
    ZVAL_NULL(rv_);
  }

  void SetBool(bool value) noexcept {
    ZVAL_BOOL(rv_, value);
  }

  void SetTrue() noexcept {
    ZVAL_TRUE(rv_);
  }

  void SetFalse() noexcept {
    ZVAL_FALSE(rv_);
  }

  void SetLong(zend_long value) noexcept {
    ZVAL_LONG(rv_, value);
  }

  void SetDouble(double value) noexcept {
    ZVAL_DOUBLE(rv_, value);
  }

  void SetString(const char* str) noexcept {
    ZVAL_STRING(rv_, str);
  }

  void SetString(const char* str, size_t len) noexcept {
    ZVAL_STRINGL(rv_, str, len);
  }

  void SetString(zend_string* str) noexcept {
    ZVAL_STR(rv_, str);
  }

  void SetStringCopy(zend_string* str) noexcept {
    ZVAL_STR_COPY(rv_, str);
  }

  void SetArray(HashTable* ht) noexcept {
    ZVAL_ARR(rv_, ht);
  }

  void SetObject(zend_object* obj) noexcept {
    ZVAL_OBJ(rv_, obj);
  }

  void SetResource(zend_resource* res) noexcept {
    ZVAL_RES(rv_, res);
  }

  void SetCopy(zval* value) noexcept {
    ZVAL_COPY(rv_, value);
  }

  void SetCopyValue(zval* value) noexcept {
    ZVAL_COPY_VALUE(rv_, value);
  }

  [[nodiscard]] zval* Get() noexcept {
    return rv_;
  }

 private:
  zval* rv_;
};

/**
 * Helper for working with HashTables
 */
class HashTableHelper {
 public:
  explicit HashTableHelper(HashTable* ht) noexcept : ht_(ht) {}

  /**
   * Add element by numeric key
   */
  void AddIndex(zend_ulong idx, zval* value) noexcept {
    zend_hash_index_update(ht_, idx, value);
  }

  void AddIndexLong(zend_ulong idx, zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_hash_index_update(ht_, idx, &zv);
  }

  void AddIndexString(zend_ulong idx, const char* str) noexcept {
    zval zv;
    ZVAL_STRING(&zv, str);
    zend_hash_index_update(ht_, idx, &zv);
  }

  /**
   * Add element by string key
   */
  void Add(const char* key, zval* value) noexcept {
    zend_hash_str_update(ht_, key, strlen(key), value);
  }

  void Add(zend_string* key, zval* value) noexcept {
    zend_hash_update(ht_, key, value);
  }

  void AddLong(const char* key, zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddString(const char* key, const char* str) noexcept {
    zval zv;
    ZVAL_STRING(&zv, str);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddDouble(const char* key, double value) noexcept {
    zval zv;
    ZVAL_DOUBLE(&zv, value);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddBool(const char* key, bool value) noexcept {
    zval zv;
    ZVAL_BOOL(&zv, value);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  /**
   * Get element
   */
  [[nodiscard]] zval* Find(const char* key) const noexcept {
    return zend_hash_str_find(ht_, key, strlen(key));
  }

  [[nodiscard]] zval* Find(zend_string* key) const noexcept {
    return zend_hash_find(ht_, key);
  }

  [[nodiscard]] zval* FindIndex(zend_ulong idx) const noexcept {
    return zend_hash_index_find(ht_, idx);
  }

  /**
   * Check if key exists
   */
  [[nodiscard]] bool Exists(const char* key) const noexcept {
    return zend_hash_str_exists(ht_, key, strlen(key));
  }

  [[nodiscard]] bool Exists(zend_string* key) const noexcept {
    return zend_hash_exists(ht_, key);
  }

  [[nodiscard]] bool ExistsIndex(zend_ulong idx) const noexcept {
    return zend_hash_index_exists(ht_, idx);
  }

  /**
   * Delete element
   */
  void Delete(const char* key) noexcept {
    zend_hash_str_del(ht_, key, strlen(key));
  }

  void Delete(zend_string* key) noexcept {
    zend_hash_del(ht_, key);
  }

  void DeleteIndex(zend_ulong idx) noexcept {
    zend_hash_index_del(ht_, idx);
  }

  /**
   * Get count
   */
  [[nodiscard]] uint32_t Count() const noexcept {
    return zend_hash_num_elements(ht_);
  }

  /**
   * Clear all elements
   */
  void Clear() noexcept {
    zend_hash_clean(ht_);
  }

  [[nodiscard]] HashTable* Get() noexcept {
    return ht_;
  }

 private:
  HashTable* ht_;
};

} // namespace ZendCPP
