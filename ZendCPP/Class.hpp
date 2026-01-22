#pragma once

#include <php.h>
#include <zend_API.h>
#include <zend_interfaces.h>
#include <functional>
#include <type_traits>

namespace ZendCPP {

/**
 * Class registration helper
 */
class ClassBuilder {
 public:
  ClassBuilder(const char* name, zend_class_entry** ce_ptr) noexcept
      : name_(name), ce_ptr_(ce_ptr) {}

  /**
   * Set parent class
   */
  ClassBuilder& Extends(zend_class_entry* parent) noexcept {
    parent_ = parent;
    return *this;
  }

  /**
   * Implement interfaces
   */
  ClassBuilder& Implements(zend_class_entry* interface) noexcept {
    interfaces_.push_back(interface);
    return *this;
  }

  /**
   * Set class flags
   */
  ClassBuilder& SetFinal() noexcept {
    flags_ |= ZEND_ACC_FINAL;
    return *this;
  }

  ClassBuilder& SetAbstract() noexcept {
    flags_ |= ZEND_ACC_ABSTRACT;
    return *this;
  }

  /**
   * Set object handlers
   */
  ClassBuilder& SetCreateHandler(zend_object* (*handler)(zend_class_entry*)) noexcept {
    create_handler_ = handler;
    return *this;
  }

  /**
   * Register the class
   */
  zend_class_entry* Register(const zend_function_entry* methods) noexcept {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, name_, methods);

    if (parent_) {
      *ce_ptr_ = zend_register_internal_class_ex(&ce, parent_);
    } else {
      *ce_ptr_ = zend_register_internal_class(&ce);
    }

    if (*ce_ptr_) {
      (*ce_ptr_)->ce_flags |= flags_;

      if (create_handler_) {
        (*ce_ptr_)->create_object = create_handler_;
      }

      // Implement interfaces
      for (auto* interface : interfaces_) {
        zend_class_implements(*ce_ptr_, 1, interface);
      }
    }

    return *ce_ptr_;
  }

 private:
  const char* name_;
  zend_class_entry** ce_ptr_;
  zend_class_entry* parent_ = nullptr;
  std::vector<zend_class_entry*> interfaces_;
  uint32_t flags_ = 0;
  zend_object* (*create_handler_)(zend_class_entry*) = nullptr;
};

/**
 * Object handler builder for easier setup
 */
template<typename T>
class ObjectHandlerBuilder {
 public:
  explicit ObjectHandlerBuilder(zend_object_handlers* handlers) noexcept
      : handlers_(handlers) {
    memcpy(handlers_, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    handlers_->offset = offsetof(T, ZEND_OBJECT_OFFSET_MEMBER);
  }

  ObjectHandlerBuilder& SetFreeObj(void (*free_obj)(zend_object*)) noexcept {
    handlers_->free_obj = free_obj;
    return *this;
  }

  ObjectHandlerBuilder& SetClone(zend_object* (*clone_obj)(zend_object*)) noexcept {
    handlers_->clone_obj = clone_obj;
    return *this;
  }

  ObjectHandlerBuilder& SetCompare(int (*compare)(zval*, zval*)) noexcept {
    handlers_->compare = compare;
    return *this;
  }

  ObjectHandlerBuilder& SetGetProperties(HashTable* (*get_properties)(zend_object*)) noexcept {
    handlers_->get_properties = get_properties;
    return *this;
  }

  ObjectHandlerBuilder& SetGetGc(HashTable* (*get_gc)(zend_object*, zval**, int*)) noexcept {
    handlers_->get_gc = get_gc;
    return *this;
  }

  ObjectHandlerBuilder& SetCastObject(int (*cast_object)(zend_object*, zval*, int)) noexcept {
    handlers_->cast_object = reinterpret_cast<zend_object_cast_t>(cast_object);
    return *this;
  }

  ObjectHandlerBuilder& SetCountElements(int (*count_elements)(zend_object*, zend_long*)) noexcept {
    handlers_->count_elements = reinterpret_cast<zend_object_count_elements_t>(count_elements);
    return *this;
  }

  ObjectHandlerBuilder& SetReadProperty(zval* (*read_property)(zend_object*, zend_string*, int, void**, zval*)) noexcept {
    handlers_->read_property = read_property;
    return *this;
  }

  ObjectHandlerBuilder& SetWriteProperty(zval* (*write_property)(zend_object*, zend_string*, zval*, void**)) noexcept {
    handlers_->write_property = write_property;
    return *this;
  }

  ObjectHandlerBuilder& DisableClone() noexcept {
    handlers_->clone_obj = nullptr;
    return *this;
  }

  zend_object_handlers* Get() noexcept {
    return handlers_;
  }

 private:
  zend_object_handlers* handlers_;
};

/**
 * Helper macros for method declarations
 */
#define ZENDCPP_METHOD(class_name, method_name) \
  ZEND_NAMED_FUNCTION(zim_##class_name##_##method_name)

#define ZENDCPP_ME(class_name, method_name, arg_info, flags) \
  ZEND_FENTRY(method_name, zim_##class_name##_##method_name, arg_info, flags)

/**
 * Property helper
 */
class PropertyHelper {
 public:
  explicit PropertyHelper(zend_class_entry* ce) noexcept : ce_(ce) {}

  void Declare(const char* name, zend_long default_value, int flags = ZEND_ACC_PUBLIC) noexcept {
    zval zv;
    ZVAL_LONG(&zv, default_value);
    zend_declare_property(ce_, name, strlen(name), &zv, flags);
  }

  void Declare(const char* name, double default_value, int flags = ZEND_ACC_PUBLIC) noexcept {
    zval zv;
    ZVAL_DOUBLE(&zv, default_value);
    zend_declare_property(ce_, name, strlen(name), &zv, flags);
  }

  void Declare(const char* name, const char* default_value, int flags = ZEND_ACC_PUBLIC) noexcept {
    zval zv;
    ZVAL_STRING(&zv, default_value);
    zend_declare_property(ce_, name, strlen(name), &zv, flags);
  }

  void Declare(const char* name, bool default_value, int flags = ZEND_ACC_PUBLIC) noexcept {
    zval zv;
    ZVAL_BOOL(&zv, default_value);
    zend_declare_property(ce_, name, strlen(name), &zv, flags);
  }

  void DeclareNull(const char* name, int flags = ZEND_ACC_PUBLIC) noexcept {
    zval zv;
    ZVAL_NULL(&zv);
    zend_declare_property(ce_, name, strlen(name), &zv, flags);
  }

  void DeclareConst(const char* name, zend_long value) noexcept {
    zval zv;
    ZVAL_LONG(&zv, value);
    zend_declare_class_constant(ce_, name, strlen(name), &zv);
  }

  void DeclareConst(const char* name, const char* value) noexcept {
    zval zv;
    ZVAL_STRING(&zv, value);
    zend_declare_class_constant(ce_, name, strlen(name), &zv);
  }

 private:
  zend_class_entry* ce_;
};

} // namespace ZendCPP
