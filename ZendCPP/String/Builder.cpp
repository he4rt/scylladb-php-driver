#include "Builder.h"

#include <zend_smart_str.h>

#include <utility>

namespace ZendCPP {

StringBuilder::StringBuilder(std::size_t size) noexcept {
  if (size > 0) {
    smart_str_alloc(&m_builder, size, false);
  }
}

StringBuilder::~StringBuilder() noexcept { smart_str_free(&m_builder); }

StringBuilder::StringBuilder(StringBuilder&& other) noexcept
    : m_builder({other.m_builder.s, other.m_builder.a}) {
  std::exchange(other.m_builder.s, nullptr);
  std::exchange(other.m_builder.a, 0);
}
std::size_t StringBuilder::Length() const noexcept {
  return m_builder.s ? ZSTR_LEN(m_builder.s) : 0;
}

#if PHP_VERSION_ID > 80200
[[nodiscard]] StringBuilder& StringBuilder::TrimToSize() noexcept {
  smart_str_trim_to_size(&m_builder);
  return *this;
}

#endif

[[nodiscard]] StringBuilder& StringBuilder::Extend(std::size_t size) noexcept {
  smart_str_extend(&m_builder, size);

  return *this;
}

[[nodiscard]] zend_string * StringBuilder::Build() noexcept {
  return smart_str_extract(&m_builder);
}

StringBuilder& StringBuilder::operator<<(const char* str) noexcept {
  smart_str_appends(&m_builder, str);
  return *this;
}

StringBuilder& StringBuilder::operator<<(const zend_string* str) noexcept {
  smart_str_append(&m_builder, str);
  return *this;
}

StringBuilder& StringBuilder::operator<<(zend_string* str) noexcept {
  smart_str_append(&m_builder, str);
  return *this;
}

StringBuilder& StringBuilder::operator<<(const StringBuilder::str_with_size& str) noexcept {
  smart_str_appendl(&m_builder, str.str, str.size);
  return *this;
}

StringBuilder& StringBuilder::operator<<(const StringBuilder& other) noexcept {
  smart_str_append_smart_str(&m_builder, &other.m_builder);
  return *this;
}

StringBuilder& StringBuilder::operator<<(char value) noexcept {
  smart_str_appendc(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(int8_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(int16_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(int32_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(int64_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(uint8_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(uint16_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(uint32_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::operator<<(uint64_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}
StringBuilder& StringBuilder::Append(const char* str) noexcept {
  smart_str_appends(&m_builder, str);
  return *this;
}

StringBuilder& StringBuilder::Append(const zend_string* str) noexcept {
  smart_str_append(&m_builder, str);
  return *this;
}

StringBuilder& StringBuilder::Append(zend_string* str) noexcept {
  smart_str_append(&m_builder, str);
  return *this;
}

StringBuilder& StringBuilder::Append(const StringBuilder::str_with_size& str) noexcept {
  smart_str_appendl(&m_builder, str.str, str.size);
  return *this;
}

StringBuilder& StringBuilder::Append(const char* str, std::size_t size) noexcept {
  smart_str_appendl(&m_builder, str, size);
  return *this;
}

StringBuilder& StringBuilder::Append(const StringBuilder& other) noexcept {
  smart_str_append_smart_str(&m_builder, &other.m_builder);
  return *this;
}

StringBuilder& StringBuilder::Append(char value) noexcept {
  smart_str_appendc(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(int8_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(int16_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(int32_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(int64_t value) noexcept {
  smart_str_append_long(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(uint8_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(uint16_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}

StringBuilder& StringBuilder::Append(uint32_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}
StringBuilder& StringBuilder::Append(uint64_t value) noexcept {
  smart_str_append_unsigned(&m_builder, value);
  return *this;
}
StringBuilder& StringBuilder::operator<<(float value) noexcept {
  smart_str_append_double(&m_builder, value, 6, false);
  return *this;
}
StringBuilder& StringBuilder::operator<<(double value) noexcept {
  smart_str_append_double(&m_builder, value, 6, false);
  return *this;
}
StringBuilder& StringBuilder::Append(double value) noexcept {
  smart_str_append_double(&m_builder, value, 6, false);
  return *this;
}
StringBuilder& StringBuilder::Append(float value) noexcept {
  smart_str_append_double(&m_builder, value, 6, false);
  return *this;
}

StringBuilder& StringBuilder::Append(const zval* value) noexcept {
  smart_str_append_scalar(&m_builder, (zval*)value, 0);
  return *this;
}
StringBuilder& StringBuilder::operator<<(const zval* value) noexcept {
  smart_str_append_scalar(&m_builder, (zval*)value, 0);
  return *this;
}
StringBuilder& StringBuilder::AppendEscaped(const char* str) noexcept {
  smart_str_append_escaped(&m_builder, str, strlen(str));
  return *this;
}

StringBuilder& StringBuilder::AppendEscaped(const StringBuilder::str_with_size& str) noexcept {
  smart_str_append_escaped(&m_builder, str.str, str.size);
  return *this;
}
StringBuilder& StringBuilder::AppendEscaped(const char* str, std::size_t size) noexcept {
  smart_str_append_escaped(&m_builder, str, size);
  return *this;
}
StringBuilder& StringBuilder::AppendEscaped(const zend_string* str) noexcept {
  smart_str_append_escaped_truncated(&m_builder, const_cast<zend_string*>(str), ZSTR_LEN(str));
  return *this;
}
StringBuilder& StringBuilder::AppendEscaped(const StringBuilder& other) noexcept {
  smart_str_append_escaped_truncated(&m_builder, other.m_builder.s, ZSTR_LEN(other.m_builder.s));
  return *this;
}

}  // namespace ZendCPP