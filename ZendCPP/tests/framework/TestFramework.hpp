#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * ZendCPP Test Framework
 *
 * Simple framework for testing ZendCPP features. Each test is a separate
 * file but all compile into one extension.
 */

namespace ZendCPPTest {

// Test function type
using TestFunc = std::function<void()>;

// Test case
struct TestCase {
  std::string name;
  std::string category;
  TestFunc cpp_test;

  TestCase(std::string n, std::string cat, TestFunc func)
      : name(std::move(n)), category(std::move(cat)), cpp_test(std::move(func)) {}
};

// Test registry
class TestRegistry {
 public:
  static TestRegistry& Instance() {
    static TestRegistry instance;
    return instance;
  }

  void RegisterTest(const std::string& name, const std::string& category, TestFunc func) {
    tests.emplace_back(name, category, func);
  }

  [[nodiscard]] const std::vector<TestCase>& GetTests() const { return tests; }

  [[nodiscard]] std::vector<TestCase> GetTestsByCategory(const std::string& category) const {
    std::vector<TestCase> result;
    for (const auto& test : tests) {
      if (test.category == category) {
        result.push_back(test);
      }
    }
    return result;
  }

 private:
  TestRegistry() = default;
  std::vector<TestCase> tests;
};

// Helper to auto-register tests
class TestRegistrar {
 public:
  TestRegistrar(const std::string& name, const std::string& category, TestFunc func) {
    TestRegistry::Instance().RegisterTest(name, category, func);
  }
};

// Macros for easy test definition
#define ZENDCPP_TEST(category, name)                               \
  static void zendcpp_test_##category##_##name();                  \
  static ZendCPPTest::TestRegistrar registrar_##category##_##name( \
      #name, #category, zendcpp_test_##category##_##name);         \
  static void zendcpp_test_##category##_##name()

// Assert helpers
#define ASSERT_TRUE(condition)                                                \
  if (!(condition)) {                                                         \
    throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
  }

#define ASSERT_FALSE(condition)                                                \
  if (condition) {                                                             \
    throw std::runtime_error(std::string("Assertion failed: !") + #condition); \
  }

#define ASSERT_EQUAL(a, b)                                                          \
  if ((a) != (b)) {                                                                 \
    throw std::runtime_error(std::string("Assertion failed: ") + #a + " == " + #b); \
  }

#define ASSERT_NOT_EQUAL(a, b)                                                      \
  if ((a) == (b)) {                                                                 \
    throw std::runtime_error(std::string("Assertion failed: ") + #a + " != " + #b); \
  }

}  // namespace ZendCPPTest
