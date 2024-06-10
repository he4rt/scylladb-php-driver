#pragma once

#include <php.h>
#include <zend_smart_str_public.h>

namespace ZendCPP {
class StringBuilder {
 public:
  struct str_with_size {
    const char* str;
    size_t size;
  };

  StringBuilder() noexcept = default;
  explicit StringBuilder(size_t size) noexcept;

  StringBuilder& operator=(const StringBuilder& other) = delete;
  StringBuilder(const StringBuilder& other) = delete;
  StringBuilder(StringBuilder&& other) noexcept;
  ~StringBuilder() noexcept;

  [[nodiscard]] StringBuilder& Extend(size_t size) noexcept;
  [[nodiscard]] size_t Length() const noexcept;

  [[nodiscard]] zend_string* Build() noexcept;

#if PHP_VERSION_ID > 80100
  [[nodiscard]] StringBuilder& TrimToSize() noexcept;
#endif

  StringBuilder& operator<<(const char* str) noexcept;
  StringBuilder& operator<<(const str_with_size& str) noexcept;
  StringBuilder& operator<<(const zend_string* str) noexcept;
  StringBuilder& operator<<(zend_string* str) noexcept;
  StringBuilder& operator<<(const StringBuilder& other) noexcept;

  StringBuilder& operator<<(char value) noexcept;
  StringBuilder& operator<<(int8_t value) noexcept;
  StringBuilder& operator<<(int16_t value) noexcept;
  StringBuilder& operator<<(int32_t value) noexcept;
  StringBuilder& operator<<(int64_t value) noexcept;

  StringBuilder& operator<<(uint8_t value) noexcept;
  StringBuilder& operator<<(uint16_t value) noexcept;
  StringBuilder& operator<<(uint32_t value) noexcept;
  StringBuilder& operator<<(uint64_t value) noexcept;

  StringBuilder& operator<<(float value) noexcept;
  StringBuilder& operator<<(double value) noexcept;

  StringBuilder& operator<<(const zval* value) noexcept;

  StringBuilder& Append(const char* str) noexcept;
  StringBuilder& Append(const str_with_size& str) noexcept;
  StringBuilder& Append(const char* str, size_t size) noexcept;
  StringBuilder& Append(const zend_string* str) noexcept;
  StringBuilder& Append(zend_string* str) noexcept;
  StringBuilder& Append(const StringBuilder& other) noexcept;

  StringBuilder& AppendEscaped(const char* str) noexcept;
  StringBuilder& AppendEscaped(const str_with_size& str) noexcept;
  StringBuilder& AppendEscaped(const char* str, size_t size) noexcept;
  StringBuilder& AppendEscaped(const zend_string* str) noexcept;
  StringBuilder& AppendEscaped(const StringBuilder& other) noexcept;

  StringBuilder& Append(char value) noexcept;
  StringBuilder& Append(int8_t value) noexcept;
  StringBuilder& Append(int16_t value) noexcept;
  StringBuilder& Append(int32_t value) noexcept;
  StringBuilder& Append(int64_t value) noexcept;

  StringBuilder& Append(uint8_t value) noexcept;
  StringBuilder& Append(uint16_t value) noexcept;
  StringBuilder& Append(uint32_t value) noexcept;
  StringBuilder& Append(uint64_t value) noexcept;

  StringBuilder& Append(float value) noexcept;
  StringBuilder& Append(double value) noexcept;

  StringBuilder& Append(const zval* value) noexcept;

 private:
  smart_str m_builder{
      .s = nullptr,
      .a = 0,
  };
};
}  // namespace ZendCPP