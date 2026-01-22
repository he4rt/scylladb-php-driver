#pragma once

#include <php.h>
#include <zend_API.h>

#include <string>

namespace ZendCPP {

/**
 * RAII wrapper for zend_string
 */
class String {
 public:
  String() noexcept : str_(nullptr) {}

  explicit String(size_t size) noexcept { str_ = zend_string_alloc(size, 0); }

  explicit String(const char* cstr) noexcept { str_ = zend_string_init(cstr, strlen(cstr), 0); }

  String(const char* cstr, size_t len) noexcept { str_ = zend_string_init(cstr, len, 0); }

  explicit String(zend_string* str, bool add_ref = true) noexcept : str_(str) {
    if (str_ && add_ref) {
      zend_string_addref(str_);
    }
  }

  ~String() noexcept {
    if (str_) {
      zend_string_release(str_);
    }
  }

  // Delete copy operations
  String(const String&) = delete;
  String& operator=(const String&) = delete;

  // Move operations
  String(String&& other) noexcept : str_(other.str_) { other.str_ = nullptr; }

  String& operator=(String&& other) noexcept {
    if (this != &other) {
      if (str_) {
        zend_string_release(str_);
      }
      str_ = other.str_;
      other.str_ = nullptr;
    }
    return *this;
  }

  /**
   * Get C string
   */
  [[nodiscard]] const char* CStr() const noexcept { return str_ ? ZSTR_VAL(str_) : ""; }

  [[nodiscard]] zend_string* ToZString(bool copy = false) const noexcept {
    if (copy) return zend_string_copy(str_);
    return str_;
  }

  /**
   * Get length
   */
  [[nodiscard]] size_t Length() const noexcept { return str_ ? ZSTR_LEN(str_) : 0; }

  /**
   * Check if empty
   */
  [[nodiscard]] bool IsEmpty() const noexcept { return !str_ || ZSTR_LEN(str_) == 0; }

  /**
   * Check if null
   */
  [[nodiscard]] bool IsNull() const noexcept { return str_ == nullptr; }

  /**
   * Get zend_string (transfers ownership)
   */
  [[nodiscard]] zend_string* Release() noexcept {
    zend_string* tmp = str_;
    str_ = nullptr;
    return tmp;
  }

  /**
   * Get zend_string (keeps ownership)
   */
  [[nodiscard]] zend_string* Get() const noexcept { return str_; }

  /**
   * Convert to std::string
   */
  [[nodiscard]] std::string ToString() const {
    return str_ ? std::string(ZSTR_VAL(str_), ZSTR_LEN(str_)) : std::string();
  }

  /**
   * String operations
   */
  [[nodiscard]] String ToLower() const noexcept {
    if (!str_) return String();
    zend_string* lower = zend_string_tolower(str_);
    return String(lower, false);
  }

  [[nodiscard]] String ToUpper() const noexcept {
    if (!str_) return String();
    zend_string* upper = zend_string_init(ZSTR_VAL(str_), ZSTR_LEN(str_), 0);
    zend_str_toupper(ZSTR_VAL(upper), ZSTR_LEN(upper));
    return String(upper, false);
  }

  /**
   * Comparison
   */
  [[nodiscard]] bool Equals(const String& other) const noexcept {
    if (str_ == other.str_) return true;
    if (!str_ || !other.str_) return false;
    return zend_string_equals(str_, other.str_);
  }

  [[nodiscard]] bool Equals(const char* cstr) const noexcept {
    if (!str_ && !cstr) return true;
    if (!str_ || !cstr) return false;
    return strcmp(ZSTR_VAL(str_), cstr) == 0;
  }

  [[nodiscard]] int Compare(const String& other) const noexcept {
    if (str_ == other.str_) return 0;
    if (!str_) return -1;
    if (!other.str_) return 1;
    return zend_binary_strcmp(ZSTR_VAL(str_), ZSTR_LEN(str_), ZSTR_VAL(other.str_),
                              ZSTR_LEN(other.str_));
  }

  /**
   * Operators
   */
  bool operator==(const String& other) const noexcept { return Equals(other); }

  bool operator!=(const String& other) const noexcept { return !Equals(other); }

  bool operator==(const char* cstr) const noexcept { return Equals(cstr); }

  bool operator!=(const char* cstr) const noexcept { return !Equals(cstr); }

  /**
   * Static factory methods
   */
  static String Copy(zend_string* str) noexcept {
    if (!str) return String();
    return String(zend_string_copy(str), false);
  }

  static String Duplicate(zend_string* str) noexcept {
    if (!str) return String();
    return String(zend_string_dup(str, 0), false);
  }

  static String FromLong(zend_long value) noexcept {
    return String(zend_long_to_str(value), false);
  }

  static String FromDouble(double value) noexcept {
    return String(zend_strpprintf(0, "%.*G", (int)EG(precision), value), false);
  }

 private:
  zend_string* str_;
};

/**
 * Helper functions for string operations
 */
namespace StringUtils {

inline zend_string* Concat(zend_string* left, zend_string* right) noexcept {
  return zend_string_concat2(ZSTR_VAL(left), ZSTR_LEN(left), ZSTR_VAL(right), ZSTR_LEN(right));
}

inline zend_string* Concat(const char* left, size_t left_len, const char* right,
                           size_t right_len) noexcept {
  return zend_string_concat2(left, left_len, right, right_len);
}

inline bool Equals(zend_string* left, zend_string* right) noexcept {
  return zend_string_equals(left, right);
}

inline bool EqualsCaseInsensitive(zend_string* left, zend_string* right) noexcept {
  if (left == right) return true;
  if (!left || !right) return false;
  if (ZSTR_LEN(left) != ZSTR_LEN(right)) return false;
  return zend_binary_strcasecmp(ZSTR_VAL(left), ZSTR_LEN(left), ZSTR_VAL(right), ZSTR_LEN(right)) ==
         0;
}

inline zend_string* Format(const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  zend_string* result = zend_vstrpprintf(0, format, args);
  va_end(args);
  return result;
}

}  // namespace StringUtils

}  // namespace ZendCPP
