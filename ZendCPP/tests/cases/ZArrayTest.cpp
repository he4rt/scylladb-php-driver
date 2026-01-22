/**
 * ZArray comprehensive tests
 */

#include "../framework/TestFramework.hpp"
#include <ZendCPP/ZVal.hpp>

using namespace ZendCPP;

// ============================================================================
// Basic Array Construction Tests
// ============================================================================

ZENDCPP_TEST(ZArray, default_constructor) {
    ZArray arr;
    ASSERT_TRUE(arr.Count() == 0);
    ASSERT_TRUE(arr.IsEmpty());
}

ZENDCPP_TEST(ZArray, sized_constructor) {
    ZArray arr(100);
    ASSERT_TRUE(arr.Count() == 0);  // Pre-allocated but empty
    ASSERT_TRUE(arr.IsEmpty());
}

ZENDCPP_TEST(ZArray, packed_array_creation) {
    auto packed = ZArray::CreatePacked(50);
    ASSERT_TRUE(packed.Count() == 0);
    ASSERT_TRUE(packed.IsEmpty());
}

// ============================================================================
// AddNext (Indexed) Tests
// ============================================================================

ZENDCPP_TEST(ZArray, add_next_long) {
    ZArray arr;
    arr.AddNextLong(42);
    arr.AddNextLong(100);
    arr.AddNextLong(-50);

    ASSERT_TRUE(arr.Count() == 3);
    ASSERT_FALSE(arr.IsEmpty());
}

ZENDCPP_TEST(ZArray, add_next_double) {
    ZArray arr;
    arr.AddNextDouble(3.14);
    arr.AddNextDouble(2.71);
    arr.AddNextDouble(-1.5);

    ASSERT_TRUE(arr.Count() == 3);
}

ZENDCPP_TEST(ZArray, add_next_string) {
    ZArray arr;
    arr.AddNextString("test");
    arr.AddNextString("hello world");
    arr.AddNextString("");

    ASSERT_TRUE(arr.Count() == 3);
}

ZENDCPP_TEST(ZArray, add_next_string_with_length) {
    ZArray arr;
    const char* str = "test string";
    arr.AddNextString(str, 4);  // Only "test"
    arr.AddNextString(str, 11); // Full string

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_next_string_std) {
    ZArray arr;
    std::string s1 = "cpp string";
    std::string s2 = "another";

    arr.AddNextString(s1);
    arr.AddNextString(s2);

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_next_bool) {
    ZArray arr;
    arr.AddNextBool(true);
    arr.AddNextBool(false);
    arr.AddNextBool(true);

    ASSERT_TRUE(arr.Count() == 3);
}

ZENDCPP_TEST(ZArray, add_next_null) {
    ZArray arr;
    arr.AddNextNull();
    arr.AddNextNull();

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_next_mixed_types) {
    ZArray arr;
    arr.AddNextLong(42);
    arr.AddNextString("test");
    arr.AddNextDouble(3.14);
    arr.AddNextBool(true);
    arr.AddNextNull();

    ASSERT_TRUE(arr.Count() == 5);
}

// ============================================================================
// AddAssocLong Tests (All Key Variants)
// ============================================================================

ZENDCPP_TEST(ZArray, add_assoc_long_cstring) {
    ZArray arr;
    arr.AddAssocLong("count", 42);
    arr.AddAssocLong("total", 100);

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_long_cstring_with_length) {
    ZArray arr;
    arr.AddAssocLong("count", 5, 42);
    arr.AddAssocLong("key", 3, 100);

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_long_std_string) {
    ZArray arr;
    std::string key1 = "count";
    std::string key2 = "total";

    arr.AddAssocLong(key1, 42);
    arr.AddAssocLong(key2, 100);

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_long_zend_string) {
    ZArray arr;
    zend_string* key = zend_string_init("count", 5, 0);

    arr.AddAssocLong(key, 42);

    ASSERT_TRUE(arr.Count() == 1);
    zend_string_release(key);
}

// ============================================================================
// AddAssocDouble Tests
// ============================================================================

ZENDCPP_TEST(ZArray, add_assoc_double_cstring) {
    ZArray arr;
    arr.AddAssocDouble("pi", 3.14159);
    arr.AddAssocDouble("e", 2.71828);

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_double_std_string) {
    ZArray arr;
    std::string key = "value";
    arr.AddAssocDouble(key, 123.456);

    ASSERT_TRUE(arr.Count() == 1);
}

// ============================================================================
// AddAssocBool Tests
// ============================================================================

ZENDCPP_TEST(ZArray, add_assoc_bool_cstring) {
    ZArray arr;
    arr.AddAssocBool("active", true);
    arr.AddAssocBool("disabled", false);

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_bool_std_string) {
    ZArray arr;
    std::string key = "enabled";
    arr.AddAssocBool(key, true);

    ASSERT_TRUE(arr.Count() == 1);
}

// ============================================================================
// AddAssocNull Tests
// ============================================================================

ZENDCPP_TEST(ZArray, add_assoc_null_cstring) {
    ZArray arr;
    arr.AddAssocNull("field1");
    arr.AddAssocNull("field2");

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_null_std_string) {
    ZArray arr;
    std::string key = "nullable";
    arr.AddAssocNull(key);

    ASSERT_TRUE(arr.Count() == 1);
}

// ============================================================================
// AddAssocString Tests (All Permutations)
// ============================================================================

ZENDCPP_TEST(ZArray, add_assoc_string_cstring_both) {
    ZArray arr;
    arr.AddAssocString("name", "John");
    arr.AddAssocString("city", "New York");

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, add_assoc_string_explicit_lengths) {
    ZArray arr;
    arr.AddAssocString("name", 4, "John Doe", 4);  // Key "name", value "John"

    ASSERT_TRUE(arr.Count() == 1);
}

ZENDCPP_TEST(ZArray, add_assoc_string_std_key_cstring_val) {
    ZArray arr;
    std::string key = "name";
    arr.AddAssocString(key, "Alice");

    ASSERT_TRUE(arr.Count() == 1);
}

ZENDCPP_TEST(ZArray, add_assoc_string_cstring_key_std_val) {
    ZArray arr;
    std::string value = "Bob";
    arr.AddAssocString("name", value);

    ASSERT_TRUE(arr.Count() == 1);
}

ZENDCPP_TEST(ZArray, add_assoc_string_both_std) {
    ZArray arr;
    std::string key = "name";
    std::string value = "Charlie";

    arr.AddAssocString(key, value);

    ASSERT_TRUE(arr.Count() == 1);
}

ZENDCPP_TEST(ZArray, add_assoc_string_zend_key) {
    ZArray arr;
    zend_string* key = zend_string_init("name", 4, 0);

    arr.AddAssocString(key, "David");

    ASSERT_TRUE(arr.Count() == 1);
    zend_string_release(key);
}

ZENDCPP_TEST(ZArray, add_assoc_string_binary_safe) {
    ZArray arr;
    const char* key_with_null = "key\0part";
    const char* val_with_null = "val\0part";

    arr.AddAssocString(key_with_null, 8, val_with_null, 8);

    ASSERT_TRUE(arr.Count() == 1);
}

// ============================================================================
// Nested Array Tests
// ============================================================================

ZENDCPP_TEST(ZArray, add_next_nested_array) {
    ZArray outer;
    ZArray inner;

    inner.AddAssocString("nested", "value");
    inner.AddAssocLong("count", 42);

    outer.AddNextArray(std::move(inner));

    ASSERT_TRUE(outer.Count() == 1);
}

ZENDCPP_TEST(ZArray, add_assoc_nested_array_cstring) {
    ZArray outer;
    ZArray inner;

    inner.AddAssocString("key", "value");
    outer.AddAssocArray("section", std::move(inner));

    ASSERT_TRUE(outer.Count() == 1);
}

ZENDCPP_TEST(ZArray, add_assoc_nested_array_std_string) {
    ZArray outer;
    ZArray inner;
    std::string key = "config";

    inner.AddAssocLong("timeout", 30);
    outer.AddAssocArray(key, std::move(inner));

    ASSERT_TRUE(outer.Count() == 1);
}

ZENDCPP_TEST(ZArray, deeply_nested_arrays) {
    ZArray level3;
    level3.AddAssocString("deep", "value");

    ZArray level2;
    level2.AddAssocArray("level3", std::move(level3));

    ZArray level1;
    level1.AddAssocArray("level2", std::move(level2));

    ZArray root;
    root.AddAssocArray("level1", std::move(level1));

    ASSERT_TRUE(root.Count() == 1);
}

// ============================================================================
// Mixed Operations Tests
// ============================================================================

ZENDCPP_TEST(ZArray, mixed_assoc_types) {
    ZArray arr;

    arr.AddAssocLong("id", 123);
    arr.AddAssocString("name", "Test");
    arr.AddAssocDouble("price", 99.99);
    arr.AddAssocBool("active", true);
    arr.AddAssocNull("optional");

    ASSERT_TRUE(arr.Count() == 5);
}

ZENDCPP_TEST(ZArray, mixed_indexed_and_assoc) {
    ZArray arr;

    arr.AddNextLong(1);
    arr.AddNextLong(2);
    arr.AddAssocString("name", "Test");
    arr.AddNextLong(3);
    arr.AddAssocLong("count", 42);

    ASSERT_TRUE(arr.Count() == 5);
}

// ============================================================================
// Packed Array Tests
// ============================================================================

ZENDCPP_TEST(ZArray, packed_array_performance) {
    auto packed = ZArray::CreatePacked(1000);

    for (int i = 0; i < 1000; i++) {
        packed.AddNextLong(i);
    }

    ASSERT_TRUE(packed.Count() == 1000);
}

ZENDCPP_TEST(ZArray, packed_array_mixed_types) {
    auto packed = ZArray::CreatePacked(10);

    packed.AddNextLong(1);
    packed.AddNextString("test");
    packed.AddNextDouble(3.14);
    packed.AddNextBool(true);
    packed.AddNextNull();

    ASSERT_TRUE(packed.Count() == 5);
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

ZENDCPP_TEST(ZArray, move_constructor) {
    ZArray arr1;
    arr1.AddAssocLong("count", 42);
    arr1.AddAssocString("name", "Test");

    ZArray arr2(std::move(arr1));

    ASSERT_TRUE(arr2.Count() == 2);
    // arr1 is now in moved-from state
}

ZENDCPP_TEST(ZArray, move_assignment) {
    ZArray arr1;
    arr1.AddAssocLong("count", 42);

    ZArray arr2;
    arr2 = std::move(arr1);

    ASSERT_TRUE(arr2.Count() == 1);
    // arr1 is now in moved-from state
}

// ============================================================================
// HashTable Access Tests
// ============================================================================

ZENDCPP_TEST(ZArray, get_hash_table) {
    ZArray arr;
    arr.AddAssocLong("count", 42);

    HashTable* ht = arr.GetHashTable();
    ASSERT_TRUE(ht != nullptr);
    ASSERT_TRUE(zend_hash_num_elements(ht) == 1);
}

ZENDCPP_TEST(ZArray, release_ownership) {
    ZArray arr;
    arr.AddAssocLong("count", 42);

    HashTable* ht = arr.Release();
    ASSERT_TRUE(ht != nullptr);
    ASSERT_TRUE(zend_hash_num_elements(ht) == 1);

    // Must manually destroy since we released ownership
    zend_array_destroy(ht);
}

// ============================================================================
// Complex Real-World Scenarios
// ============================================================================

ZENDCPP_TEST(ZArray, user_data_structure) {
    ZArray user;

    user.AddAssocLong("id", 123);
    user.AddAssocString("name", "John Doe");
    user.AddAssocString("email", "john@example.com");
    user.AddAssocLong("age", 30);
    user.AddAssocBool("active", true);

    ASSERT_TRUE(user.Count() == 5);
}

ZENDCPP_TEST(ZArray, config_structure) {
    ZArray db_config;
    db_config.AddAssocString("host", "localhost");
    db_config.AddAssocLong("port", 3306);
    db_config.AddAssocString("user", "admin");

    ZArray cache_config;
    cache_config.AddAssocString("driver", "redis");
    cache_config.AddAssocLong("ttl", 3600);

    ZArray config;
    config.AddAssocArray("database", std::move(db_config));
    config.AddAssocArray("cache", std::move(cache_config));

    ASSERT_TRUE(config.Count() == 2);
}

ZENDCPP_TEST(ZArray, list_of_items) {
    auto items = ZArray::CreatePacked(100);

    for (int i = 0; i < 100; i++) {
        ZArray item;
        item.AddAssocLong("id", i);
        item.AddAssocString("name", "Item " + std::to_string(i));
        items.AddNextArray(std::move(item));
    }

    ASSERT_TRUE(items.Count() == 100);
}

ZENDCPP_TEST(ZArray, key_override_behavior) {
    ZArray arr;

    arr.AddAssocLong("count", 42);
    arr.AddAssocLong("count", 100);  // Should override

    // Count is still 1 because same key
    ASSERT_TRUE(arr.Count() == 1);
}

ZENDCPP_TEST(ZArray, empty_strings) {
    ZArray arr;

    arr.AddAssocString("empty", "");
    arr.AddNextString("");

    ASSERT_TRUE(arr.Count() == 2);
}

ZENDCPP_TEST(ZArray, unicode_strings) {
    ZArray arr;

    arr.AddAssocString("greeting", "Hello 世界");
    arr.AddAssocString("emoji", "🎉🚀✨");

    ASSERT_TRUE(arr.Count() == 2);
}
