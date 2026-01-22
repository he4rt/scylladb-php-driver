#pragma once

/**
 * ZendCPP - Modern C++ Wrapper for Zend API
 *
 * This library provides RAII wrappers and helper classes for working with
 * PHP's Zend Engine from C++. It makes extension development safer and more
 * ergonomic by providing:
 *
 * - Memory-safe wrappers for zval, zend_string, arrays
 * - Exception handling helpers
 * - Class and object management utilities
 * - String manipulation helpers
 * - Function calling utilities
 * - Type-safe object fetching and allocation
 */

#include <zend_API.h>
#include <cstring>

// Compiler-specific attributes
#if defined(__GNUC__)
#if __GNUC__ >= 3
#define ZENDCPP_ALWAYS_INLINE inline __attribute__((always_inline))
#define ZENDCPP_NEVER_INLINE __attribute__((noinline))
#else
#define ZENDCPP_ALWAYS_INLINE inline
#define ZENDCPP_NEVER_INLINE
#endif
#else
#if __has_attribute(always_inline)
#define ZENDCPP_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define ZENDCPP_ALWAYS_INLINE inline
#endif
#if __has_attribute(noinline)
#define ZENDCPP_NEVER_INLINE __attribute__((noinline))
#else
#define ZENDCPP_NEVER_INLINE
#endif
#endif

// C linkage macros
#ifdef __cplusplus
#define EXTERN_C() extern "C" {
#define END_EXTERN_C() }
#else
#define BEGIN_EXTERN_C()
#define END_EXTERN_C()
#endif

// Default member name for zend_object in custom objects
#ifndef ZEND_OBJECT_OFFSET_MEMBER
#define ZEND_OBJECT_OFFSET_MEMBER zendObject
#endif

namespace ZendCPP {

/**
 * Fetch custom object from zend_object
 *
 * @tparam T Type of the custom object containing zend_object
 * @param obj Pointer to zend_object
 * @return Pointer to the custom object
 */
template <typename T>
ZENDCPP_ALWAYS_INLINE T *ObjectFetch(zend_object *obj) {
  auto offset = reinterpret_cast<std::size_t>(&reinterpret_cast<T *>(0)->ZEND_OBJECT_OFFSET_MEMBER);
  auto *casted = reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(obj) - offset);
  return casted;
}

/**
 * Fetch custom object from zval containing an object
 *
 * @tparam T Type of the custom object
 * @param obj Pointer to zval containing object
 * @return Pointer to the custom object
 */
template <typename T>
ZENDCPP_ALWAYS_INLINE T *ObjectFetch(zval *obj) {
  return ObjectFetch<T>(Z_OBJ_P(obj));
}

/**
 * Allocate and initialize custom object
 *
 * @tparam T Type of the custom object
 * @param ce Class entry
 * @param handlers Object handlers
 * @return Pointer to initialized custom object
 */
template <typename T>
ZENDCPP_ALWAYS_INLINE T *Allocate(zend_class_entry *ce, zend_object_handlers *handlers) {
  auto *self = static_cast<T *>(emalloc(sizeof(T) + zend_object_properties_size(ce)));
  std::memset(self, 0, sizeof(T));
  zend_object_std_init(&self->ZEND_OBJECT_OFFSET_MEMBER, ce);

  if (zend_object_properties_size(ce) > 0) {
    object_properties_init(&self->ZEND_OBJECT_OFFSET_MEMBER, ce);
  }

  self->zendObject.handlers = handlers;

  return self;
}

/**
 * Allocate custom object with template handler type
 */
template <typename T, typename THandlers>
ZENDCPP_ALWAYS_INLINE T *Allocate(zend_class_entry *ce, THandlers *handlers) {
  auto *self = static_cast<T *>(emalloc(sizeof(T) + zend_object_properties_size(ce)));
  zend_object_std_init(&self->ZEND_OBJECT_OFFSET_MEMBER, ce);

  if (zend_object_properties_size(ce) > 0) {
    object_properties_init(&self->ZEND_OBJECT_OFFSET_MEMBER, ce);
  }

  self->zendObject.handlers = reinterpret_cast<zend_object_handlers *>(handlers);

  return self;
}

/**
 * Initialize object handlers with correct offset
 *
 * @tparam Object Type of the custom object
 * @tparam T Type of handlers (usually zend_object_handlers or custom struct)
 * @param handlers Pointer to handlers to initialize
 * @return Initialized handlers pointer
 */
template <typename Object, typename T>
[[maybe_unused]] ZENDCPP_ALWAYS_INLINE T *InitHandlers(T *handlers) {
  memcpy(handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  auto h = reinterpret_cast<zend_object_handlers *>(handlers);
  h->offset = XtOffsetOf(Object, ZEND_OBJECT_OFFSET_MEMBER);
  return handlers;
}

}  // namespace ZendCPP

// Include all ZendCPP components
#include "ZVal.hpp"
#include "String.hpp"
#include "Exception.hpp"
#include "Helpers.hpp"
#include "Class.hpp"
#include "Runtime.hpp"
#include "Utilities.hpp"
