#pragma once

#include <php.h>
#include <type_traits>

namespace ZendCPP {

/**
 * Type traits for working with Zend types
 */
namespace Traits {

// Check if type is a zval
template<typename T>
struct is_zval : std::is_same<typename std::remove_cv<T>::type, zval> {};

// Check if type is a zend_string
template<typename T>
struct is_zend_string : std::is_same<typename std::remove_cv<T>::type, zend_string> {};

// Check if type is a zend_object
template<typename T>
struct is_zend_object : std::is_same<typename std::remove_cv<T>::type, zend_object> {};

// Check if type is a HashTable
template<typename T>
struct is_hashtable : std::is_same<typename std::remove_cv<T>::type, HashTable> {};

} // namespace Traits

/**
 * Scope guard for automatic cleanup
 */
template<typename Func>
class ScopeGuard {
 public:
  explicit ScopeGuard(Func&& func) noexcept : func_(std::forward<Func>(func)), active_(true) {}

  ~ScopeGuard() noexcept {
    if (active_) {
      func_();
    }
  }

  void Dismiss() noexcept {
    active_ = false;
  }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;

  ScopeGuard(ScopeGuard&& other) noexcept
      : func_(std::move(other.func_)), active_(other.active_) {
    other.active_ = false;
  }

 private:
  Func func_;
  bool active_;
};

/**
 * Helper to create scope guards
 */
template<typename Func>
ScopeGuard<Func> MakeScopeGuard(Func&& func) noexcept {
  return ScopeGuard<Func>(std::forward<Func>(func));
}

/**
 * RAII wrapper for addref/delref
 */
class RefGuard {
 public:
  explicit RefGuard(zval* val) noexcept : val_(val) {
    if (val_) {
      Z_TRY_ADDREF_P(val_);
    }
  }

  ~RefGuard() noexcept {
    if (val_) {
      zval_ptr_dtor(val_);
    }
  }

  RefGuard(const RefGuard&) = delete;
  RefGuard& operator=(const RefGuard&) = delete;

  RefGuard(RefGuard&& other) noexcept : val_(other.val_) {
    other.val_ = nullptr;
  }

 private:
  zval* val_;
};

/**
 * Utilities for type conversion
 */
namespace Convert {

/**
 * Convert various types to zend_long
 */
inline zend_long ToLong(const zval* val) noexcept {
  return zval_get_long(const_cast<zval*>(val));
}

inline zend_long ToLong(double val) noexcept {
  return static_cast<zend_long>(val);
}

inline zend_long ToLong(bool val) noexcept {
  return val ? 1 : 0;
}

/**
 * Convert to double
 */
inline double ToDouble(const zval* val) noexcept {
  return zval_get_double(const_cast<zval*>(val));
}

inline double ToDouble(zend_long val) noexcept {
  return static_cast<double>(val);
}

/**
 * Convert to bool
 */
inline bool ToBool(const zval* val) noexcept {
  return zend_is_true(const_cast<zval*>(val));
}

inline bool ToBool(zend_long val) noexcept {
  return val != 0;
}

/**
 * Convert to string (caller must release)
 */
inline zend_string* ToString(const zval* val) noexcept {
  return zval_get_string(const_cast<zval*>(val));
}

inline zend_string* ToString(zend_long val) noexcept {
  return zend_long_to_str(val);
}

inline zend_string* ToString(double val) noexcept {
  return zend_strpprintf(0, "%.*G", (int)EG(precision), val);
}

} // namespace Convert

/**
 * Memory utilities
 */
namespace Memory {

/**
 * Smart pointer for emalloc'd memory
 */
template<typename T>
class EMallocPtr {
 public:
  EMallocPtr() noexcept : ptr_(nullptr) {}

  explicit EMallocPtr(size_t count) noexcept {
    ptr_ = static_cast<T*>(emalloc(sizeof(T) * count));
  }

  ~EMallocPtr() noexcept {
    if (ptr_) {
      efree(ptr_);
    }
  }

  EMallocPtr(const EMallocPtr&) = delete;
  EMallocPtr& operator=(const EMallocPtr&) = delete;

  EMallocPtr(EMallocPtr&& other) noexcept : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }

  EMallocPtr& operator=(EMallocPtr&& other) noexcept {
    if (this != &other) {
      if (ptr_) {
        efree(ptr_);
      }
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    }
    return *this;
  }

  T* Get() noexcept { return ptr_; }
  const T* Get() const noexcept { return ptr_; }

  T* Release() noexcept {
    T* tmp = ptr_;
    ptr_ = nullptr;
    return tmp;
  }

  T& operator*() noexcept { return *ptr_; }
  const T& operator*() const noexcept { return *ptr_; }

  T* operator->() noexcept { return ptr_; }
  const T* operator->() const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

 private:
  T* ptr_;
};

/**
 * Allocate zeroed memory
 */
template<typename T>
EMallocPtr<T> ECallocPtr(size_t count = 1) noexcept {
  EMallocPtr<T> ptr;
  T* mem = static_cast<T*>(ecalloc(count, sizeof(T)));
  ptr = EMallocPtr<T>();
  // Hack to set the pointer
  return std::move(*reinterpret_cast<EMallocPtr<T>*>(&mem));
}

} // namespace Memory

/**
 * Debug utilities
 */
namespace Debug {

/**
 * Check memory leaks in debug mode
 */
#ifdef PHP_DEBUG
#define ZENDCPP_DEBUG_PRINT(...) ZendCPP::Debug::Print(__VA_ARGS__)
#define ZENDCPP_DEBUG_DUMP(val, label) ZendCPP::Debug::DumpZval(val, label)
#else
#define ZENDCPP_DEBUG_PRINT(...) ((void)0)
#define ZENDCPP_DEBUG_DUMP(val, label) ((void)0)
#endif

} // namespace Debug

/**
 * Common helper macros
 */

// Safely get string value from zval
#define ZENDCPP_ZVAL_STR(zv) (Z_TYPE_P(zv) == IS_STRING ? Z_STRVAL_P(zv) : "")
#define ZENDCPP_ZVAL_STRLEN(zv) (Z_TYPE_P(zv) == IS_STRING ? Z_STRLEN_P(zv) : 0)

// Safely get numeric value from zval
#define ZENDCPP_ZVAL_LONG_SAFE(zv) (Z_TYPE_P(zv) == IS_LONG ? Z_LVAL_P(zv) : 0)
#define ZENDCPP_ZVAL_DOUBLE_SAFE(zv) (Z_TYPE_P(zv) == IS_DOUBLE ? Z_DVAL_P(zv) : 0.0)

// Check zval type
#define ZENDCPP_IS_STRING(zv) (Z_TYPE_P(zv) == IS_STRING)
#define ZENDCPP_IS_LONG(zv) (Z_TYPE_P(zv) == IS_LONG)
#define ZENDCPP_IS_DOUBLE(zv) (Z_TYPE_P(zv) == IS_DOUBLE)
#define ZENDCPP_IS_ARRAY(zv) (Z_TYPE_P(zv) == IS_ARRAY)
#define ZENDCPP_IS_OBJECT(zv) (Z_TYPE_P(zv) == IS_OBJECT)
#define ZENDCPP_IS_NULL(zv) (Z_TYPE_P(zv) == IS_NULL)
#define ZENDCPP_IS_BOOL(zv) (Z_TYPE_P(zv) == IS_TRUE || Z_TYPE_P(zv) == IS_FALSE)

/**
 * Iteration helpers
 */
namespace Iterate {

/**
 * Iterate over array with callback
 */
template<typename Func>
void Array(zval* arr, Func&& callback) {
  if (Z_TYPE_P(arr) != IS_ARRAY) {
    return;
  }

  HashTable* ht = Z_ARRVAL_P(arr);
  zend_ulong idx;
  zend_string* key;
  zval* val;

  ZEND_HASH_FOREACH_KEY_VAL(ht, idx, key, val) {
    callback(idx, key, val);
  } ZEND_HASH_FOREACH_END();
}

/**
 * Iterate over array values only
 */
template<typename Func>
void ArrayValues(zval* arr, Func&& callback) {
  if (Z_TYPE_P(arr) != IS_ARRAY) {
    return;
  }

  HashTable* ht = Z_ARRVAL_P(arr);
  zval* val;

  ZEND_HASH_FOREACH_VAL(ht, val) {
    callback(val);
  } ZEND_HASH_FOREACH_END();
}

/**
 * Map array values to new array
 */
template<typename Func>
zval MapArray(zval* arr, Func&& callback) {
  zval result;
  array_init(&result);

  if (Z_TYPE_P(arr) != IS_ARRAY) {
    return result;
  }

  HashTable* ht = Z_ARRVAL_P(arr);
  zval* val;

  ZEND_HASH_FOREACH_VAL(ht, val) {
    zval mapped = callback(val);
    add_next_index_zval(&result, &mapped);
  } ZEND_HASH_FOREACH_END();

  return result;
}

/**
 * Filter array values
 */
template<typename Func>
zval FilterArray(zval* arr, Func&& predicate) {
  zval result;
  array_init(&result);

  if (Z_TYPE_P(arr) != IS_ARRAY) {
    return result;
  }

  HashTable* ht = Z_ARRVAL_P(arr);
  zval* val;

  ZEND_HASH_FOREACH_VAL(ht, val) {
    if (predicate(val)) {
      Z_TRY_ADDREF_P(val);
      add_next_index_zval(&result, val);
    }
  } ZEND_HASH_FOREACH_END();

  return result;
}

} // namespace Iterate

} // namespace ZendCPP
