/**
 * ZVal comprehensive tests
 */

#include <ZendCPP/ZVal.hpp>
#include <map>

#include "../framework/TestFramework.hpp"

using namespace ZendCPP;

// ============================================================================
// Construction Tests
// ============================================================================

ZENDCPP_TEST(ZVal, default_constructor) {
  ZVal val;
  ASSERT_TRUE(val.IsUndef());
}

ZENDCPP_TEST(ZVal, construct_from_zval) {
  zval zv;
  ZVAL_LONG(&zv, 42);

  ZVal val(&zv);
  ASSERT_TRUE(val.IsLong());
  ASSERT_TRUE(val.ToLong() == 42);
}

// ============================================================================
// Copy Semantics Tests
// ============================================================================

ZENDCPP_TEST(ZVal, copy_constructor_long) {
  ZVal v1;
  v1.SetLong(42);

  ZVal v2(v1);

  ASSERT_TRUE(v2.IsLong());
  ASSERT_TRUE(v2.ToLong() == 42);
}

ZENDCPP_TEST(ZVal, copy_assignment_long) {
  ZVal v1;
  v1.SetLong(100);

  ZVal v2;
  v2 = v1;

  ASSERT_TRUE(v2.IsLong());
  ASSERT_TRUE(v2.ToLong() == 100);
}

ZENDCPP_TEST(ZVal, copy_constructor_string) {
  ZVal v1;
  v1.SetString("test string");

  ZVal v2(v1);

  ASSERT_TRUE(v2.IsString());
  // Both should reference the same or copied string
}

ZENDCPP_TEST(ZVal, copy_assignment_string) {
  ZVal v1;
  v1.SetString("original");

  ZVal v2;
  v2.SetLong(42);  // Different type initially
  v2 = v1;

  ASSERT_TRUE(v2.IsString());
}

ZENDCPP_TEST(ZVal, self_assignment) {
  ZVal v1;
  v1.SetLong(42);

  v1 = v1;  // Should be safe

  ASSERT_TRUE(v1.IsLong());
  ASSERT_TRUE(v1.ToLong() == 42);
}

ZENDCPP_TEST(ZVal, copy_multiple_times) {
  ZVal v1;
  v1.SetString("test");

  ZVal v2(v1);
  ZVal v3(v2);
  ZVal v4 = v3;

  ASSERT_TRUE(v4.IsString());
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

ZENDCPP_TEST(ZVal, move_constructor) {
  ZVal v1;
  v1.SetLong(42);

  ZVal v2(std::move(v1));

  ASSERT_TRUE(v2.IsLong());
  ASSERT_TRUE(v2.ToLong() == 42);
  ASSERT_TRUE(v1.IsUndef());  // Moved-from state
}

ZENDCPP_TEST(ZVal, move_assignment) {
  ZVal v1;
  v1.SetString("test");

  ZVal v2;
  v2 = std::move(v1);

  ASSERT_TRUE(v2.IsString());
  ASSERT_TRUE(v1.IsUndef());  // Moved-from state
}

// ============================================================================
// SetNull Tests
// ============================================================================

ZENDCPP_TEST(ZVal, set_null) {
  ZVal val;
  val.SetNull();

  ASSERT_TRUE(val.IsNull());
  ASSERT_FALSE(val.IsLong());
  ASSERT_FALSE(val.IsString());
}

ZENDCPP_TEST(ZVal, set_null_overwrites) {
  ZVal val;
  val.SetLong(42);
  val.SetNull();

  ASSERT_TRUE(val.IsNull());
  ASSERT_FALSE(val.IsLong());
}

// ============================================================================
// SetBool Tests
// ============================================================================

ZENDCPP_TEST(ZVal, set_bool_true) {
  ZVal val;
  val.SetBool(true);

  ASSERT_TRUE(val.IsBool());
  ASSERT_TRUE(val.ToBool());
}

ZENDCPP_TEST(ZVal, set_bool_false) {
  ZVal val;
  val.SetBool(false);

  ASSERT_TRUE(val.IsBool());
  ASSERT_FALSE(val.ToBool());
}

ZENDCPP_TEST(ZVal, set_bool_overwrites) {
  ZVal val;
  val.SetLong(100);
  val.SetBool(true);

  ASSERT_TRUE(val.IsBool());
  ASSERT_FALSE(val.IsLong());
}

// ============================================================================
// SetLong Tests
// ============================================================================

ZENDCPP_TEST(ZVal, set_long_positive) {
  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(val.IsLong());
  ASSERT_TRUE(val.ToLong() == 42);
}

ZENDCPP_TEST(ZVal, set_long_negative) {
  ZVal val;
  val.SetLong(-100);

  ASSERT_TRUE(val.IsLong());
  ASSERT_TRUE(val.ToLong() == -100);
}

ZENDCPP_TEST(ZVal, set_long_zero) {
  ZVal val;
  val.SetLong(0);

  ASSERT_TRUE(val.IsLong());
  ASSERT_TRUE(val.ToLong() == 0);
}

ZENDCPP_TEST(ZVal, set_long_max) {
  ZVal val;
  val.SetLong(ZEND_LONG_MAX);

  ASSERT_TRUE(val.IsLong());
  ASSERT_TRUE(val.ToLong() == ZEND_LONG_MAX);
}

ZENDCPP_TEST(ZVal, set_long_min) {
  ZVal val;
  val.SetLong(ZEND_LONG_MIN);

  ASSERT_TRUE(val.IsLong());
  ASSERT_TRUE(val.ToLong() == ZEND_LONG_MIN);
}

// ============================================================================
// SetDouble Tests
// ============================================================================

ZENDCPP_TEST(ZVal, set_double_positive) {
  ZVal val;
  val.SetDouble(3.14);

  ASSERT_TRUE(val.IsDouble());
  ASSERT_TRUE(val.ToDouble() > 3.13 && val.ToDouble() < 3.15);
}

ZENDCPP_TEST(ZVal, set_double_negative) {
  ZVal val;
  val.SetDouble(-2.71);

  ASSERT_TRUE(val.IsDouble());
  ASSERT_TRUE(val.ToDouble() < -2.70 && val.ToDouble() > -2.72);
}

ZENDCPP_TEST(ZVal, set_double_zero) {
  ZVal val;
  val.SetDouble(0.0);

  ASSERT_TRUE(val.IsDouble());
  ASSERT_TRUE(val.ToDouble() == 0.0);
}

ZENDCPP_TEST(ZVal, set_double_scientific) {
  ZVal val;
  val.SetDouble(1.23e-10);

  ASSERT_TRUE(val.IsDouble());
}

// ============================================================================
// SetString Tests
// ============================================================================

ZENDCPP_TEST(ZVal, set_string_cstring) {
  ZVal val;
  val.SetString("test string");

  ASSERT_TRUE(val.IsString());
}

ZENDCPP_TEST(ZVal, set_string_empty) {
  ZVal val;
  val.SetString("");

  ASSERT_TRUE(val.IsString());
}

ZENDCPP_TEST(ZVal, set_string_with_length) {
  ZVal val;
  val.SetString("test string", 4);  // Only "test"

  ASSERT_TRUE(val.IsString());
}

ZENDCPP_TEST(ZVal, set_string_binary_safe) {
  ZVal val;
  const char* str_with_null = "test\0string";
  val.SetString(str_with_null, 11);

  ASSERT_TRUE(val.IsString());
}

ZENDCPP_TEST(ZVal, set_string_zend_string) {
  zend_string* zstr = zend_string_init("test", 4, 0);

  ZVal val;
  val.SetString(zstr);

  ASSERT_TRUE(val.IsString());
  // SetString copies the zend_string (increments refcount), so we release our reference
  zend_string_release(zstr);
}

ZENDCPP_TEST(ZVal, set_string_unicode) {
  ZVal val;
  val.SetString("Hello 世界 🎉");

  ASSERT_TRUE(val.IsString());
}

// ============================================================================
// SetArray Tests
// ============================================================================

ZENDCPP_TEST(ZVal, set_array) {
  ZArray arr;
  arr.AddAssocLong("count", 42);

  ZVal val;
  val.SetArray(arr.Release());

  ASSERT_TRUE(val.IsArray());
}

ZENDCPP_TEST(ZVal, set_array_empty) {
  ZArray arr;

  ZVal val;
  val.SetArray(arr.Release());

  ASSERT_TRUE(val.IsArray());
}

// ============================================================================
// Type Checking Tests
// ============================================================================

ZENDCPP_TEST(ZVal, is_null) {
  ZVal val;
  val.SetNull();

  ASSERT_TRUE(val.IsNull());
  ASSERT_FALSE(val.IsBool());
  ASSERT_FALSE(val.IsLong());
  ASSERT_FALSE(val.IsDouble());
  ASSERT_FALSE(val.IsString());
  ASSERT_FALSE(val.IsArray());
  ASSERT_FALSE(val.IsObject());
  ASSERT_FALSE(val.IsResource());
}

ZENDCPP_TEST(ZVal, is_bool) {
  ZVal val;
  val.SetBool(true);

  ASSERT_TRUE(val.IsBool());
  ASSERT_FALSE(val.IsNull());
  ASSERT_FALSE(val.IsLong());
}

ZENDCPP_TEST(ZVal, is_long) {
  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(val.IsLong());
  ASSERT_FALSE(val.IsDouble());
  ASSERT_FALSE(val.IsString());
}

ZENDCPP_TEST(ZVal, is_double) {
  ZVal val;
  val.SetDouble(3.14);

  ASSERT_TRUE(val.IsDouble());
  ASSERT_FALSE(val.IsLong());
  ASSERT_FALSE(val.IsString());
}

ZENDCPP_TEST(ZVal, is_string) {
  ZVal val;
  val.SetString("test");

  ASSERT_TRUE(val.IsString());
  ASSERT_FALSE(val.IsLong());
  ASSERT_FALSE(val.IsArray());
}

ZENDCPP_TEST(ZVal, is_array) {
  ZArray arr;
  ZVal val;
  val.SetArray(arr.Release());

  ASSERT_TRUE(val.IsArray());
  ASSERT_FALSE(val.IsString());
  ASSERT_FALSE(val.IsLong());
}

ZENDCPP_TEST(ZVal, is_undef) {
  ZVal val;

  ASSERT_TRUE(val.IsUndef());

  val.SetLong(42);
  ASSERT_FALSE(val.IsUndef());
}

// ============================================================================
// Type Conversion Tests
// ============================================================================

ZENDCPP_TEST(ZVal, to_bool_from_true) {
  ZVal val;
  val.SetBool(true);

  ASSERT_TRUE(val.ToBool());
}

ZENDCPP_TEST(ZVal, to_bool_from_false) {
  ZVal val;
  val.SetBool(false);

  ASSERT_FALSE(val.ToBool());
}

ZENDCPP_TEST(ZVal, to_bool_from_long_nonzero) {
  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(val.ToBool());
}

ZENDCPP_TEST(ZVal, to_bool_from_long_zero) {
  ZVal val;
  val.SetLong(0);

  ASSERT_FALSE(val.ToBool());
}

ZENDCPP_TEST(ZVal, to_bool_from_string_nonempty) {
  ZVal val;
  val.SetString("test");

  ASSERT_TRUE(val.ToBool());
}

ZENDCPP_TEST(ZVal, to_bool_from_string_empty) {
  ZVal val;
  val.SetString("");

  ASSERT_FALSE(val.ToBool());
}

ZENDCPP_TEST(ZVal, to_long_from_long) {
  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(val.ToLong() == 42);
}

ZENDCPP_TEST(ZVal, to_long_from_double) {
  ZVal val;
  val.SetDouble(3.14);

  ASSERT_TRUE(val.ToLong() == 3);  // Truncated
}

ZENDCPP_TEST(ZVal, to_long_from_string) {
  ZVal val;
  val.SetString("123");

  ASSERT_TRUE(val.ToLong() == 123);
}

ZENDCPP_TEST(ZVal, to_double_from_double) {
  ZVal val;
  val.SetDouble(3.14);

  double result = val.ToDouble();
  ASSERT_TRUE(result > 3.13 && result < 3.15);
}

ZENDCPP_TEST(ZVal, to_double_from_long) {
  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(val.ToDouble() == 42.0);
}

ZENDCPP_TEST(ZVal, to_string_from_string) {
  ZVal val;
  val.SetString("test");

  zend_string* result = val.ToString();
  ASSERT_TRUE(result != nullptr);
  zend_string_release(result);
}

ZENDCPP_TEST(ZVal, to_string_from_long) {
  ZVal val;
  val.SetLong(42);

  zend_string* result = val.ToString();
  ASSERT_TRUE(result != nullptr);
  // Should be "42"
  zend_string_release(result);
}

// ============================================================================
// Accessor Tests
// ============================================================================

ZENDCPP_TEST(ZVal, get_access) {
  ZVal val;
  val.SetLong(42);

  zval* zv = val.Get();
  ASSERT_TRUE(zv != nullptr);
  ASSERT_TRUE(Z_TYPE_P(zv) == IS_LONG);
  ASSERT_TRUE(Z_LVAL_P(zv) == 42);
}

ZENDCPP_TEST(ZVal, arrow_operator) {
  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(Z_TYPE_P(val.operator->()) == IS_LONG);
}

ZENDCPP_TEST(ZVal, dereference_operator) {
  ZVal val;
  val.SetLong(42);

  zval& zv = *val;
  ASSERT_TRUE(Z_TYPE(zv) == IS_LONG);
  ASSERT_TRUE(Z_LVAL(zv) == 42);
}

// ============================================================================
// Type Mutation Tests
// ============================================================================

ZENDCPP_TEST(ZVal, change_type_sequence) {
  ZVal val;

  val.SetLong(42);
  ASSERT_TRUE(val.IsLong());

  val.SetString("test");
  ASSERT_TRUE(val.IsString());
  ASSERT_FALSE(val.IsLong());

  val.SetDouble(3.14);
  ASSERT_TRUE(val.IsDouble());
  ASSERT_FALSE(val.IsString());

  val.SetBool(true);
  ASSERT_TRUE(val.IsBool());
  ASSERT_FALSE(val.IsDouble());

  val.SetNull();
  ASSERT_TRUE(val.IsNull());
  ASSERT_FALSE(val.IsBool());
}

ZENDCPP_TEST(ZVal, overwrite_string_with_long) {
  ZVal val;
  val.SetString("long string that allocates memory");
  val.SetLong(42);

  ASSERT_TRUE(val.IsLong());
  ASSERT_FALSE(val.IsString());
}

ZENDCPP_TEST(ZVal, overwrite_array_with_string) {
  ZArray arr;
  arr.AddAssocLong("key", 42);

  ZVal val;
  val.SetArray(arr.Release());
  val.SetString("test");

  ASSERT_TRUE(val.IsString());
  ASSERT_FALSE(val.IsArray());
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

ZENDCPP_TEST(ZVal, empty_string_is_not_null) {
  ZVal val;
  val.SetString("");

  ASSERT_TRUE(val.IsString());
  ASSERT_FALSE(val.IsNull());
}

ZENDCPP_TEST(ZVal, zero_is_not_null) {
  ZVal val;
  val.SetLong(0);

  ASSERT_TRUE(val.IsLong());
  ASSERT_FALSE(val.IsNull());
}

ZENDCPP_TEST(ZVal, false_is_not_null) {
  ZVal val;
  val.SetBool(false);

  ASSERT_TRUE(val.IsBool());
  ASSERT_FALSE(val.IsNull());
}

ZENDCPP_TEST(ZVal, zero_double_is_not_null) {
  ZVal val;
  val.SetDouble(0.0);

  ASSERT_TRUE(val.IsDouble());
  ASSERT_FALSE(val.IsNull());
}

// ============================================================================
// Real-World Usage Tests
// ============================================================================

ZENDCPP_TEST(ZVal, store_in_vector) {
  std::vector<ZVal> values;

  ZVal v1;
  v1.SetLong(42);

  ZVal v2;
  v2.SetString("test");

  values.push_back(v1);
  values.push_back(v2);
  values.emplace_back();

  ASSERT_TRUE(values.size() == 3);
  ASSERT_TRUE(values[0].IsLong());
  ASSERT_TRUE(values[1].IsString());
}

ZENDCPP_TEST(ZVal, store_in_map) {
  std::map<std::string, ZVal> values_map;

  ZVal count;
  count.SetLong(42);

  ZVal name;
  name.SetString("Test");

  values_map["count"] = count;
  values_map["name"] = name;

  ASSERT_TRUE(values_map.size() == 2);
  ASSERT_TRUE(values_map["count"].IsLong());
  ASSERT_TRUE(values_map["name"].IsString());
}

ZENDCPP_TEST(ZVal, return_by_value) {
  auto create_val = []() -> ZVal {
    ZVal val;
    val.SetLong(42);
    return val;
  };

  ZVal result = create_val();
  ASSERT_TRUE(result.IsLong());
  ASSERT_TRUE(result.ToLong() == 42);
}

ZENDCPP_TEST(ZVal, pass_by_value) {
  auto process = [](ZVal val) { return val.IsLong() && val.ToLong() == 42; };

  ZVal val;
  val.SetLong(42);

  ASSERT_TRUE(process(val));
  ASSERT_TRUE(val.IsLong());  // Original unchanged
}

ZENDCPP_TEST(ZVal, multiple_copies_share_refcount) {
  ZVal v1;
  v1.SetString("shared string");

  ZVal v2 = v1;
  ZVal v3 = v2;
  ZVal v4 = v3;

  // All four should have the same string (refcounted)
  ASSERT_TRUE(v1.IsString());
  ASSERT_TRUE(v2.IsString());
  ASSERT_TRUE(v3.IsString());
  ASSERT_TRUE(v4.IsString());
}
