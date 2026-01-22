#pragma once

#include <php.h>
#include <zend_API.h>

namespace ZendCPP {

/**
 * Helper for calling PHP functions from C++
 */
class FunctionCaller {
 public:
  /**
   * Call a function by name
   */
  static bool Call(const char* function_name, zval* retval, uint32_t param_count, zval params[]) noexcept {
    zval func_name;
    ZVAL_STRING(&func_name, function_name);

    int result = call_user_function(CG(function_table), nullptr, &func_name, retval, param_count, params);

    zval_ptr_dtor(&func_name);
    return result == SUCCESS;
  }

  /**
   * Call a method on an object
   */
  static bool CallMethod(zval* object, const char* method_name, zval* retval, uint32_t param_count, zval params[]) noexcept {
    zval func_name;
    ZVAL_STRING(&func_name, method_name);

    int result = call_user_function(nullptr, object, &func_name, retval, param_count, params);

    zval_ptr_dtor(&func_name);
    return result == SUCCESS;
  }

  /**
   * Call a static method
   */
  static bool CallStatic(zend_class_entry* ce, const char* method_name, zval* retval, uint32_t param_count, zval params[]) noexcept {
    zval func_name;
    ZVAL_STRING(&func_name, method_name);

    zval obj;
    ZVAL_NULL(&obj);

    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    fci.size = sizeof(fci);
    fci.retval = retval;
    fci.params = params;
    fci.param_count = param_count;
    fci.named_params = nullptr;
    ZVAL_STR_COPY(&fci.function_name, Z_STR(func_name));

    fcc.function_handler = nullptr;
    fcc.called_scope = ce;
    fcc.object = nullptr;

    int result = zend_call_function(&fci, &fcc);

    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&fci.function_name);

    return result == SUCCESS;
  }

  /**
   * Check if function exists
   */
  static bool FunctionExists(const char* function_name) noexcept {
    zend_string* name = zend_string_init(function_name, strlen(function_name), 0);
    bool exists = zend_hash_exists(CG(function_table), name);
    zend_string_release(name);
    return exists;
  }

  /**
   * Check if method exists
   */
  static bool MethodExists(zend_class_entry* ce, const char* method_name) noexcept {
    zend_string* name = zend_string_init(method_name, strlen(method_name), 0);
    zend_string* lc_name = zend_string_tolower(name);
    bool exists = zend_hash_exists(&ce->function_table, lc_name);
    zend_string_release(lc_name);
    zend_string_release(name);
    return exists;
  }
};

/**
 * Helper for checking instance types
 */
class InstanceOf {
 public:
  /**
   * Check if object is instance of class
   */
  static bool Check(zval* obj, zend_class_entry* ce) noexcept {
    if (Z_TYPE_P(obj) != IS_OBJECT) {
      return false;
    }
    return instanceof_function(Z_OBJCE_P(obj), ce);
  }

  /**
   * Check if object is instance of class by name
   */
  static bool Check(zval* obj, const char* class_name) noexcept {
    if (Z_TYPE_P(obj) != IS_OBJECT) {
      return false;
    }

    zend_string* name = zend_string_init(class_name, strlen(class_name), 0);
    zend_string* lc_name = zend_string_tolower(name);
    zend_class_entry* ce = zend_lookup_class(lc_name);

    zend_string_release(lc_name);
    zend_string_release(name);

    if (!ce) {
      return false;
    }

    return instanceof_function(Z_OBJCE_P(obj), ce);
  }
};

/**
 * Helper for object instantiation
 */
class ObjectFactory {
 public:
  /**
   * Create object instance
   */
  static bool CreateObject(zend_class_entry* ce, zval* retval) noexcept {
    if (object_init_ex(retval, ce) != SUCCESS) {
      return false;
    }

    // Call constructor if it exists
    if (ce->constructor) {
      zval func_name;
      ZVAL_STRING(&func_name, "__construct");

      zval ret;
      call_user_function(nullptr, retval, &func_name, &ret, 0, nullptr);

      zval_ptr_dtor(&func_name);
      zval_ptr_dtor(&ret);
    }

    return true;
  }

  /**
   * Create object with constructor arguments
   */
  static bool CreateObject(zend_class_entry* ce, zval* retval, uint32_t param_count, zval params[]) noexcept {
    if (object_init_ex(retval, ce) != SUCCESS) {
      return false;
    }

    // Call constructor if it exists
    if (ce->constructor) {
      zval func_name;
      ZVAL_STRING(&func_name, "__construct");

      zval ret;
      call_user_function(nullptr, retval, &func_name, &ret, param_count, params);

      zval_ptr_dtor(&func_name);
      zval_ptr_dtor(&ret);
    }

    return true;
  }
};

/**
 * Helper for working with object properties
 */
class PropertyReader {
 public:
  explicit PropertyReader(zval* obj) noexcept : obj_(obj) {}

  /**
   * Read property value
   */
  [[nodiscard]] zval* Read(const char* name) const noexcept {
    return zend_read_property(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name), 1, nullptr);
  }

  [[nodiscard]] zval* Read(zend_string* name) const noexcept {
    return zend_read_property(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), ZSTR_VAL(name), ZSTR_LEN(name), 1, nullptr);
  }

  /**
   * Write property value
   */
  void Write(const char* name, zval* value) const noexcept {
    zend_update_property(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name), value);
  }

  void Write(zend_string* name, zval* value) const noexcept {
    zend_update_property(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), ZSTR_VAL(name), ZSTR_LEN(name), value);
  }

  /**
   * Write typed property values
   */
  void WriteLong(const char* name, zend_long value) const noexcept {
    zend_update_property_long(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name), value);
  }

  void WriteDouble(const char* name, double value) const noexcept {
    zend_update_property_double(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name), value);
  }

  void WriteString(const char* name, const char* value) const noexcept {
    zend_update_property_string(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name), value);
  }

  void WriteBool(const char* name, bool value) const noexcept {
    zend_update_property_bool(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name), value);
  }

  void WriteNull(const char* name) const noexcept {
    zend_update_property_null(Z_OBJCE_P(obj_), Z_OBJ_P(obj_), name, strlen(name));
  }

 private:
  zval* obj_;
};

} // namespace ZendCPP
