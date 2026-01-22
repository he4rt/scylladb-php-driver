#pragma once

#include <php.h>
#include <spl/spl_exceptions.h>
#include <zend_exceptions.h>

#include <stdexcept>
#include <string>

namespace ZendCPP {

/**
 * Helper class for throwing PHP exceptions from C++
 */
class Exception {
 public:
  /**
   * Throw a generic exception
   */
  static void Throw(const char* message, zend_long code = 0) noexcept {
    zend_throw_exception(zend_ce_exception, message, code);
  }

  static void Throw(const std::string& message, zend_long code = 0) noexcept {
    zend_throw_exception(zend_ce_exception, message.c_str(), code);
  }

  /**
   * Throw a specific exception class
   */
  static void ThrowEx(zend_class_entry* ce, const char* message, zend_long code = 0) noexcept {
    zend_throw_exception(ce, message, code);
  }

  static void ThrowEx(zend_class_entry* ce, const std::string& message, zend_long code = 0) noexcept {
    zend_throw_exception(ce, message.c_str(), code);
  }

  /**
   * Throw standard exception types
   */
  static void ThrowInvalidArgument(const char* message) noexcept {
    zend_throw_exception(spl_ce_InvalidArgumentException, message, 0);
  }

  static void ThrowInvalidArgument(const std::string& message) noexcept {
    zend_throw_exception(spl_ce_InvalidArgumentException, message.c_str(), 0);
  }

  static void ThrowRuntimeError(const char* message) noexcept {
    zend_throw_exception(spl_ce_RuntimeException, message, 0);
  }

  static void ThrowRuntimeError(const std::string& message) noexcept {
    zend_throw_exception(spl_ce_RuntimeException, message.c_str(), 0);
  }

  static void ThrowLogicError(const char* message) noexcept {
    zend_throw_exception(spl_ce_LogicException, message, 0);
  }

  static void ThrowLogicError(const std::string& message) noexcept {
    zend_throw_exception(spl_ce_LogicException, message.c_str(), 0);
  }

  static void ThrowBadMethodCall(const char* message) noexcept {
    zend_throw_exception(spl_ce_BadMethodCallException, message, 0);
  }

  static void ThrowBadMethodCall(const std::string& message) noexcept {
    zend_throw_exception(spl_ce_BadMethodCallException, message.c_str(), 0);
  }

  static void ThrowOutOfBounds(const char* message) noexcept {
    zend_throw_exception(spl_ce_OutOfBoundsException, message, 0);
  }

  static void ThrowOutOfBounds(const std::string& message) noexcept {
    zend_throw_exception(spl_ce_OutOfBoundsException, message.c_str(), 0);
  }

  /**
   * Emit error messages
   */
  static void Error(int type, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    php_verror(nullptr, "", type, format, args);
    va_end(args);
  }

  /**
   * Emit warning
   */
  static void Warning(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    php_verror(nullptr, "", E_WARNING, format, args);
    va_end(args);
  }

  /**
   * Emit notice
   */
  static void Notice(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    php_verror(nullptr, "", E_NOTICE, format, args);
    va_end(args);
  }

  /**
   * Check if an exception is pending
   */
  [[nodiscard]] static bool IsPending() noexcept {
    return EG(exception) != nullptr;
  }

  /**
   * Clear pending exception
   */
  static void Clear() noexcept {
    zend_clear_exception();
  }
};

/**
 * RAII guard for exception handling in C++ code
 * Automatically converts C++ exceptions to PHP exceptions
 */
class ExceptionGuard {
 public:
  ExceptionGuard() = default;
  ~ExceptionGuard() = default;

  ExceptionGuard(const ExceptionGuard&) = delete;
  ExceptionGuard& operator=(const ExceptionGuard&) = delete;

  template<typename Func>
  static bool Call(Func&& func, const char* error_prefix = "Error") noexcept {
    try {
      func();
      return true;
    } catch (const std::invalid_argument& e) {
      Exception::ThrowInvalidArgument(std::string(error_prefix) + ": " + e.what());
      return false;
    } catch (const std::runtime_error& e) {
      Exception::ThrowRuntimeError(std::string(error_prefix) + ": " + e.what());
      return false;
    } catch (const std::logic_error& e) {
      Exception::ThrowLogicError(std::string(error_prefix) + ": " + e.what());
      return false;
    } catch (const std::exception& e) {
      Exception::Throw(std::string(error_prefix) + ": " + e.what());
      return false;
    } catch (...) {
      Exception::Throw(std::string(error_prefix) + ": Unknown error");
      return false;
    }
  }
};

} // namespace ZendCPP
