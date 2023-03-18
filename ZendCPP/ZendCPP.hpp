#pragma once

#include <php.h>

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

#ifndef ZEND_OBJECT_OFFSET_MEMBER
#define ZEND_OBJECT_OFFSET_MEMBER zval
#endif

namespace ZendCPP {
template <typename T> ZENDCPP_ALWAYS_INLINE T *ObjectFetch(zend_object *obj) {
  auto offset = reinterpret_cast<std::size_t>(
      &reinterpret_cast<T *>(0)->ZEND_OBJECT_OFFSET_MEMBER);
  auto *casted =
      reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(obj) - offset);
  return casted;
}

template <typename T> ZENDCPP_ALWAYS_INLINE T *ObjectFetch(zval *val) {
  return ObjectFetch<T>(Z_OBJ_P(val));
}

template <typename T>
ZENDCPP_ALWAYS_INLINE T *Allocate(zend_class_entry *ce,
                                  zend_object_handlers *handlers) {
  auto *self =
      static_cast<T *>(emalloc(sizeof(T) + zend_object_properties_size(ce)));
  zend_object_std_init(&self->ZEND_OBJECT_OFFSET_MEMBER, ce);

  if (zend_object_properties_size(ce) > 0) {
    object_properties_init(&self->ZEND_OBJECT_OFFSET_MEMBER, ce);
  }

  self->zval.handlers = handlers;

  return self;
}
} // namespace ZendCPP