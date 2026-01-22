#pragma once

#include <php.h>
#include <zend_API.h>

#include <string>
#include <type_traits>

namespace ZendCPP {

/**
 * RAII wrapper for zval that automatically handles memory management
 */
class ZVal {
 public:
  ZVal() noexcept { ZVAL_UNDEF(&val_); }

  explicit ZVal(zval* val) noexcept : val_(*val) {}

  inline ~ZVal() noexcept { zval_ptr_dtor(&val_); }

  // Copy operations - properly handle refcounting
  inline ZVal(const ZVal& other) noexcept : val_(other.val_) {
    zval_copy_ctor(&val_);
  }

  inline ZVal& operator=(const ZVal& other) noexcept {
    if (this != &other) {
      zval_ptr_dtor(&val_);
      val_ = other.val_;
      zval_copy_ctor(&val_);
    }
    return *this;
  }

  // Move operations
  ZVal(ZVal&& other) noexcept : val_(other.val_) { ZVAL_UNDEF(&other.val_); }

  ZVal& operator=(ZVal&& other) noexcept {
    if (this != &other) {
      zval_ptr_dtor(&val_);
      val_ = other.val_;
      ZVAL_UNDEF(&other.val_);
    }
    return *this;
  }

  // Setters
  void SetNull() noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_NULL(&val_);
  }

  void SetBool(bool value) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_BOOL(&val_, value);
  }

  void SetLong(zend_long value) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_LONG(&val_, value);
  }

  void SetDouble(double value) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_DOUBLE(&val_, value);
  }

  void SetString(const char* str) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_STRING(&val_, str);
  }

  void SetString(const char* str, size_t len) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_STRINGL(&val_, str, len);
  }

  inline void SetString(zend_string* str) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_STR(&val_, zend_string_copy(str));
  }

  void SetArray(HashTable* ht) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_ARR(&val_, ht);
  }

  void SetObject(zend_object* obj) noexcept {
    zval_ptr_dtor(&val_);
    ZVAL_OBJ(&val_, obj);
  }

  // Getters
  [[nodiscard]] bool IsNull() const noexcept { return Z_TYPE(val_) == IS_NULL; }
  [[nodiscard]] bool IsBool() const noexcept {
    return Z_TYPE(val_) == IS_TRUE || Z_TYPE(val_) == IS_FALSE;
  }
  [[nodiscard]] bool IsLong() const noexcept { return Z_TYPE(val_) == IS_LONG; }
  [[nodiscard]] bool IsDouble() const noexcept { return Z_TYPE(val_) == IS_DOUBLE; }
  [[nodiscard]] bool IsString() const noexcept { return Z_TYPE(val_) == IS_STRING; }
  [[nodiscard]] bool IsArray() const noexcept { return Z_TYPE(val_) == IS_ARRAY; }
  [[nodiscard]] bool IsObject() const noexcept { return Z_TYPE(val_) == IS_OBJECT; }
  [[nodiscard]] bool IsResource() const noexcept { return Z_TYPE(val_) == IS_RESOURCE; }
  [[nodiscard]] bool IsUndef() const noexcept { return Z_TYPE(val_) == IS_UNDEF; }

  [[nodiscard]] bool ToBool() const noexcept { return zend_is_true(&val_); }
  [[nodiscard]] zend_long ToLong() const noexcept {
    return zval_get_long(const_cast<zval*>(&val_));
  }
  [[nodiscard]] double ToDouble() const noexcept {
    return zval_get_double(const_cast<zval*>(&val_));
  }
  [[nodiscard]] zend_string* ToString() const noexcept {
    return zval_get_string(const_cast<zval*>(&val_));
  }

  // Access underlying zval
  [[nodiscard]] zval* Get() noexcept { return &val_; }
  [[nodiscard]] const zval* Get() const noexcept { return &val_; }
  [[nodiscard]] zval* operator->() noexcept { return &val_; }
  [[nodiscard]] const zval* operator->() const noexcept { return &val_; }
  [[nodiscard]] zval& operator*() noexcept { return val_; }
  [[nodiscard]] const zval& operator*() const noexcept { return val_; }

 private:
  zval val_;
};

/**
 * Helper class for creating and managing HashTable arrays
 * Stores HashTable* directly for better performance and flexibility
 */
class ZArray {
 public:
  // Create a regular associative array
  ZArray() noexcept : ht_(zend_new_array(0)), owns_(true) {}

  explicit ZArray(uint32_t size) noexcept : ht_(zend_new_array(size)), owns_(true) {}

  // Create a packed array (indexed, no gaps) for better performance
  static ZArray CreatePacked(uint32_t size = 0) noexcept {
    ZArray arr;
    if (arr.ht_) {
      zend_hash_real_init_packed(arr.ht_);
      if (size > 0) {
        zend_hash_extend(arr.ht_, size, 1);
      }
    }
    return arr;
  }

  ~ZArray() noexcept {
    if (owns_ && ht_) {
      zend_array_destroy(ht_);
    }
  }

  ZArray(const ZArray&) = delete;
  ZArray& operator=(const ZArray&) = delete;

  ZArray(ZArray&& other) noexcept : ht_(other.ht_), owns_(other.owns_) {
    other.ht_ = nullptr;
    other.owns_ = false;
  }

  ZArray& operator=(ZArray&& other) noexcept {
    if (this != &other) {
      if (owns_ && ht_) {
        zend_array_destroy(ht_);
      }
      ht_ = other.ht_;
      owns_ = other.owns_;
      other.ht_ = nullptr;
      other.owns_ = false;
    }
    return *this;
  }

  // Indexed (numeric) additions
  void AddNext(zval* value) noexcept { zend_hash_next_index_insert(ht_, value); }

  void AddNextLong(zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_hash_next_index_insert(ht_, &zv);
  }

  void AddNextDouble(double value) noexcept {
    zval zv;
    ZVAL_DOUBLE(&zv, value);
    zend_hash_next_index_insert(ht_, &zv);
  }

  void AddNextString(const char* str) noexcept {
    zval zv;
    ZVAL_STRING(&zv, str);
    zend_hash_next_index_insert(ht_, &zv);
  }

  void AddNextString(const char* str, size_t len) noexcept {
    zval zv;
    ZVAL_STRINGL(&zv, str, len);
    zend_hash_next_index_insert(ht_, &zv);
  }

  void AddNextString(const std::string& str) noexcept {
    AddNextString(str.c_str(), str.length());
  }

  void AddNextBool(bool value) noexcept {
    zval zv;
    ZVAL_BOOL(&zv, value);
    zend_hash_next_index_insert(ht_, &zv);
  }

  void AddNextNull() noexcept {
    zval zv;
    ZVAL_NULL(&zv);
    zend_hash_next_index_insert(ht_, &zv);
  }

  // Add nested array (HashTable of HashTables)
  void AddNextArray(ZArray&& nested) noexcept {
    zval zv;
    ZVAL_ARR(&zv, nested.Release());
    zend_hash_next_index_insert(ht_, &zv);
  }

  // Associative additions with const char* key
  void AddAssoc(const char* key, zval* value) noexcept {
    zend_hash_str_update(ht_, key, strlen(key), value);
  }

  void AddAssoc(const char* key, size_t key_len, zval* value) noexcept {
    zend_hash_str_update(ht_, key, key_len, value);
  }

  void AddAssoc(const std::string& key, zval* value) noexcept {
    zend_hash_str_update(ht_, key.c_str(), key.length(), value);
  }

  void AddAssoc(zend_string* key, zval* value) noexcept {
    zend_hash_update(ht_, key, value);
  }

  // AddAssocLong - all key variants
  void AddAssocLong(const char* key, zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddAssocLong(const char* key, size_t key_len, zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_hash_str_update(ht_, key, key_len, &zv);
  }

  void AddAssocLong(const std::string& key, zend_long value) noexcept {
    AddAssocLong(key.c_str(), key.length(), value);
  }

  void AddAssocLong(zend_string* key, zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_hash_update(ht_, key, &zv);
  }

  // AddAssocDouble - all key variants
  void AddAssocDouble(const char* key, double value) noexcept {
    zval zv;
    ZVAL_DOUBLE(&zv, value);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddAssocDouble(const char* key, size_t key_len, double value) noexcept {
    zval zv;
    ZVAL_DOUBLE(&zv, value);
    zend_hash_str_update(ht_, key, key_len, &zv);
  }

  void AddAssocDouble(const std::string& key, double value) noexcept {
    AddAssocDouble(key.c_str(), key.length(), value);
  }

  void AddAssocDouble(zend_string* key, double value) noexcept {
    zval zv;
    ZVAL_DOUBLE(&zv, value);
    zend_hash_update(ht_, key, &zv);
  }

  // AddAssocBool - all key variants
  void AddAssocBool(const char* key, bool value) noexcept {
    zval zv;
    ZVAL_BOOL(&zv, value);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddAssocBool(const char* key, size_t key_len, bool value) noexcept {
    zval zv;
    ZVAL_BOOL(&zv, value);
    zend_hash_str_update(ht_, key, key_len, &zv);
  }

  void AddAssocBool(const std::string& key, bool value) noexcept {
    AddAssocBool(key.c_str(), key.length(), value);
  }

  void AddAssocBool(zend_string* key, bool value) noexcept {
    zval zv;
    ZVAL_BOOL(&zv, value);
    zend_hash_update(ht_, key, &zv);
  }

  // AddAssocNull - all key variants
  void AddAssocNull(const char* key) noexcept {
    zval zv;
    ZVAL_NULL(&zv);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  void AddAssocNull(const char* key, size_t key_len) noexcept {
    zval zv;
    ZVAL_NULL(&zv);
    zend_hash_str_update(ht_, key, key_len, &zv);
  }

  void AddAssocNull(const std::string& key) noexcept {
    AddAssocNull(key.c_str(), key.length());
  }

  void AddAssocNull(zend_string* key) noexcept {
    zval zv;
    ZVAL_NULL(&zv);
    zend_hash_update(ht_, key, &zv);
  }

  // String additions - all permutations
  // const char* key, const char* val
  void AddAssocString(const char* key, const char* val) noexcept {
    zval zv;
    ZVAL_STRING(&zv, val);
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  // const char* key, size_t key_size, const char* val, size_t val_size
  void AddAssocString(const char* key, size_t key_size, const char* val,
                      size_t val_size) noexcept {
    zval zv;
    ZVAL_STRINGL(&zv, val, val_size);
    zend_hash_str_update(ht_, key, key_size, &zv);
  }

  // zend_string* key, const char* val
  void AddAssocString(zend_string* key, const char* val) noexcept {
    zval zv;
    ZVAL_STRING(&zv, val);
    zend_hash_update(ht_, key, &zv);
  }

  // zend_string* key, const char* val, size_t val_size
  void AddAssocString(zend_string* key, const char* val, size_t val_size) noexcept {
    zval zv;
    ZVAL_STRINGL(&zv, val, val_size);
    zend_hash_update(ht_, key, &zv);
  }

  // zend_string* key, zend_string* val
  void AddAssocString(zend_string* key, zend_string* val) noexcept {
    zval zv;
    ZVAL_STR(&zv, zend_string_copy(val));
    zend_hash_update(ht_, key, &zv);
  }

  // std::string variants
  // const char* key, const std::string& val
  void AddAssocString(const char* key, const std::string& val) noexcept {
    AddAssocString(key, val.c_str(), val.length());
  }

  // const std::string& key, const char* val
  void AddAssocString(const std::string& key, const char* val) noexcept {
    AddAssocString(key.c_str(), key.length(), val, strlen(val));
  }

  // const std::string& key, const std::string& val
  void AddAssocString(const std::string& key, const std::string& val) noexcept {
    AddAssocString(key.c_str(), key.length(), val.c_str(), val.length());
  }

  // const std::string& key, const char* val, size_t val_size
  void AddAssocString(const std::string& key, const char* val, size_t val_size) noexcept {
    AddAssocString(key.c_str(), key.length(), val, val_size);
  }

  // zend_string* key, const std::string& val
  void AddAssocString(zend_string* key, const std::string& val) noexcept {
    AddAssocString(key, val.c_str(), val.length());
  }

  // Add nested array (HashTable of HashTables) - associative
  inline void AddAssocArray(const char* key, ZArray&& nested) noexcept {
    zval zv;
    ZVAL_ARR(&zv, nested.Release());
    zend_hash_str_update(ht_, key, strlen(key), &zv);
  }

  inline void AddAssocArray(const char* key, size_t key_len, ZArray&& nested) noexcept {
    zval zv;
    ZVAL_ARR(&zv, nested.Release());
    zend_hash_str_update(ht_, key, key_len, &zv);
  }

  inline void AddAssocArray(const std::string& key, ZArray&& nested) noexcept {
    AddAssocArray(key.c_str(), key.length(), std::move(nested));
  }

  inline void AddAssocArray(zend_string* key, ZArray&& nested) noexcept {
    zval zv;
    ZVAL_ARR(&zv, nested.Release());
    zend_hash_update(ht_, key, &zv);
  }

  // Query operations
  [[nodiscard]] inline uint32_t Count() const noexcept { return zend_hash_num_elements(ht_); }

  [[nodiscard]] inline bool IsEmpty() const noexcept { return Count() == 0; }

  // Access to underlying HashTable
  [[nodiscard]] inline HashTable* GetHashTable() noexcept { return ht_; }
  [[nodiscard]] inline const HashTable* GetHashTable() const noexcept { return ht_; }

  // Release ownership (for returning to PHP)
  [[nodiscard]] inline HashTable* Release() noexcept {
    owns_ = false;
    return ht_;
  }

  // Convert to zval for use in PHP return values
  inline void ToZval(zval* target) noexcept {
    ZVAL_ARR(target, Release());
  }

 private:
  HashTable* ht_;
  bool owns_;
};

}  // namespace ZendCPP
