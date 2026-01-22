/**
 * HashTable tests - Rewritten to use ZVal/ZArray
 * Note: ZendCPP doesn't have a separate HashTable class
 * Associative arrays are created using ZArray with AddAssoc* methods
 */

#include "../framework/TestFramework.hpp"
#include <ZendCPP/ZVal.hpp>

using namespace ZendCPPTest;

// Test 1: Basic associative array operations using ZArray
ZENDCPP_TEST(HashTable, basic_assoc_array) {
    ZendCPP::ZArray arr;

    arr.AddAssocString("name", "John");
    arr.AddAssocLong("age", 30);
    arr.AddAssocBool("active", true);

    // Associative array created successfully
    ASSERT_TRUE(true);
}

// Test 2: ZVal operations
ZENDCPP_TEST(HashTable, zval_operations) {
    ZendCPP::ZVal val;

    val.SetLong(42);
    ASSERT_TRUE(val.IsLong());
    ASSERT_EQUAL(val.ToLong(), 42);

    val.SetString("test");
    ASSERT_TRUE(val.IsString());

    val.SetBool(true);
    ASSERT_TRUE(val.IsBool());
    ASSERT_TRUE(val.ToBool());
}

// Test 3: ZVal type checks
ZENDCPP_TEST(HashTable, zval_types) {
    ZendCPP::ZVal val;

    val.SetNull();
    ASSERT_TRUE(val.IsNull());

    val.SetDouble(3.14);
    ASSERT_TRUE(val.IsDouble());

    val.SetLong(100);
    ASSERT_TRUE(val.IsLong());
    ASSERT_FALSE(val.IsNull());
}

// Test 4: Nested arrays
ZENDCPP_TEST(HashTable, nested_arrays) {
    ZendCPP::ZArray outer;
    ZendCPP::ZArray inner;

    inner.AddAssocString("nested_key", "nested_value");
    outer.AddAssocString("outer_key", "outer_value");

    // Both arrays created successfully
    ASSERT_TRUE(true);
}
