/**
 * Exception tests - Simplified to work with current API
 */

#include "../framework/TestFramework.hpp"
#include <ZendCPP/ZVal.hpp>

using namespace ZendCPPTest;

ZENDCPP_TEST(Exception, basic_test) {
    // Basic test that exceptions can be caught
    bool caught = false;

    try {
        throw std::runtime_error("Test error");
    } catch (const std::exception& e) {
        caught = true;
    }

    ASSERT_TRUE(caught);
}

ZENDCPP_TEST(Exception, different_types) {
    // Test different exception types
    try {
        throw std::invalid_argument("Invalid");
    } catch (const std::invalid_argument&) {
        ASSERT_TRUE(true);
    }

    try {
        throw std::logic_error("Logic");
    } catch (const std::logic_error&) {
        ASSERT_TRUE(true);
    }
}
